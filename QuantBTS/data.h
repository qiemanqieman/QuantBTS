#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

// 数据结构，用于存储单日股票交易数据:包含日期, 开盘价, 最高价, 最低价, 收盘价, 成交量
struct Bar {
	int date; // 格式YYYYMMDD
	double open, high, low, close;
	int volume;
};

// 数据加载（Data Feed）假设我们已经有了一个CSV文件，其中包含股票的日交易数据。
class DataFeed {
public:
	std::vector<Bar> loadData(const std::string& filename) const {
		std::vector<Bar> data;
		std::ifstream file(filename);
		std::string line;

		// Skip header
		std::getline(file, line);

		while (std::getline(file, line)) {
			std::istringstream s(line);
			std::string field;
			Bar bar;

			std::getline(s, field, ','); bar.date = std::stoi(field);
			std::getline(s, field, ','); bar.open = std::stod(field);
			std::getline(s, field, ','); bar.high = std::stod(field);
			std::getline(s, field, ','); bar.low = std::stod(field);
			std::getline(s, field, ','); bar.close = std::stod(field);
			std::getline(s, field, ','); bar.volume = std::stoi(field);

			data.push_back(bar);
		}

		return data;
	}
};
