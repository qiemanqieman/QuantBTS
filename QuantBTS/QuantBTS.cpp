#include "stdafx.h"
#include "QuantBTS.h"

#include "Backtest.h"
#include "Database.h"
#include "Strategy.h"

QString convertIntToDateStr(int dateInt) {
	QString dateStr = QString::number(dateInt);
	// 假设dateStr是YYYYMMDD格式
	QString year = dateStr.mid(0, 4);
	QString month = dateStr.mid(4, 2);
	QString day = dateStr.mid(6, 2);
	return year + "-" + month + "-" + day;
}

QuantBTS::QuantBTS(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.execute_button, &QPushButton::clicked, this, &QuantBTS::onExecButtonClicked); // 连接信号和槽
}

QuantBTS::~QuantBTS()
{}

void QuantBTS::onExecButtonClicked()
{
	// 在这里执行回测逻辑
	// 例如，加载数据，执行策略，展示结果等

	// 绘制指定时间范围内的股价K线图
	auto startDate = ui.date_start->dateTime().toString("yyyyMMdd").toInt();
	auto endDate = ui.date_end->dateTime().toString("yyyyMMdd").toInt();
	auto db = StockDataBase();
	std::vector<Bar> s = db.select(startDate, endDate);
	QList<Bar> selection(s.begin(), s.end());
	createStockChart(selection);

	// 执行策略
	Strategy* strategy;
	auto strategyName = ui.strategy->currentText();
	if (strategyName == "SMA strategy")  strategy = new SimpleMovingAverageStrategy(5, 20);
	else if (strategyName == "Buy-Hold strategy") strategy = new BuyAndHoldStrategy();
	else if (strategyName == "Momentum strategy") strategy = new MomentumStrategy();
	BackTest backtest;
	backtest.addDataFeed(s);
	backtest.setStrategy(strategy); // short window 5 days, long window 20 days
	backtest.setInitialCapital(1000);
	backtest.setCommission(0.001);
	auto stats = backtest.run().statistics();

	// 展示选定策略在指定时间范围内的回测结果
	//ui.statistics->clear();
	ui.statistics->setItem(0, 0, new QTableWidgetItem("本金"));
	ui.statistics->setItem(0, 1, new QTableWidgetItem(QString::number(1000)));
	ui.statistics->setItem(1, 0, new QTableWidgetItem("费率"));
	ui.statistics->setItem(1, 1, new QTableWidgetItem(QString::number(0.001)));
	int idx = 2;
	for (auto stat : stats) {
		ui.statistics->setItem(idx, 0, new QTableWidgetItem(stat.first.c_str()));
		ui.statistics->setItem(idx, 1, new QTableWidgetItem(QString::number(stat.second)));
		++idx;
	}
}

void QuantBTS::createStockChart(const QList<Bar>& data) {
	// K线图
	QCandlestickSeries* candleSeries = new QCandlestickSeries();
	candleSeries->setIncreasingColor(Qt::red);
	candleSeries->setDecreasingColor(Qt::green);
	QLineSeries* closePriceSeries = new QLineSeries();
	closePriceSeries->setColor(Qt::darkGray);
	// X轴
	QStringList categories;

	for (int i = 0; i < data.size(); i++) {
		const Bar& bar = data.at(i);
		QCandlestickSet* set = new QCandlestickSet(bar.open, bar.high, bar.low, bar.close);
		candleSeries->append(set);
		closePriceSeries->append(i, bar.close); // 使用索引作为X值
		categories << convertIntToDateStr(bar.date); // 日期格式可能需要调整
	}

	// 创建图表
	QChart* chartK = new QChart();
	chartK->legend()->hide();
	chartK->addSeries(candleSeries);
	chartK->addSeries(closePriceSeries);
	chartK->setTitle("Stock K-line Chart");
	chartK->setAnimationOptions(QChart::SeriesAnimations);

	// X轴
	QBarCategoryAxis* axisX = new QBarCategoryAxis();
	axisX->setLabelsVisible(false);
	axisX->append(categories);
	chartK->setAxisX(axisX, candleSeries);
	chartK->setAxisX(axisX, closePriceSeries);

	// Y轴（价格）
	QValueAxis* axisYPrice = new QValueAxis();
	axisYPrice->setTitleText("Price");
	chartK->addAxis(axisYPrice, Qt::AlignLeft);
	candleSeries->attachAxis(axisYPrice);
	closePriceSeries->attachAxis(axisYPrice);

	// 配置图表视图
	ui.k_chart->setChart(chartK);
	ui.k_chart->setRenderHint(QPainter::Antialiasing);

	// 交易量图
	QBarSet* set = new QBarSet("Volume");
	for (int i = 0; i < data.length(); ++i) *set << data[i].volume;

	QBarSeries* series = new QBarSeries();
	series->append(set);

	QChart* chartVolume = new QChart();
	chartVolume->addSeries(series);
	chartVolume->setAnimationOptions(QChart::SeriesAnimations);

	QBarCategoryAxis* axisXVolume = new QBarCategoryAxis();
	// 设置要显示的日期，这里需要计算间隔
	int interval = qMax(1, categories.count() / 5); // 确保至少为1
	QStringList displayDates;
	for (int i = 0; i < categories.count(); i += interval)
		displayDates.append(categories[i]);
	axisXVolume->append(displayDates);
	axisXVolume->setTitleText("Date");
	chartVolume->addAxis(axisXVolume, Qt::AlignBottom);

	QValueAxis* axisYVolume = new QValueAxis();
	axisYVolume->setLabelFormat("%i");
	chartVolume->setAxisY(axisYVolume, series);
	chartVolume->legend()->hide();

	ui.volume_chart->setChart(chartVolume);
	ui.volume_chart->setRenderHint(QPainter::Antialiasing);
}
