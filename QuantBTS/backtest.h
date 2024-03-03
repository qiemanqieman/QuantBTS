#pragma once

// 回测框架设置
#include <iostream>

#include "Data.h"
#include "Statistics.h"
#include "Strategy.h"
#include <memory>

class BackTest {
private:
	double initialCapital;
	double commission;
	std::vector<Bar> data;
	std::unique_ptr<Strategy> strategy;
	std::vector<double> equityCurve;
	std::vector<int> signal;

public:
	BackTest() : initialCapital(10000), commission(0.001) {}

	void addDataFeed(const DataFeed& feed, const std::string& filename) {
		this->data = feed.loadData(filename);
	}

	void addDataFeed(std::vector<Bar> data) {
		this->data = data;
	}

	void setStrategy(Strategy* strategy) {
		this->strategy.reset(strategy);
	}

	void setInitialCapital(double capital) {
		initialCapital = capital;
	}

	void setCommission(double commission) {
		this->commission = commission;
	}

	Statistics run() {
		// Implementation of the backtesting logic goes here
		strategy->applyStrategy(data, signal);

		double capital = initialCapital;
		double position = 0.0;  // 持仓股数

		for (size_t i = 0; i < data.size(); ++i) {
			if (signal[i] == 1) { // buy
				position = capital / data[i].close;
				capital -= commission * capital;
			}
			else if (signal[i] == -1 && position > 0) { // sell
				capital = position * data[i].close;
				position = 0;
				capital -= commission * capital;
			}
			equityCurve.push_back(capital + position * data[i].close);
		}
		
		return Statistics(equityCurve);
	}
};