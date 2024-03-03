//#pragma once

// Statistics module for calculating and storing statistical indicators

#include <vector>
#include <numeric>
#include <algorithm>
#include <sstream>

#include <vector>
#include <numeric>
#include <algorithm>
#include <sstream>
#include <unordered_map>

class Statistics {
private:
	std::vector<double> equityCurve, dailyReturns;

	// Three private utility functions
	static double _mean(const std::vector<double>& curve) { // Calculate mean
		double sum = std::accumulate(curve.begin(), curve.end(), 0.0);
		return sum / curve.size();
	}
	static double _stdDev(const std::vector<double>& curve) { // Calculate standard deviation
		double mean = _mean(curve);
		double sqSum = std::inner_product(curve.begin(), curve.end(), curve.begin(), 0.0);
		return std::sqrt(sqSum / curve.size() - mean * mean);
	}
	static double _covariance(const std::vector<double>& op1, const std::vector<double>& op2) { // Calculate covariance
		auto mean1 = _mean(op1), mean2 = _mean(op2);
		auto cov = 0;
		auto size = op1.size();
		for (size_t i = 0; i < size; ++i)
			cov += (op1[i] - mean1) * (op2[i] - mean2);
		return cov / size;
	}

public:
	Statistics(const std::vector<double>& ec) : equityCurve(ec) {
		// Calculate daily returns
		for (size_t i = 1; i < ec.size(); ++i) {
			double dailyReturn = (ec[i] - ec[i - 1]) / ec[i - 1];
			dailyReturns.push_back(dailyReturn);
		}
	}

	// 计算期末资产总数
	double finalAssets() { return equityCurve.back(); }

	// Calculate total return during the investment period
	double totalReturn() {
		return (equityCurve.back() - equityCurve.front()) / equityCurve.front();
	}

	// Calculate Sharpe ratio
	double sharpeRatio(double riskFreeRate = 0.0) {
		const double annualizedReturnRatio = annualizedReturn(),
			annualizedVolatility_ = annualizedVolatility();
		return (annualizedReturnRatio - riskFreeRate) / annualizedVolatility_;
	}

	// Calculate annualized return, converting total return to annual standard for easy comparison of strategies with different time spans
	// For simplicity, assume there are 252 trading days in a year
	double annualizedReturn() {
		double tr = totalReturn();
		return pow(1 + tr, 252.0 / equityCurve.size()) - 1;
	}

	// Calculate maximum drawdown, a key indicator for measuring the maximum potential loss a strategy may suffer, is a key indicator for risk assessment
	double maxDrawdown() {
		double peak = equityCurve.front();
		double maxDrawdown = 0.0;
		for (auto& value : equityCurve) {
			if (value > peak) {
				peak = value;
			}
			double drawdown = (peak - value) / peak;
			if (drawdown > maxDrawdown) {
				maxDrawdown = drawdown;
			}
		}
		return maxDrawdown;
	}

	// Calculate IC (Information Coefficient): a measure of the linear correlation between predicted values and actual values, commonly used to evaluate the performance of prediction models in quantitative investment
	double IC(const std::vector<double>& predictionCurve) {
		// Calculate covariance
		double covariance = _covariance(predictionCurve, equityCurve);

		// Calculate standard deviation of forecasted values and actual values
		double stdForecast = _stdDev(predictionCurve);
		double stdActual = _stdDev(equityCurve);

		// Calculate IC
		return covariance / (stdForecast * stdActual);
	}

	// Calculate Beta coefficient: a measure of the sensitivity of a strategy to the overall market volatility
	double beta(const std::vector<double>& marketCurve) {
		double cov = _covariance(equityCurve, marketCurve);
		double var = _covariance(marketCurve, marketCurve);
		return cov / var;
	}

	// Calculate Alpha coefficient: a measure of the risk-adjusted excess return of a strategy relative to a benchmark index
	// Alpha = Annualized Strategy Return - (Risk-Free Rate + Beta * (Annualized Market Return - Risk-Free Rate))
	// NOTE: R = Rf + Beta * (Rb - Rf) + Alpha
	// In practice, Beta and Alpha need to be calculated based on the actual returns of the strategy, market returns, risk-free rate, etc.
	double alpha(double annualizedStrategyReturn, double riskFreeRate, double beta, double annualizedMarketReturn) {
		return annualizedStrategyReturn - (riskFreeRate + beta * (annualizedMarketReturn - riskFreeRate));
	}

	// Calculate annualized volatility
	double annualizedVolatility() {
		double mean = _mean(dailyReturns);
		double sqSum = std::inner_product(dailyReturns.begin(), dailyReturns.end(), dailyReturns.begin(), 0.0);
		double sigma = std::sqrt(sqSum / dailyReturns.size() - mean * mean); // Daily volatility
		return sigma * std::sqrt(252); // Annualized volatility (assuming 252 trading days in a year)
	}

	// Calculate Sortino ratio = (Annualized Return - Risk-Free Rate) / Downside Volatility. Similar to Sharpe ratio, but only considers downside risk,
	// making it a powerful tool for evaluating strategy performance in adverse market conditions
	double sortinoRatio(double riskFreeRate = 0, double targetReturn = 0) {
		double annualizedReturn_ = annualizedReturn(); // Assuming 252 trading days per year
		double downsideRisk = 0.0;

		for (auto& ret : dailyReturns) {
			if (ret < targetReturn)
				downsideRisk += std::pow(ret - targetReturn, 2);
		}
		downsideRisk = std::sqrt(downsideRisk / dailyReturns.size()) * std::sqrt(252); // Annualized downside risk

		return (annualizedReturn_ - riskFreeRate) / downsideRisk;
	}

	// Calculate Calmar ratio = Annualized Return / Maximum Drawdown. Evaluates the performance of a strategy by dividing the annualized return by the maximum drawdown, especially in the face of severe losses.
	double calmarRatio()
	{
		return annualizedReturn() / maxDrawdown();
	}

	// Calculate mean daily return
	double meanDailyReturn() {
		return _mean(dailyReturns);
	}

	// Calculate standard deviation of daily return
	double stdDailyReturn() {
		return _stdDev(dailyReturns);
	}

	// Get various statistics as a dictionary
	std::unordered_map<std::string, double> statistics() {
		std::unordered_map<std::string, double> statistics;
		statistics["Final Assets"] = finalAssets();
		statistics["Mean Daily Return"] = meanDailyReturn();
		statistics["Annualized Return"] = annualizedReturn();
		statistics["Max Drawdown"] = maxDrawdown();
		statistics["Annualized Volatility"] = annualizedVolatility();
		statistics["Sharpe Ratio"] = sharpeRatio();
		statistics["Sortino Ratio"] = sortinoRatio();
		statistics["Calmar Ratio"] = calmarRatio();
		statistics["Alpha"] = alpha(annualizedReturn(), 0.03, beta(equityCurve), 0.1);
		statistics["Beta"] = beta(equityCurve);
		statistics["IC"] = IC(equityCurve);
		return statistics;
	}
};