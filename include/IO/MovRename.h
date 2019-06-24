#ifndef MOVRENAME_H
#define MOVRENAME_H
	
#include "iofunctions.h"
#include "constants.h"
#include "FileFinder.h"
#include <iostream>

class MovRename
{
public:
	MovRename()
	{

	}
	void rename_files( const std::string folder )
	{
		FileFinder finder;
		stringlist ext;
		ext.push_back(".mov");
		finder.FindFiles( folder , ext ) ;
		auto fileList = finder.getFileNames();

		auto fileIter = fileList.begin();

		const char Canon_name[]= "Canon";
		const int Canon_offset = 246;
		const int date_offset = 300;
		const int date_size = 19;


		int file_counter = 0;


		while ( fileIter != fileList.end())
		{
			std::string raw_name = *fileIter;

			HANDLE hRead = INVALID_HANDLE_VALUE;

			if ( !IO::open_read(hRead , raw_name ) )
			{
				printf("Error to open_file %s" , raw_name.c_str() );
				break;
			}

			DWORD bytesRead = 0;
			char buffer[SECTOR_SIZE];

			bool bResult = IO::read_block(hRead , (BYTE*)buffer , SECTOR_SIZE , bytesRead );
			if ( !bResult && bytesRead == 0 )
			{
				printf("Error read from file ");
				break;
			}

			CloseHandle(hRead);

			if ( memcmp( Canon_name , &buffer[Canon_offset] , strlen(Canon_name) ) == 0  )
			{
				std::string dateStr = &buffer[date_offset];
				std::replace(dateStr.begin() , dateStr.end() , ':' , '-' );
				dateStr.append("-" + IO::numberToString( file_counter++) );
				
				unsigned found = raw_name.find_last_of("/\\");
				std::string folder = raw_name.substr(0, found );

				std::string new_name = folder + "\\" + dateStr+ ".mov";

				fs::rename( raw_name , new_name );

				int k = 1;
				k = 2;



			}
			else
			{
				printf("Not found Canon signature.");
			}



			++fileIter;
			if ( fileIter == fileList.end() )
				break;
		}
	}
};

#endif