#include <QtCore/QCoreApplication>

#include "../JsonReader/JsonReader.h"

#include "IO/Finder.h"

#include <experimental/filesystem>

using fs = std::experimental::filesystem;

void testMp3(const IO::FileStruct & Mp3FileStruct, const IO::path_string & file_to_test)
{
	IO::File file(file_to_test);
	file.OpenRead();
	auto file_size = file.Size();

	IO::DataArray sector_data(default_sector_size);
	file.ReadData(sector_data);
	file.Close();

	auto bFound = Mp3FileStruct.compareWithAllHeaders(sector_data.data(), sector_data.size());
	
	if (!bFound)
	{
		fs::rename(file_to_test, file_to_test + L".bad_file");
	}
	

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
			testMp3(Mp3FileStruct,theFile);
		}
	}
	


	return a.exec();
}
