// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <boost/lexical_cast.hpp>
#include "IO/Finder.h"
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
#include "IO/ext2_raw.h"
#include "IO/SignatureTest.h"


void copy_every_1024(const IO::path_string & enc_filename, const IO::path_string & old_filename, const IO::path_string & target_filename)
{
	IO::File enc(enc_filename);
	enc.OpenRead();

	IO::File old(old_filename);
	old.OpenRead();

	IO::File target(target_filename);
	target.OpenCreate();

	const uint32_t max_to_copy = 276738048;

	uint64_t offset = 0;

	const uint32_t blk_size = 4096;
	const uint32_t enc_size = 1024;

	IO::DataArray enc_buffer(blk_size);
	IO::DataArray old_buffer(blk_size);
	IO::DataArray target_buffer(blk_size);

	while (offset < max_to_copy)
	{
		enc.setPosition(offset);
		enc.ReadData(enc_buffer);

		old.setPosition(offset);
		old.ReadData(old_buffer);


		memcpy(target_buffer.data(), old_buffer.data(), enc_size);
		memcpy(target_buffer.data() + enc_size, enc_buffer.data() + enc_size, blk_size - enc_size);
		target.WriteData(target_buffer.data(), target_buffer.size());
		offset += blk_size;
	}

}


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
	std::cout << "Found files " << fileList.size();

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

void findAndTestHeaderToNULLS(const IO::path_string & folder)
{
	IO::Finder finder;
	finder.FindFiles(folder);
	auto fileList = finder.getFiles();
	std::cout << "Found files " << fileList.size();

	for (auto & theFile : fileList)
	{
		try
		{
			IO::testHeaderToNullsKeywords(theFile);
		}
		catch (IO::Error::IOErrorException & ex)
		{
			const char* text = ex.what();
			std::cout << " Cougth exception " << text;

		}
	}



}

const uint8_t TIFF_SING[] = { 0x49 , 0x49 , 0x2A };
const uint8_t JPG_SING[] = { 0xFF , 0xD8 , 0xFF };
const uint8_t AVI_SING[] = { 0x52 , 0x49 , 0x46 , 0x46 };

const uint32_t pic_size = 3;

bool isQtFileFromData(const IO::DataArray & buff)
{
	IO::qt_block_t * pQtBlock = (IO::qt_block_t*)buff.data();
	if (IO::isQuickTime(*pQtBlock))
		return true;
	return false;
}

IO::path_string getExtensionFromRAW(const IO::path_string & filename)
{
	IO::File file(filename);
	file.OpenRead();

	IO::path_string ext_name;

	const uint32_t cmp_size = default_sector_size;

	if (file.Size() >= cmp_size)
	{
		IO::DataArray buff(cmp_size);
		file.ReadData(buff);

		if (memcmp(TIFF_SING, buff.data(), pic_size) == 0)
			ext_name = L".tif";
		else if (memcmp(JPG_SING, buff.data(), pic_size) == 0)
			ext_name = L".jpg";
		else if (isQtFileFromData(buff))
			ext_name = L".mov";
		else if (memcmp(AVI_SING, buff.data(), 4) == 0)
			ext_name = L".avi";
			
	}

	return ext_name;
}


void getExtFromFile(const IO::path_string & folder)
{
	IO::Finder finder;
	finder.FindFiles(folder);
	auto fileList = finder.getFiles();

	for (auto & theFile : fileList)
	{
 		auto new_ext = getExtensionFromRAW(theFile);
		if (!new_ext.empty())
		{
			try {
				fs::rename(theFile, theFile + new_ext);
			}
			catch (const fs::filesystem_error& e)
			{
				std::cout << "Error: " << e.what() << std::endl;
			}


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

#include <iostream>




#include <sqlite_modern_cpp.h>
#include <ctime>
#include <iomanip>
#include "IO/utility.h"

using namespace  sqlite;

void jpg_pdf_end_file_change( const IO::path_string & folder)
{
	IO::Finder finder;
	finder.add_extension(L".jpg");
	finder.add_extension(L".pdf");
	finder.FindFiles(folder);
	auto file_list = finder.getFiles();

	for (auto cur_file : file_list)
	{
		IO::removeNullsFromEndFile(cur_file);
	}

}

void zip_end_file_change(const IO::path_string & folder)
{
	IO::Finder finder;
	finder.add_extension(L".zip");
	finder.FindFiles(folder);
	auto file_list = finder.getFiles();

	for (auto cur_file : file_list)
	{
		IO::removeNullsFromEndZipFile(cur_file, 4096);
	}

}

void rename_mov_file(const IO::path_string  & folder)
{
	IO::Finder finder;
	finder.add_extension(L".mov");
	finder.FindFiles(folder);
	auto file_list = finder.getFiles();

	for (auto cur_file : file_list)
	{

		auto res_name = IO::ReadFileName(cur_file);
		if (!res_name.empty())
		{
			auto named_name = IO::toWString(res_name);
			
			fs::path src_path(cur_file);

			auto folder_path = src_path.parent_path().generic_wstring();
			auto only_name_path = src_path.stem().generic_wstring();
			auto ext = src_path.extension().generic_wstring();

			auto new_file_name = folder_path + L"//" + named_name + L"-" + only_name_path + ext;

			fs::rename(cur_file, new_file_name);
		}
	}

}

#include "IO/FireBird.h"

//template< std::size_t N >
std::vector<std::string> split_buffer(IO::DataArray & data_array)
{
	std::vector<std::string> result;
	auto tmp_size = data_array.size() + 1;
	char * pBuf = new char[tmp_size];
	ZeroMemory(pBuf, tmp_size);
	memcpy(pBuf, data_array.data(), data_array.size());

	for (const char* p = pBuf; p != pBuf + tmp_size; p += result.back().size() + 1)
		result.push_back(p);

	return result;
}

#include <ctype.h>
std::vector<std::string> getFtypNames(const IO::path_string & fileName)
{
	std::wcout << fileName;
	std::cout << std::endl;
	std::vector<std::string> strings;
	
	try
	{
		auto qtFile = IO::makeFilePtr(fileName);
		qtFile->OpenRead();
		IO::QuickTimeRaw qt_raw(qtFile);
		auto ftyp_handle = qt_raw.readQtAtom(0);
		if (ftyp_handle.isValid())
			if (ftyp_handle.compareKeyword(IO::s_ftyp))
			{
				auto ftyp_data = qt_raw.readFtypData(ftyp_handle);
				if (ftyp_data.isValid())
				{
					strings = split_buffer(ftyp_data);

					for (auto & str : strings)
					{
						str.erase(std::remove_if(str.begin(), str.end(), [](char c) { return !isalnum(c); }), str.end());
						if (!str.empty())
							std::cout << str << std::endl;
					}


				}
			}
		std::cout << std::endl;

	}

	catch (std::exception ex)
	{
		std::cout << "Cought exception " << ex.what() << std::endl;
	}
	return strings;
}

void moveQT_files(const IO::path_string  & folder)
{
	IO::Finder finder;
	finder.add_extension(L".mov");
	finder.add_extension(L".mp4");
	finder.FindFiles(folder);
	auto file_list = finder.getFiles();

	for (auto cur_file : file_list)
	{
		auto ftypNames = getFtypNames(cur_file);
		if (ftypNames.size() >= 2)
		{
			auto folder_type = ftypNames.at(1);
			//fs::exists
			try
			{
				IO::path_string new_folder = folder + IO::toWString(folder_type);
				fs::create_directory(new_folder);

				fs::path src_path(cur_file);
				auto file_name = src_path.filename();
				IO::path_string new_file_path = new_folder + L"\\" + file_name.generic_wstring();

				fs::rename(cur_file, new_file_path);


			}
			catch (fs::filesystem_error ex)
			{
				std::cout << ex.what();
			}
		}
	}
}


#include "IO/func_utils.h"
#include "tiffio.h"
//
//
//void ReadTiffRows(const std::string_view fileName)
//{
//	TIFF *tif = TIFFOpen(fileName.data(), "r");
//	//tif_postdecode
//	
//
//	uint32 imagelength;
//	tsize_t scanline;
//	tdata_t buf;
//	uint32 row;
//	uint32 col;
//	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imagelength);
//	scanline = TIFFScanlineSize(tif);
//	buf = _TIFFmalloc(scanline);
//	int read_result = 0;
//	for (row = 0; row < imagelength; row++)
//	{
//		read_result = TIFFReadScanline(tif, buf, row);
//		if (read_result == -1)
//		{
//			std::cout << "Error" << std::endl;
//		}
//
//
//	}
//	_TIFFfree(buf);
//	TIFFClose(tif);
//
//}
//




void ReadTiff(const std::string_view fileName)
{
	TIFF *tif = TIFFOpen(fileName.data(), "r");
	if (!tif)
	{
		std::cout << "Error open tiff"<<std::endl;
		return;
	}

	uint32_t width = 0; 
	uint32_t height = 0;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);    
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height); 

	uint32_t npixels = width * height;
	auto buff_size = npixels * sizeof(uint32_t);
	auto raster = (uint32_t *)_TIFFmalloc(buff_size);
	auto result = TIFFReadRGBAImage(tif, width, height, raster, 0);

	if (result == 0)
	{
		std::cout << "Error" <<std::endl;
	}
	
	BITMAPINFOHEADER bfh = BITMAPINFOHEADER();
	//BITMAPINFO bi;
	bfh.biSize = sizeof(BITMAPINFOHEADER);
	bfh.biHeight = height;
	bfh.biWidth = width;
	bfh.biBitCount = 24;
	bfh.biPlanes = 1;
	bfh.biCompression = BI_RGB;

	const char BM_TEXT[] = "BM";

	BITMAPFILEHEADER bm_header = BITMAPFILEHEADER();
	ZeroMemory(&bm_header, sizeof(BITMAPFILEHEADER));
	memcpy(&bm_header.bfType, BM_TEXT, 2);
	bm_header.bfSize = sizeof(BITMAPFILEHEADER) + bfh.biSize + buff_size;

	const uint32_t bmp_size = 3;

	IO::DataArray write_buff(npixels * bmp_size);

	for (uint32_t iPixel = 0; iPixel < npixels; ++iPixel)
	{
		memcpy(write_buff.data() + iPixel * bmp_size, raster + iPixel, bmp_size);
	}
	IO::File raw_file(L"1.raw");
	raw_file.OpenCreate();
	raw_file.WriteData((IO::ByteArray)raster, buff_size);
	raw_file.Close();


	IO::File bmp_file(L"1.bmp");
	bmp_file.OpenCreate();
	bmp_file.WriteData((IO::ByteArray)&bm_header, sizeof(BITMAPFILEHEADER));
	bmp_file.WriteData((IO::ByteArray)&bfh, bfh.biSize);
	bmp_file.WriteData(write_buff.data() , write_buff.size());
	bmp_file.Close();

	//AnalyzeTiffData(write_buff , width , height);

	_TIFFfree(raster);

}


//void ErrorHandler(thandle_t thdl, const char* ch1, const char* ch2, va_list ap)
//{
//	// ignore errors and warnings (or handle them your own way)
//	int k = 1;
//	k = 2;
//}
//void WarningHandler(thandle_t thdl, const char* ch1, const char* ch2, va_list ap)
//{
//	// ignore errors and warnings (or handle them your own way)
//	int k = 1;
//	k = 2;
//}


#include "IO/analyzer.h"

void test_tiff(const IO::path_string & folder)
{
	IO::Finder finder;
	finder.add_extension(L".cr2");
	finder.add_extension(L".tif");
	finder.FindFiles(folder);
	auto file_list = finder.getFiles();

	for (auto cur_file : file_list)
	{
		std::wcout << cur_file;
		IO::TiffAnalyzer tiff_analyzer;
		tiff_analyzer.analyze(cur_file.data());
		try
		{
			if (tiff_analyzer.isCorupted())
			{
				fs::rename(cur_file, cur_file + L".bad_file");
				std::wcout << " - BAD";
			}
			else
				fs::rename(cur_file, cur_file + L".good");
			std::wcout << " - OK";


		}
		catch (fs::filesystem_error &ex)
		{
			std::cout << ex.what();
		}
		catch (const IO::Error::IOErrorException & error_ex)
		{
			
			std::cout << error_ex.what();
		}
		catch (std::exception& ex)
		{
			std::cout << ex.what(); 
		}

		std::wcout << std::endl;
	}
}

void extract_MiNotes()
{
	IO::path_string folder(LR"(d:\incoming\46112\notes\)");

	try {
		// creates a database file 'dbfile.db' if it does not exists.
		database db(R"(d:\incoming\46112\com.miui.notes\databases\note.db )");

		db << "SELECT modified_date,snippet FROM note_view;"
			//<< 18
			>> [&](long long unix_date, std::string text_content) {
			time_t unix_time = unix_date / 1000;
			struct tm buf;
			auto localTime = localtime_s(&buf, &unix_time);

			std::ostringstream oss;
			oss << std::put_time(&buf, "%Y-%m-%d- %H-%M-%S");
			auto text_date = oss.str();

			std::cout << text_date << std::endl;

			auto target_string = folder + IO::toWString(text_date) + L".txt";
			IO::File txt_file(target_string);
			txt_file.OpenCreate();

			std::string str_to_write = text_content + "\n";

			txt_file.WriteText(str_to_write);

			txt_file.Close();
			//std::cout << unix_date << std::endl;
			//std::cout << text_content << std::endl;
		};
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}


}

const uint8_t enc_start_name[] = { 0x7B , 0x45 , 0x4E , 0x43 , 0x52 , 0x59 , 0x50 , 0x54 , 0x53 , 0x54 , 0x41 , 0x52 , 0x54 , 0x7D };
const uint32_t enc_start_size = SIZEOF_ARRAY(enc_start_name);


uint64_t findMarkerAndGetOffset(const IO::path_string & fileName)
{
	IO::File file(fileName);
	file.OpenRead();

	IO::DataArray buffer(default_block_size);

	uint64_t offset = 0;
	uint32 bytes_to_test = 0;
	while (offset < file.Size())
	{
		bytes_to_test = IO::calcBlockSize(offset, file.Size(), default_block_size);

		file.setPosition(offset);
		file.ReadData(buffer.data() , bytes_to_test);

		for (uint32_t i = 0; i < bytes_to_test; ++i)
		{
			if (memcmp(buffer.data() + i, enc_start_name, enc_start_size) == 0)
			{
				auto marker_pos = offset + i;
				return marker_pos;
			}
		}
		offset += default_block_size;
	}
	return 0;
}

void joinWithOldFile(const IO::path_string & enc_folder, const IO::path_string & old_folder, const IO::path_string & result_folder)
{
	const uint32_t enc_size = 50000;

	IO::Finder finder;
	finder.FindFiles(enc_folder);

	auto files = finder.getFiles();
	for (auto enc_filename : files)
	{
		IO::File enc_file(enc_filename);
		enc_file.OpenRead();
		if (enc_file.Size() < enc_size + 1500)
			continue;

		auto fileName = IO::getFileNameFromPath(enc_filename);
		auto old_filename = IO::addBackSlash(old_folder) + fileName;
		if (!fs::exists(old_filename))
			continue;

		IO::File old_file(old_filename);
		old_file.OpenRead();

		auto copy_size = findMarkerAndGetOffset(enc_filename);

		auto new_filename = IO::addBackSlash(result_folder) + fileName;
		IO::File new_file(new_filename);
		new_file.OpenCreate();

		IO::DataArray dataArray(default_block_size);

		old_file.ReadData(dataArray.data(), enc_size);
		new_file.WriteData(dataArray.data(), enc_size);

		uint64_t offset = enc_size;

		uint64_t bytes_to_write = 0;
		while (offset < copy_size)
		{
			bytes_to_write = IO::calcBlockSize(offset, copy_size, default_block_size);
			enc_file.setPosition(offset);
			enc_file.ReadData(dataArray.data(), bytes_to_write);

			new_file.setPosition(offset);
			new_file.WriteData(dataArray.data(), bytes_to_write);

			offset += bytes_to_write;
		}
		enc_file.Close();
		old_file.Close();
		new_file.Close();







	}



}




#include "IO/Entropy.h"
#include "../FileSystem/fat_fs.h"

void SaveUseFat(const IO::path_string & image_filename, const DWORD start_cluster )
{
	auto file_ptr = IO::makeFilePtr(image_filename);
	file_ptr->OpenRead();

	FileSystem::SectorReader reader = std::make_shared<FileSystem::CSectorReader>(file_ptr, 512);

	FileSystem::MasterBootRecord mbr;
	if (!mbr.open(reader))
		return;

	auto part1 = mbr.getPartition(0);
	//part1->
	FileSystem::FatFileSystem fatFS(reader);
	fatFS.mount(part1);

	FileSystem::FileEntry fileEntry = std::make_shared< FileSystem::FileNode >();
	fileEntry->setCluster(start_cluster);
	fileEntry->OpenFile();
	DWORD bytesRead = 0;
	const uint32_t four_gb = 0xFFFFFFFF;
	BYTE * pMEM = new BYTE[four_gb];
	memset(pMEM, 0xFF, four_gb);
	fatFS.ReadUsingFatTable(fileEntry, pMEM, bytesRead);
	
	IO::path_string target_filename = LR"(d:\PaboTa\46560\test.mov)";

	IO::File write_file(target_filename);
	write_file.OpenCreate();
	if (bytesRead > 0)
		write_file.WriteData(pMEM, bytesRead);
	write_file.Close();
	//uint32_t mem_offset = 0;
	//while (mem_offset < bytesRead)
	//{

	//}


	delete [] pMEM;


	FileSystem::File_Handle file_handle;
}

int _tmain(int argc, TCHAR **argv)
{
	//IO::path_string enc_folder = LR"(d:\PaboTa\46539\enc\DB\)";
	//IO::path_string old_folder = LR"(d:\Public\46539\DB\)";
	//IO::path_string result_folder = LR"(d:\Public\46539\)";

	IO::path_string img_filename = LR"(d:\PaboTa\46560\46560.bin)";
	SaveUseFat(img_filename, 17391);

	//joinWithOldFile(enc_folder , old_folder , result_folder);
	//IO::path_string folder = LR"(y:\$LostFiles\333\)";
	//getExtFromFile(folder);
	//IO::path_string filename = LR"(d:\PaboTa\46218\data\big\DJI_0472.MOV )";

	//uint32_t cluster_size = 32768;

	//IO::calcNullsForFile(filename, cluster_size);
	//IO::calcEntropyForFolder(folder , cluster_size);
	//getExtFromFile(LR"(g:\45786\NoName\FOUND.002\)");

	//extract_MiNotes();

	//if (argc == 2)
	//{
	//	std::wstring folderToTest = argv[1];

	//	setlocale(LC_CTYPE, "ukr");

	//	TIFFSetErrorHandler(NULL);
	//	TIFFSetWarningHandler(NULL);
	//	test_tiff(folderToTest);
	//}


	//std::wstring_view tiff_filename = LR"(f:\cr2\)";
	//IO::TiffAnalyzer tiff_analyzer;
	//tiff_analyzer.analyze(tiff_filename.data());




	// IO::path_string dataFileName	= LR"(d:\incoming\45831\02_01.dump)";
	// IO::path_string servcieFileName = LR"(d:\incoming\45831\02_01_SA.dump)";
	// IO::path_string targetFileName	= LR"(d:\incoming\45831\02_01.result)";

	// IO::joinDataWithService(dataFileName, servcieFileName, targetFileName);
	//moveQT_files(LR"(f:\mp4\)");

	//IO::path_string datafileName = LR"(d:\incoming\45831\02_01.dump)";
	//IO::path_string servicefileName = LR"(d:\incoming\45831\02_01_SA.dump)";
	//IO::path_string targetFileName = LR"(d:\incoming\45831\02_01.dat)";

	//IO::joinDataWithService(datafileName, servicefileName , targetFileName);

	//if (argc == 2)
	//{
	//	std::wstring folderToTest = argv[1];
	//	testMP4_signature(folderToTest);
	//	std::cout << std::endl;
	//	std::cout << "Finished.";
	//	char ch;
	//	std::cin >> ch;
	//}
	//else
	//	std::cout << "Wrong params." << std::endl;



	//IO::path_string enc_file = LR"(d:\PaboTa\45853\emcimed.mdf)";
	//IO::path_string old_file = LR"(d:\PaboTa\45853\emcimed1.mdf)";
	//IO::path_string target_file = LR"(d:\PaboTa\45853\emcimed.mdf.result)";
	//copy_every_1024(enc_file, old_file , target_file);





	//if (argc == 2)
	//{
	//	std::wstring folderToTest = argv[1];
	//	findAndTestHeaderToNULLS(folderToTest);
	//}

//	testMP4_signature(L"f:\\");
	
	//rename_mov_file(LR"(f:\45561_video\)");
	//IO::Signture_Testing(L"f:\\");
	//if (argc == 2)
	//{

	//	IO::path_string folder_to_test(argv[1]);
	//	std::wcout << folder_to_test << std::endl;
	//	findAndTestHeaderToBadSector(folder_to_test);
	//}
	//else
	//{
	//	std::cout << "Wrong params" << std::endl;
	//}

	//auto src_name = LR"(d:\Medoc\ZVIT.FDB )";
	//auto dst_name = LR"(d:\Medoc\\ZVIT.FDB.fixed )";

	//IO::fixFireBirdPages(src_name, dst_name);


	//IO::path_string target_folder(LR"(e:\raw\)");

	//auto listDrives = IO::ReadPhysicalDrives();
	//auto physical_drivePtr = listDrives.find_by_number(2);
	//auto disk = new IO::DiskDevice(physical_drivePtr);
	//if (!disk->Open(IO::OpenMode::OpenRead))
	//	return -1;

	//IO::ext2_raw extRaw(disk);
	//extRaw.execute(target_folder);

	//delete disk;
	//IO::remove_0xff(L"g:\\backup\\gscu.bin", L"g:\\backup\\gscu.mdf");
	//IO::removeNullsFromEndZipFile(LR"(e:\raw\zip\0000003703.zip)");
	//IO::path_string folder(LR"(e:\raw\zip\)");
	//zip_end_file_change(folder);

	/*
	IO::path_string folder(LR"(d:\PaboTa\45130\notes\)");

	try {
		// creates a database file 'dbfile.db' if it does not exists.
		database db(R"(d:\PaboTa\45130\memo.db)");

		db << "SELECT lastModifiedAt,strippedContent FROM memo;"
			//<< 18
			>> [&](long long unix_date , std::string text_content) {
			time_t unix_time = unix_date / 1000;
			struct tm buf;
			auto localTime = localtime_s(&buf, &unix_time);

			std::ostringstream oss;
			oss << std::put_time(&buf, "%Y-%m-%d- %H-%M-%S");
			auto text_date = oss.str();

			std::cout << text_date << std::endl;

			auto target_string = folder + IO::toWString(text_date) + L".txt";
			IO::File txt_file(target_string);
			txt_file.OpenCreate();

			std::string str_to_write = text_content + "\n";

			txt_file.WriteText(str_to_write);

			txt_file.Close();
			//std::cout << unix_date << std::endl;
			//std::cout << text_content << std::endl;
		};
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}



	*/


	///------------------------------------------------------------

	//char str[26];
	//ZeroMemory(&str, 26);

	//ctime_s(str, sizeof( str), &unix_time);
	//std::cout << str;




	//try
	//{
	//	auto tiff_path = R"(d:\test_folder\cr2_bad\2017-12-01 12h22m24s.cr2)";
	//	//TIFFSetErrorHandlerExt(ErrorHandler);
	//	//TIFFSetWarningHandlerExt(WarningHandler);
	//	//ReadTiff(tiff_path);
	//	ReadTiff(tiff_path);

	//}
	//catch (...)
	//{
	//	std::cout << "exception";
	//}
	//int k = 1;
	//k = 2;

	//getExtFromFile(LR"(d:\PaboTa\45331\1\)");

	//fix_bad_mpeg_file(LR"(d:\incoming\45153\video.mpg)" ,LR"(d:\incoming\45153\fixed.mpg )");



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