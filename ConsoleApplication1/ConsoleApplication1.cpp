// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "IO\XorAnalyzer.h"
#include <boost/lexical_cast.hpp>
#include "IO\Finder.h"


//int main(int argc, CHAR **argv)
//{
//	if (argc == 4)
//	{
//		std::string dump_file = argv[1];
//		std::string xor_file = argv[2];
//		auto xor_size = boost::lexical_cast<uint32_t>(argv[3]);
//
//		IO::XorAnalyzer xorAnalyzer(dump_file);
//		xorAnalyzer.Analize(xor_file, xor_size);
//	}
//	else
//	{
//		printf("Wrong parametrs.\r\n");
//		printf("XorAnalyzer.exe dump.bin xor.bin xor_size\r\n");
//	}
//    return 0;
//}

///////////////////////////////////////////////////


///////////////////////////////////////////////////
/*
#include "IO\ImageWriter.h"

IO::Finder finder;
finder.FindFiles(L"e:\\44016\\MacBook Pro - 123.sparsebundle\\bands\\");
auto fileList = finder.getFiles();

auto drive_list = IO::ReadPhysicalDrives();
auto physical_drive = drive_list.find_by_number(3);
if (!physical_drive)
{
printf("Error Find disk device %d" , 3);
return -1;
}
std::shared_ptr<IO::DiskDevice> disk = std::make_shared<IO::DiskDevice>(physical_drive);
IO::FileOffsetWriter offsetWriter(disk);
offsetWriter.setBlockSize(8388608);

for (auto & theFile : fileList)
{
offsetWriter.saveFileToDisk(theFile);
}

*/


//int main(int argc, TCHAR **argv)
//{
//	IO::Finder finder;
//	
//	finder.add_extension(L".mov");
//	finder.add_extension(L".mp4");
//	//finder.add_extension(L".mpg");
//	//finder.add_extension(L".mpeg");
//	//finder.add_extension(L".mts");
//	//finder.add_extension(L".avi");
//	//finder.add_extension(L".vob");
//	finder.add_extension(L".3gp");
//	finder.FindFiles(L"e:\\44176\\");
//	auto fileList = finder.getFiles();
//	
//	for (auto & theFile : fileList)
//	{
//		IO::testSignatureMP4(theFile);
//		//IO::testHeaderToBadSectorKeyword(theFile);
//		//	IO::TestEndJpg(theFile);
//	}
//}



//#include "IO\Translator.h"
//
//int main(int argc, TCHAR **argv)
//{
//	IO::NumberTranslator numTranslator;
//
//	auto drive_list = IO::ReadPhysicalDrives();
//	auto physical_drive = drive_list.find_by_number(4);
//	if (!physical_drive)
//	{
//		printf("Error Find disk device %d", 4);
//		return -1;
//	}
//	std::shared_ptr<IO::DiskDevice> disk = std::make_shared<IO::DiskDevice>(physical_drive);
//
//	numTranslator.execute(L"e:\\44115\\", disk);
//}

#include "IO\zbk_medoc.h"

int main(int argc, TCHAR **argv)
{
	IO::path_string src_folder = L"e:\\44157\\src\\";
	IO::path_string dst_folder = L"e:\\44157\\NoName\\zbk\\";
	IO::renameToDateTime(src_folder,dst_folder);
}