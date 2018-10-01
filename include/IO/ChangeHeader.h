#ifndef CHANGEHEADER_H
#define CHANGEHEADER_H
//
//#include "iofunctions.h"
//#include "FileFinder.h"
//#include "constants.h"
//
//const BYTE new_header[] = { 0x44 , 0x41 , 0x48 , 0x55 , 0x41 , 0x22 , 0x05 , 0x00 , 0x5D , 0xEB , 0x96 , 0x39 , 0xD8 , 0xEB , 0x96 , 0x39 };
//const int new_header_size = 1024;
//
//class HeaderChanger
//{
//public:
//	HeaderChanger()
//	{
//
//	}
//
//	void Execute( const std::string folder )
//	{
//		FileFinder finder;
//		stringlist ext;
//		ext.push_back( ".dav" );
//		finder.FindFiles( folder , ext );
//
//		auto files = finder.getFileNames();
//
//		stringlist::iterator it = files.begin();
//
//		while ( it != files.end() )
//		{
//			this->change_header( *it );
//
//			++it;
//			if ( it == files.end())
//				break;
//		}
//
//		printf("Finished.");
//	}
//
//	void change_header( const std::string file_name )
//	{
//		HANDLE hRead = INVALID_HANDLE_VALUE;
//		if ( !IO::open_read( hRead , file_name ) )
//		{
//			printf( "Error to open file - %s\r\n" , file_name.c_str());
//			return;
//		}
//
//		const int source_header_size = 0x60000;
//
//		LARGE_INTEGER file_size = { 0 };
//		::GetFileSizeEx(hRead , & file_size );
//		if ( file_size.QuadPart == 0 )
//		{
//			printf( "Error source file size is 0.\r\n");
//			return;
//		}
//
//		HANDLE hTemp = INVALID_HANDLE_VALUE;
//		std::string temp_file_name = file_name + ".tmp";
//		if ( !IO::create_file( hTemp , temp_file_name ) )
//		{
//			printf( "Error to create file - %s\r\n" , temp_file_name.c_str());
//			return;
//		}
//
//		DWORD bytesWritten = 0;
//		BYTE target_header[new_header_size];
//		ZeroMemory( target_header , new_header_size );
//		memcpy( target_header , new_header , SIZEOF_ARRAY( new_header ) );
//		bool bResult = IO::write_block(hTemp , target_header , new_header_size , bytesWritten );
//		if ( !bResult || ( bytesWritten == 0) )
//		{
//			printf( "Error source file size is 0.\r\n");
//			return;
//		}
//
//		printf("File %s - " , file_name.c_str());
//		LONGLONG copy_size = file_size.QuadPart - source_header_size;
//		bResult = IO::copy_to_file(hRead , source_header_size , copy_size , hTemp , 1024 );
//		if ( !bResult )
//		{
//			printf( "Error source file size is 0.\r\n");
//			return;
//		}
//		CloseHandle( hRead );
//		CloseHandle( hTemp );
//		boost::filesystem::remove( file_name );
//		boost::filesystem::rename( temp_file_name , file_name );
//		printf("-OK\r\n");
//
//
//	}
//
//private:
//
//};


#endif