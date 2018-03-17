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
int main(int argc, TCHAR **argv)
{
	IO::Finder finder;

	finder.add_extension(L".mov");
	finder.add_extension(L".mp4");
	finder.add_extension(L".mpg");
	finder.add_extension(L".mpeg");
	finder.add_extension(L".mts");
	finder.add_extension(L".avi");
	finder.add_extension(L".vob");
	finder.FindFiles(L"e:\\");
	auto fileList = finder.getFiles();

	for (auto & theFile : fileList)
	{
		//IO::testSignatureMP4(theFile);
		IO::testHeaderToBadSectorKeyword(theFile);
		//	IO::TestEndJpg(theFile);
	}
}