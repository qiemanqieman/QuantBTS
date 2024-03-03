#include "stdafx.h"
#include "QuantBTS.h" // 实现GUI
#include <QtWidgets/QApplication>
#include "database.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QuantBTS w;
	w.showMaximized();

	auto db = StockDataBase();
	db.connect();
	//db.insert("stock.csv");
	
	return app.exec();
}