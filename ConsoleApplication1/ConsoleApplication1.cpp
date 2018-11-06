// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "IO\XorAnalyzer.h"
#include <boost/lexical_cast.hpp>
#include "IO\Finder.h"
#include "IO/ext4_raw.h"


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

#include "IO/GoPro.h"
#include "IO/vhd.h"

void testMP4_signature(const IO::path_string & folder)
{
		IO::Finder finder;
		finder.add_extension(L".mov");
		finder.add_extension(L".mp4");
		finder.add_extension(L".3gp");
		finder.add_extension(L"..3gpp");
		finder.add_extension(L".m4a");
		//finder.add_extension(L".msdoc");
		finder.FindFiles(folder);
		auto fileList = finder.getFiles();
	
		for (auto & theFile : fileList)
		{
			//IO::removeNullsAlignedToSector(theFile, 4096);
			//IO::testSignatureMP4(theFile);
			try {
				std::wcout << theFile;
				//IO::testHeaderToBadSectorKeyword(theFile);
				IO::testSignatureMP4(theFile);
				std::cout << std::endl;
			}
			catch (IO::Error::IOErrorException & ex)
			{
				const char* text = ex.what();
				std::cout << " Cougth exception " << text;
	
			}
			

		}
}

void findAndTestHeaderToBadSector(const IO::path_string & folder)
{
	IO::Finder finder;
	finder.FindFiles(folder);
	auto fileList = finder.getFiles();
	for (auto & theFile : fileList)
	{
		try 
		{
			IO::testHeaderToBadSectorKeyword(theFile);
		}
		catch (IO::Error::IOErrorException & ex)
		{
			const char* text = ex.what();
			std::cout << " Cougth exception " << text;

		}
	}

}

//int main(int argc, TCHAR **argv) 
//{
//	IO::Finder finder;
//	finder.add_extension(L".mov");
//	finder.add_extension(L".mp4");
//	finder.add_extension(L".mpg");
//	finder.add_extension(L".mpeg");
//	finder.add_extension(L".mts");
//	finder.add_extension(L".avi");
//	finder.add_extension(L".vob");
//	finder.add_extension(L".3gp");
//	//finder.add_extension(L".msdoc");
//	finder.FindFiles(L"g:\\45077\\");
//	auto fileList = finder.getFiles();
//
//	for (auto & theFile : fileList)
//	{
//		//IO::removeNullsAlignedToSector(theFile, 4096);
//		//IO::testSignatureMP4(theFile);
//		try {
//			std::wcout << theFile;
//			IO::testHeaderToBadSectorKeyword(theFile);
//			std::cout << std::endl;
//		}
//		catch (IO::Error::IOErrorException & ex)
//		{
//			const char* text = ex.what();
//			std::cout << " Cougth exception " << text;
//
//		}
//		
//	//	//	IO::TestEndJpg(theFile);
//	}
//
//	return 0;
//}

const char BAD_SECTOR_WINHEX[] = { 0x20 , 0x3F , 0x20 , 0x42 , 0x41 , 0x44 , 0x20 , 0x53 , 0x45 , 0x43 , 0x54 , 0x4F , 0x52 , 0x20 , 0x3F , 0x20 };
const uint32_t BAD_SECTOR_WINHEX_SIZE = SIZEOF_ARRAY(BAD_SECTOR_WINHEX);

const char mpeg_keyword[] = { 0x00, 0x00, 0x01, 0xBA };

void fix_bad_mpeg_file(const IO::path_string & source_name, const IO::path_string & target_name)
{
	auto src_file = IO::makeFilePtr(source_name);
	src_file->OpenRead();

	auto dst_file = IO::makeFilePtr(target_name);
	dst_file->OpenCreate();

	IO::DataArray sector(2048);

	while (true)
	{
		src_file->ReadData(sector.data(), sector.size());
		if (memcmp(sector.data(), BAD_SECTOR_WINHEX, BAD_SECTOR_WINHEX_SIZE) != 0)
		{
			memcpy(sector.data(), mpeg_keyword, 4);
			dst_file->WriteData(sector.data(), sector.size());
		}
		
	}
}

int main(int argc, TCHAR **argv)
{
	
	fix_bad_mpeg_file(LR"(d:\incoming\45153\video.mpg)" ,LR"(d:\incoming\45153\fixed.mpg )");

	//testMP4_signature(L"f:\\45090\\");
	//findAndTestHeaderToBadSector(L"f:\\45090\\");

	/*ext4 _ recovery
	auto src_file = IO::makeFilePtr( L"f:\\md1\\md1.img");
	src_file->OpenRead();
	IO::ext4_raw ext4_recovery(src_file);
	uint64_t inode_offset = 0x748AD81000;//0x773A21D000;
	//0x748DAA9000; -- 300 GB	// depth == 2
	//0x748ED0C000; -- 100 GB entries = 0x22

	IO::path_string target_name = L"k:\\45222\\400_new.qcow2";
	ext4_recovery.Execute(inode_offset, target_name);
*/
	//IO::remove_0xff(L"g:\\backup\\gscu.bin" , L"g:\\backup\\gscu.mdf");
	//IO::fixDBF(L"d:\\PaboTa\\45036\\1c-baza\\to_fix\\1SBKTTL.DBF");
	//IO::Finder finder;
	//finder.add_extension(L".mp4");
	//finder.FindFiles(L"z:\\44455\\result16\\1\\");
	//auto fileList = finder.getFiles();



	//for (auto & theFile : fileList)
	//{
	//	IO::GoProAnalyzer  gpAnalyzer(theFile);
	//	gpAnalyzer.Analyze(theFile);
	//	gpAnalyzer.Close();
	//	std::wcout << theFile.c_str() << L"File analyzed ";

	//	if (gpAnalyzer.isValid())
	//	{
	//		std::wcout << "OK" << std::endl;
	//	}
	//	else
	//	{
	//		std::wcout << "BAD" << std::endl;
	//		fs::rename(theFile, theFile + L".bad_file");

	//	}
	//}
	//IO::JpegTester jpgTest;
	//jpgTest.test_jpeg_files(L"d:\\Public\\jpg_bad\\1\\" , 90);
	//IO::TibExtractor tib(L"g:\\srv_d\\FULL\\Архив(1)_2018_07_21_21_13_39_356D.TIB");
	//tib.extractTo(L"h:\\srv_d_var2");

	//SQLHA
	//IO::path_string tib_file = L"d:\\PaboTa\\43410\\src_file.bin";
	//IO::path_string tib_file = L"g:\\src_file.bin";
	//IO::path_string dst_folder = L"g:\\decompress\\";
	//uint32_t counter = 0;
	//

	//IO::AcronisDecompress acronis_decompressor(tib_file);
	//uint64_t offset = 0;
	//do
	//{
	//	auto file_name = IO::offsetToPath(dst_folder, offset,L".bin");
	//	offset = acronis_decompressor.saveToFile(file_name,offset);

	//} while (offset != 0);

	//auto listDrives = IO::ReadPhysicalDrives();
	//auto physical_drivePtr = listDrives.find_by_number(3);
	//auto disk = new IO::DiskDevice(physical_drivePtr);
	//if (!disk->Open(IO::OpenMode::OpenRead))
	//	return -1;

	//IO::ext2_raw extRaw(disk);
	//extRaw.execute(L"f:\\44501\\");

	//IO::replaceBadsFromOtherFile(L"d:\\incoming\\44518\\hotel3.mdf", L"d:\\incoming\\44518\\hotel3_01.mdf", L"d:\\incoming\\44518\\TARGET\\hotel3.mdf");


	//IO::Finder finder;
	//finder.add_extension(L".mov");
	//finder.add_extension(L".mp4");
	////finder.add_extension(L".mpg");
	////finder.add_extension(L".mpeg");
	////finder.add_extension(L".mts");
	////finder.add_extension(L".avi");
	////finder.add_extension(L".vob");
	////finder.add_extension(L".3gp");
	////finder.add_extension(L".msdoc");
	//finder.FindFiles(L"g:\\44554\\");
	//auto fileList = finder.getFiles();
	//
	//for (auto & theFile : fileList)
	//{
	//	//IO::removeNullsAlignedToSector(theFile, 4096);
	//	IO::testSignatureMP4(theFile);
	////	//IO::testHeaderToBadSectorKeyword(theFile);
	////	//	IO::TestEndJpg(theFile);
	//}
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
//#include "IO\Entropy.h"
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
//	//IO::Table1CHandle table1CHandle(L"");
//	//auto vec_str = table1CHandle.splitStrings(str, delimiters);
//	//auto offsetsList = table1CHandle.getFilesOffset(vec_str);
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
//	//IO::File1C file1C(L"d:\\test_folder\\1C\\1Cv8.1CD", 4096);
//	//file1C.Read();
//	IO::calcNullsForFolder(L"d:\\incoming\\44410\\samples\\file_1Gb\\",32786);
//	int k = 0;
//	k = 1;
//
//
//}