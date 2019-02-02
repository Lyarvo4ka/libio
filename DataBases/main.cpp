#include <QtCore/QCoreApplication>



#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDriver>

#include <qdebug.h>
#include <string>


QString createDropTableQuery(const QString & table_name)
{
	return QString(
		"if exists"
		"(select *	from sys.tables	"
		"where name = \'%1\'and schema_id = schema_id(\'dbo\')) "
		"begin "
		"drop table dbo.%1"
		" end"
	).arg(table_name);
	//return 	"if exists(select *	from sys.tables	where name = \'"+ table_name + "\'and schema_id = schema_id(\'dbo\')) begin drop table dbo." + table_name +" end";
}

QString createCopyTableQuery(const QString &soruce_table, const QString &target_table)
{
	return QString("SELECT * INTO %2.dbo.v8users "
		"FROM %1.dbo.v8users"
	).arg(soruce_table, target_table);

}




class MSSQLDB
{
	QSqlDatabase db_;
public:
	MSSQLDB()
	{
	}
	~MSSQLDB()
	{
		close();
	}
	QStringList getAllTableNames(QSqlDatabase & db)
	{
		QString selectAllTables =
			"SELECT name "
			"FROM sys.Tables";

		if (!db.isOpen())
			return QStringList();

		QStringList tableList;

		auto sql_query = db.exec(selectAllTables);
		while (sql_query.next())
		{
			tableList.append(sql_query.value(0).toString());
		}
		return tableList;
	}
	bool connect(const QString &  host, const QString &  databaseName,
		const QString &  user, const QString &  password)
	{
		//QString connectString = "DRIVER = { SQL Server };";
		//connectString.append("SERVERNODE=" + host + ";");
		//connectString.append("UID=" + user + ";");
		//connectString.append("PWD=" + password + ";");

		db_ = QSqlDatabase::addDatabase("QODBC");
		auto driverAndDatabase = QString("Driver={SQL Server};Database=%1;").arg(databaseName);
		db_.setDatabaseName(driverAndDatabase);
		db_.setHostName(host);
		db_.setUserName(user);
		db_.setPassword(password);

		return db_.open();
	}
	QSqlError lastError()
	{
		return db_.lastError();
	}
	void close()
	{
		if (db_.isOpen())
			db_.close();
	}

};



int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QSqlDatabase db;
	db = QSqlDatabase::addDatabase("QODBC");

	QString connectString = "Driver={SQL Server};"; // Driver can also be {SQL Server Native Client 11.0}
	connectString.append("WIN-BD5OT7T92EA\\MSSQL_2008;");   // Hostname,SQL-Server Instance
	connectString.append("Database=UTP;");  // Schema
	connectString.append("Uid=sa;");           // User
	connectString.append("Pwd=Admin123;");     // Pass
	db.setDatabaseName(connectString);

	bool bOK = db.open();
	if (bOK)
	{
		qDebug() << "UTP Connect OK";
	}
	else
		qDebug() << "Error" << db.lastError().text();


	//MSSQLDB sourceDB;
	//if (!sourceDB.connect("WIN-BD5OT7T92EA\\MSSQL_2008", "UTP", "sa", "Admin123"))
	//{
	//	auto errText = sourceDB.lastError().text().toStdWString();
	//	qDebug() << sourceDB.lastError().text();
	//}
	//else
	//	qDebug() << "UTP Connect OK";

	//MSSQLDB targetDB;
	//if (!targetDB.connect("WIN-BD5OT7T92EA\\MSSQL_2008", "UTP_new", "sa", "Admin123"))
	//{
	//	auto errText = targetDB.lastError().text().toStdWString();
	//}
	//else
	//	qDebug() << "UTP_new Connect OK";


	return a.exec();
}
