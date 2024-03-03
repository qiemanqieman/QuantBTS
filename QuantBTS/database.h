#pragma once

#include <string>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include "Data.h"

class StockDataBase
{
public:
	// 连接股票数据库 stock_data.db
	void connect() {
		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
		db.setDatabaseName("stock_data.db");//数据库文件名
		if (!db.open()) {
			qDebug() << "Error: connection with database fail";
		}
		else {
			qDebug() << "Database: connection ok";
		}

		QSqlQuery query;
		//query.exec("DROP TABLE IF EXISTS stock_data");
		query.exec("CREATE TABLE IF NOT EXISTS stock_data ("
			"date INTEGER PRIMARY KEY,"
			"open REAL,"
			"high REAL,"
			"low REAL,"
			"close REAL,"
			"volume INTEGER)");
	}

	// 将stock文件中的数据插入到数据库中
	void insert(const std::string& fileName)
	{
		QFile file(fileName.c_str());
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qDebug() << "Cannot open file for reading";
			return;
		}

		QTextStream in(&file);
		QString line = in.readLine(); // Skip the header line
		while (!in.atEnd()) {
			line = in.readLine();
			QStringList fields = line.split(",");

			QSqlQuery query;
			query.prepare("INSERT OR IGNORE INTO stock_data "
				"(date, open, high, low, close, volume) "
				"VALUES (:date, :open, :high, :low, :close, :volume)");
			query.bindValue(":date", fields.at(0).toInt());
			query.bindValue(":open", fields.at(1).toDouble());
			query.bindValue(":high", fields.at(2).toDouble());
			query.bindValue(":low", fields.at(3).toDouble());
			query.bindValue(":close", fields.at(4).toDouble());
			query.bindValue(":volume", fields.at(5).toInt());

			if (!query.exec()) {
				qDebug() << "error insert into stock_data";
			}
		}
		file.close();
	}

	std::vector<Bar> select(int startDate, int endDate)
	{
		std::vector<Bar> res;
		QSqlQuery query;
		auto sql = QString("SELECT * FROM stock_data WHERE date BETWEEN %1 AND %2")
			.arg(startDate).arg(endDate);
		query.exec(sql);
		while (query.next()) {
			int date = query.value(0).toInt();
			double open = query.value(1).toDouble();
			double high = query.value(2).toDouble();
			double low = query.value(3).toDouble();
			double close = query.value(4).toDouble();
			int volume = query.value(5).toInt();
			res.push_back(Bar({ date, open, high, low, close, volume }));
		}

		return res;
	}
};