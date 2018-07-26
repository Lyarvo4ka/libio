#include <QtCore/QCoreApplication>

#include <QSqlDatabase>
#include <qdebug.h>
#include <QSqlError>
#include <QSqlQuery>

QString createQueryDropTable(const QString & table_name)
{
	return 	"if exists(select *	from sys.tables	where name = \'"+ table_name + "\'and schema_id = schema_id(\'dbo\')) begin drop table dbo." + table_name +" end";

}

void copyDatabase(QSqlDatabase &src_database, QSqlDatabase & dst_database)
{
	auto srcName = src_database.databaseName();
	auto dstName = dst_database.databaseName();



}

//QString createDatabase

void connectToDatabase(const QString & driverName, const QString & serverInstance , const QString & databaseName)
{

}
#include <QSqlDriver>

class MSSQLDB
{
	QSqlDatabase db_;
public:
	MSSQLDB() {}
	~MSSQLDB() {}

	QString prepareConnectString(const QString &  databaseName, const QString &  password , 
			const QString &  host,const QString &  user)
	{
		QString connectString = "DRIVER = { SQL Server };";
		connectString.append("SERVERNODE=" + host + ";");
		connectString.append("UID=" + user + ";");
		connectString.append("PWD=" + password + ";");

		return connectString;
	}

	//bool open(const QString &  databaseName, const QString &  user,
	//	const QString &  password , const QString &  host )
	//{
	//	db_ = QSqlDatabase::addDatabase("QODBC");

	//	auto connectString = prepareConnectString(databaseName, host, user, password);
	//	db_.setDatabaseName(connectString);
	//	
	//	return db_.open();
	//}

};


int main(int argc, char *argv[])
{
	//QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

	QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
	
	//db.setDatabaseName("Driver=SQL Server;Server=WIN-BD5OT7T92EA\\MSSQL_2008;Database=db2010_backup;");
	//db.setDatabaseName("Driver={SQL Server};");

	db.setDatabaseName("Driver={SQL Server};Database=db2010_backup");
	db.setHostName("WIN-BD5OT7T92EA\MSSQL_2008");
	db.setUserName("sa");
	db.setPassword("Admin123");
	//db.se
	bool ok = db.open();
	//qDebug() << db.lastError().text().to;
	auto errText = db.lastError().text().toStdWString();
	auto native_code = db.lastError().nativeErrorCode();

	auto lastErr = db.lastError();
	//query


	/*
	use TargetBaseName
	go

	if exists (select *	from sys.tables	where name = 'TableName' and schema_id = schema_id('dbo')) begin drop table dbo.TableName end

	SELECT * INTO TargetBaseName.dbo.v8users FROM SoruceBaseName.dbo.v8users

	
	*/

	//QSqlQuery query;
	////auto bOK = query.exec("CREATE TABLE dbo.Products2 (ProductID int PRIMARY KEY NOT NULL,ProductName varchar(25) NOT NULL,Price money NULL,	ProductDescription text NULL) ");
	//auto bOK1 = query.exec("SELECT name FROM sys.Tables");
	//QStringList tableList;
	//while (query.next())
	//{
	//	tableList.append(query.value(0).toString());
	//}

	//for (auto table_name : tableList)
	//{
	//	printf("%s\r\n", table_name.toStdString().c_str());
	//	auto sqlQueryTxt = createQueryDropTable(table_name);
	//	auto drop_result = query.exec(sqlQueryTxt);
	//	int k = 1;
	//	k = 2;
	//}
	//printf("number table = %d\r\n", tableList.size());

	db.close();

	QCoreApplication a(argc, argv);

	return a.exec();
}
