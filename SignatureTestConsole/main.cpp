#include <QtCore/QCoreApplication>

#include "../JsonReader/JsonReader.h"

#include "IO/Finder.h"


void testMp3(const JsonFileStruct sign_list, const IO::path_string & file_to_test)
{
	IO::File file(file_to_test);
	file.OpenRead();
	auto file_size = file.Size();

	for (auto & file_struct : )
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QString mp3_json("mp3.json");

	QFile file(mp3_json);
	if (!file.open(QIODevice::ReadOnly))
	{
		qInfo() << "Error to open file. \"" << file.fileName() << "\"";
		return -1;
	}

	auto json_str = file.readAll();
	QList<JsonFileStruct> listFileStruct;
	ReadJsonFIle(json_str, listFileStruct);

	IO::Finder finder;
	finder.add_extension(L".mp3");

	finder.FindFiles(LR"(e:\45309\)");
	auto fileList = finder.getFiles();

	if (listFileStruct.empty())
	{
		auto Mp3FileStruct = listFileStruct.first();

		for (auto & theFile : fileList)
		{

		}
	}
	


	return a.exec();
}
