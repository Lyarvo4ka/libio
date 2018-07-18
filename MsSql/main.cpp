#include <QtCore/QCoreApplication>

#include <QSqlDatabase>
#include <qdebug.h>
#include <QSqlError>
#include <QSqlQuery>

int main(int argc, char *argv[])
{
	//QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

	QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
	
	//db.setDatabaseName("Driver={SQL Server};Server=10.0.0.155\\MSSQL_2008;Database=testDB;");

	//db.setHostName("WIN-BD5OT7T92EA\MSSQL_2008");
	//db.setDatabaseName("testDB");
	db.setUserName("sa");
	db.setPassword("Admin123");
	bool ok = db.open();
	//qDebug() << db.lastError().text().to;
	auto errText = db.lastError().text().toStdWString();
	auto native_code = db.lastError().nativeErrorCode();

	//auto lastErr = db.lastError();
	//query

	QSqlQuery query;
	//auto bOK = query.exec("CREATE TABLE dbo.Products (ProductID int PRIMARY KEY NOT NULL,ProductName varchar(25) NOT NULL,Price money NULL,	ProductDescription text NULL) ");
	auto bOK = query.exec("SELECT name FROM sys.Tables");
	QStringList tableList;
	while (query.next())
	{
		tableList.append(query.value(0).toString());
	}

	for (auto table_name : tableList)
	{
		printf("%s\r\n", table_name.toStdString().c_str());
	}

	db.close();

	QCoreApplication a(argc, argv);

	return a.exec();
}
