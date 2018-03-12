#include "IO\SignatureTest.h"
#include "IO\constants.h"

#include "boost/filesystem.hpp"
#include "IO\constants.h"

void Signture_Testing(const std::string & folder)
{
	
	//FileFinder finder;
	//stringlist extList = {"*.*"};
	//finder.FindFiles(folder, extList);
	//auto file_list = finder.getFileNames();

	//BYTE buff[SECTOR_SIZE];
	//DWORD bytesRead = 0;

	//bool bResult = false;

	//auto fileIter = file_list.begin();
	//while (fileIter != file_list.end())
	//{
	//	std::string file_name = *fileIter;
	//	HANDLE hFile = INVALID_HANDLE_VALUE;
	//	if (!IO::open_read(hFile, file_name))
	//	{
	//		printf("Error to open file: %s", file_name.c_str());
	//		return;
	//	}
	//	
	//	bResult = IO::read_block(hFile, buff, SECTOR_SIZE, bytesRead);
	//	CloseHandle(hFile);
	//	if (!bResult && ( bytesRead == 0 ) )
	//	{
	//		printf("Error read file.%s" , file_name.c_str());
	//		break;
	//	}


	//	bool isGoodHeader = false;
	//	for (DWORD iByte = 0; iByte < bytesRead; ++iByte)
	//		if ( buff[iByte] != 0x00 )
	//		{
	//			isGoodHeader = true;
	//			break;
	//		}

	//	if (!isGoodHeader)
	//	{
	//		try
	//		{
	//			boost::filesystem::remove(file_name);
	//		}
	//		catch (const boost::filesystem::filesystem_error& e)
	//		{
	//			auto error_str = e.what();
	//			printf("Error remove file %s. \r\n", file_name.c_str() );
	//		}
	//	}


		//if (!isGoodHeader)
		//{
		//	boost::filesystem::rename(file_name, file_name + ".bad_file");
		//}
		//else
		//{
		//	if (memcmp(buff, Signatures::bad_sector_header, SIZEOF_ARRAY(Signatures::bad_sector_header)) == 0)
		//		boost::filesystem::rename(file_name, file_name + ".bad_file");
		//}
//		++fileIter;
//	}
}