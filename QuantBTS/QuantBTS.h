#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCharts>
#include "ui_QuantBTS.h"

#include "data.h"

class QuantBTS : public QMainWindow
{
	Q_OBJECT // 这个宏启用了Qt的信号和槽机制

public:
	QuantBTS(QWidget* parent = nullptr);
	~QuantBTS();

private:
	Ui::QuantBTSClass ui;

	void createStockChart(const QList<Bar>& data);

private slots:
	void onExecButtonClicked(); // 定义槽函数
};
