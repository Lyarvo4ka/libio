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


int main(int argc, TCHAR **argv)
{
	IO::Finder finder;
	
	finder.add_extension(L".mov");
	finder.add_extension(L".mp4");
	//finder.add_extension(L".mpg");
	//finder.add_extension(L".mpeg");
	//finder.add_extension(L".mts");
	//finder.add_extension(L".avi");
	//finder.add_extension(L".vob");
	//finder.add_extension(L".3gp");
	finder.FindFiles(L"g:\\44213\\");
	auto fileList = finder.getFiles();
	
	for (auto & theFile : fileList)
	{
		IO::testSignatureMP4(theFile);
		//IO::testHeaderToBadSectorKeyword(theFile);
		//	IO::TestEndJpg(theFile);
	}
}



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
//
//#include "IO\zbk_medoc.h"
//#include "IO\OneC.h"
//
//const IO::path_string delimiters(L"{},");
//
//int main(int argc, TCHAR **argv)
//{
//	IO::path_string str = 
//LR"("{"_CONST8789",0,
//{"Fields",
//{"_FLD8087","L",0,0,0,"CS"},
//{"_RECORDKEY","B",0,1,0,"CS"}
//},
//{"Indexes",
//{"_CONST8789_BYKEY_B",0,
//{"_RECORDKEY",1}
//}
//},
//{"Recordlock","0"},
//{"Files",38923,0,137205}
//}")";
//	///IO::path_string str = L"{\"Files\",38923,0,137205}";
//	//str.erase(std::remove_if(str.begin(), str.end(), boost::is_any_of(L"\n\t")), str.end());
//	//boost::trim_if(str, boost::is_any_of(delimiters));
//
//	IO::Table1CHandle table1CHandle(L"");
//	auto vec_str = table1CHandle.splitStrings(str, delimiters);
//	auto offsetsList = table1CHandle.getFilesOffset(vec_str);
//
//	//vec_str.erase(std::remove_if(std::begin(vec_str), std::end(vec_str),
//	//	[](IO::path_string  tmp_str) 
//	//	{
//	//	if (tmp_str.empty())s
//	//		return true; 
//	//	if (tmp_str.compare(L"\""))
//	//		return true;
//	//	return false;
//	//	}),
//	//	std::end(vec_str));
//	int k = 0;
//	k = 1;
//
//
//}