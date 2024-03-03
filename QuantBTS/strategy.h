#pragma once

// 交易策略, 实现了三种简单的交易策略

#include "data.h"
#include <vector>

// 抽象基类
class Strategy {
public:
	virtual void applyStrategy(const std::vector<Bar>& data, std::vector<int>& signal) = 0;
};

// 简化的移动平均交易策略实现
// 为了简单起见，我们将实现一个非常基础的移动平均策略，当短期移动平均超过长期移动平均时买入，反之则卖出。
class SimpleMovingAverageStrategy : public Strategy {
private:
	size_t shortWindow;
	size_t longWindow;

	double calculateSMA(const std::vector<Bar>& data, size_t start, size_t end) {
		double sum = 0.0;
		for (size_t i = start; i < end; i++) {
			sum += data[i].close;
		}
		return sum / (end - start);
	}

public:
	SimpleMovingAverageStrategy(size_t shortWindow, size_t longWindow) :
		shortWindow(shortWindow), longWindow(longWindow) {}

	void applyStrategy(const std::vector<Bar>& data, std::vector<int>& signal) override {
		// A simple SMA strategy would go here
		signal.resize(data.size(), 0);

		for (size_t i = longWindow; i < data.size(); i++) {
			double shortSMA = calculateSMA(data, i - shortWindow, i);
			double longSMA = calculateSMA(data, i - longWindow, i);

			if (shortSMA > longSMA && signal[i - 1] != 1) {
				signal[i] = 1; // buy
			}
			else if (shortSMA < longSMA && signal[i - 1] != -1) {
				signal[i] = -1; // sell
			}
		}
	}
};


// 买入持有策略
class BuyAndHoldStrategy : public Strategy {
public:
	void applyStrategy(const std::vector<Bar>& data, std::vector<int>& signal) override {
		signal.resize(data.size(), 0);
		// 仅在策略开始时买入
		if (!data.empty()) {
			signal[0] = 1; // 第一天买入
		}
		// 之后保持持有，无需额外操作，故不设置其他信号
	}
};


// 简单的针对单一资产（股票）的动量策略
// 动量策略基于一个简单的假设：如果一个资产在过去一段时间内表现良好，那么它在未来也可能继续会表现良好。
class MomentumStrategy : public Strategy {
private:
	int days;
	double buyThreshold, sellThreshold;

public:

	MomentumStrategy(int days = 10, double buyThreshold = 1.05, double sellThreshold = 0.96) :
		days(days), buyThreshold(buyThreshold), sellThreshold(sellThreshold) {}

	// 生成交易信号：1为买入，-1为卖出，0为持仓不变
	void applyStrategy(const std::vector<Bar>& data, std::vector<int>& signal) override {
		signal.resize(data.size(), 0);
		if (days >= data.size()) return;
		for (size_t i = days; i < data.size(); i++) {
			double momentum = data[i].close / data[i - days].close; // 计算动量，这里简单定义为当前价格除以N天前的价格
			if (momentum > buyThreshold) {
				signal[i] = 1; // buy
			}
			else if (momentum < sellThreshold) {
				signal[i] = -1; // sell
			}
		}
	}
};