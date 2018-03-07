
#include "iofunctions.h"

#include <assert.h>
#include "constants.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

bool IO::open_read( HANDLE & handle , const std::string & path)
{
	handle = CreateFileA( path .c_str(),
						GENERIC_READ ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);
	DWORD dwError = GetLastError();
	return (handle != INVALID_HANDLE_VALUE) ? true : false;		
	
}

bool IO::open_write(HANDLE & handle, const std::string & path)
{
	handle = CreateFileA( path .c_str(),
						GENERIC_READ | GENERIC_WRITE ,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);
	return (handle != INVALID_HANDLE_VALUE) ? true : false;	
}

bool IO::create_file( HANDLE & handle , const std::string & path )
{
	handle = CreateFileA( path .c_str(),
						GENERIC_WRITE ,
						FILE_SHARE_READ,
						NULL,
						CREATE_ALWAYS,
						0,
						NULL);
	return (handle != INVALID_HANDLE_VALUE) ? true : false;	
}

LONGLONG IO::getFileSize(HANDLE & handle)
{
	LARGE_INTEGER liSize;
	::GetFileSizeEx(handle, &liSize);
	return liSize.QuadPart;
}

void IO::set_position( HANDLE & handle , LONGLONG position )
{
	LARGE_INTEGER pos;
	pos.QuadPart = position;

	::SetFilePointerEx( handle , pos , NULL , FILE_BEGIN );
}

bool IO::read_block( HANDLE & handle , BYTE * buffer , DWORD size , DWORD & bytesRead )
{
	assert ( handle != INVALID_HANDLE_VALUE );
	assert( handle != nullptr );
	assert( size != 0 );

	bytesRead = 0;
	return (::ReadFile(handle , buffer , size , &bytesRead , NULL )  ) ? true : false;
}

bool IO::write_block( HANDLE & handle , BYTE * buffer , DWORD size , DWORD & bytesWritten )
{
	assert ( handle != INVALID_HANDLE_VALUE );
	assert(buffer != nullptr);
	assert( size != 0 );

	bytesWritten = 0;
	return (::WriteFile(handle , buffer , size , &bytesWritten , NULL ) ) ? true : false ;
}

bool IO::write_block_to_file( HANDLE & source , LONGLONG source_offset , DWORD block_size, HANDLE & target , LONGLONG write_offset )
{
	assert( source != INVALID_HANDLE_VALUE );
	assert( target != INVALID_HANDLE_VALUE );
	assert( block_size != 0 );

	BYTE read_buffer[ BLOCK_SIZE + SECTOR_SIZE ];
	//BYTE write_buffer[ BLOCK_SIZE ];

	DWORD bytes_read = 0;
	DWORD bytes_written = 0;


	DWORD bytes_to_copy = 0;
	DWORD cur_pos = 0;
	DWORD bytes_to_write = 0;

	bool bReadResult = false;
	bool bWriteResult = false;


	IO::set_position( source , source_offset );

	while( cur_pos < block_size )
	{
		if ( (cur_pos + BLOCK_SIZE) < block_size )
			bytes_to_copy = BLOCK_SIZE;
		else
		{
			bytes_to_copy = ( block_size - cur_pos );
		}

		bytes_to_write = bytes_to_copy;

		if ( bytes_to_copy % SECTOR_SIZE != 0 )
		{
			bytes_to_copy /= SECTOR_SIZE;
			++bytes_to_copy;
			bytes_to_copy *= SECTOR_SIZE;
		}


		bReadResult = IO::read_block(source , read_buffer , bytes_to_copy , bytes_read );
		if ( bytes_read == 0 && !bReadResult )
			return false;

		bWriteResult = IO::write_block( target , read_buffer , bytes_to_write , bytes_written );
		if ( bytes_written == 0 && !bWriteResult )
			return false;

		cur_pos += bytes_read;
	}
	return true;
}

bool IO::read_all(HANDLE & handle, BYTE * buffer, LONGLONG size)
{
	assert( handle != INVALID_HANDLE_VALUE);
	assert(buffer != nullptr);
	assert ( size != 0 );

	LONGLONG cur = 0;
	DWORD bytesRead = 0;
	DWORD bytesToRead = BLOCK_SIZE;


	while (cur < size)
	{
		bytesToRead = IO::BytesToCopy(cur , size , BLOCK_SIZE);
		if ( !IO::read_block(handle , &buffer[cur] , BLOCK_SIZE , bytesRead ) )
			break;

		if ( bytesRead == 0 )
			break;

		cur += bytesRead;
	}
	return true;
}

bool IO::write_all(HANDLE & handle, BYTE * buffer, LONGLONG size)
{
	assert(handle != INVALID_HANDLE_VALUE);
	assert(buffer != nullptr);
	assert(size != 0);

	LONGLONG cur = 0;
	DWORD bytesWritten = 0;
	DWORD bytesToWrite = BLOCK_SIZE;
	//BYTE tmp_buffer[BLOCK_SIZE];


	while (cur < size)
	{
		bytesToWrite = IO::BytesToCopy(cur, size, BLOCK_SIZE);
		//memcpy(tmp_buffer, buffer + cur , bytesToWrite);
		if (!IO::write_block(handle, &buffer[cur] , bytesToWrite, bytesWritten))
			return false;

		if (bytesWritten == 0)
			return false;

		cur += bytesWritten;
	}
	return true;

}


bool IO::copy_to_file( HANDLE & source , LONGLONG source_offset , LONGLONG block_size, HANDLE & target , LONGLONG target_offset )
{
	assert( source != INVALID_HANDLE_VALUE );
	assert( target != INVALID_HANDLE_VALUE );
	assert( block_size != 0 );

	BYTE read_buffer[ BLOCK_SIZE + SECTOR_SIZE ];
	//BYTE write_buffer[ BLOCK_SIZE ];

	DWORD bytes_read = 0;
	DWORD bytes_written = 0;


	DWORD bytes_to_copy = 0;
	LONGLONG cur_pos = 0;
	DWORD bytes_to_write = 0;

	bool bReadResult = false;
	bool bWriteResult = false;


	IO::set_position( source , source_offset );

	while( cur_pos < block_size )
	{
		if ( (cur_pos + BLOCK_SIZE) < block_size )
			bytes_to_copy = BLOCK_SIZE;
		else
		{
			bytes_to_copy = DWORD( block_size - cur_pos );
		}

		bytes_to_write = bytes_to_copy;

		/*	This need only for physical drive/*/
		//if ( bytes_to_copy % SECTOR_SIZE != 0 )
		//{
		//	bytes_to_copy /= SECTOR_SIZE;
		//	++bytes_to_copy;
		//	bytes_to_copy *= SECTOR_SIZE;
		//}


		bReadResult = IO::read_block(source , read_buffer , bytes_to_copy , bytes_read );
		if ( bytes_read == 0 && !bReadResult )
			return false;

		bWriteResult = IO::write_block( target , read_buffer , bytes_to_write , bytes_written );
		if ( bytes_written == 0 && !bWriteResult )
			return false;

		cur_pos += bytes_read;
	}
	return true;
}

DWORD IO::BytesToCopy(LONGLONG current , LONGLONG max_size , DWORD block_size)
{
	return ((current + block_size) < max_size) ? block_size : DWORD(max_size - current);
}

std::string IO::get_extension(const std::string & file_name)
{
	boost::filesystem::path file_path(file_name);
	return file_path.extension().generic_string();
}

std::string IO::file_path_number(const std::string & folder, DWORD number, const std::string & extension)
{
	char buff[10];
	memset(buff,0,10);
	sprintf_s(buff,10,"%.9u", number );
	std::string file_name( folder );

	file_name.append( buff );
	file_name.append(extension);

	return file_name;
}
std::string IO::file_offset_name(const std::string & folder, LONGLONG number, const std::string & extension)
{
	const int buff_size = 20;
	char buff[buff_size];
	memset(buff, 0, buff_size);
	sprintf_s(buff, buff_size, "%llX", number);
	std::string file_name(folder);

	file_name.append(buff);
	file_name.append(extension);

	return file_name;
}
bool IO::isLastBackspace( const std::string str )
{
	if ( !str.empty() )
	{
		char back_space = *str.rbegin();
		if ( back_space == '\\' )
			return true;
	}
		
	return false;
}
void IO::addBackspace( std::string & str )
{
	if ( ! isLastBackspace( str ) )
		str.append("\\");
}
std::string IO::addFolderName( const std::string & folder , std::string new_folder)
{
	std::string result_name( folder );
	if ( !IO::isLastBackspace( result_name ) )
		result_name.append("\\");

	result_name.append( new_folder );
	return result_name;
}

std::string IO::add_folder(const std::string & current_dir, std::string new_folder)
{
	std::string target_folder = current_dir;
	target_folder = IO::addFolderName(target_folder, new_folder);
	if (!boost::filesystem::exists(target_folder))
		boost::filesystem::create_directory(target_folder);

	return target_folder;
}

std::string IO::make_file_path( const std::string & folder , const std::string & file )
{
	std::string new_file( folder );
	if ( ! IO::isLastBackspace( folder ) )
		new_file.append("\\");

	new_file.append( file );
	return new_file;
}

bool IO::isPresentInList( const stringlist & strlist , const std::string & text )
{
	auto str = text;
	boost::algorithm::to_lower(str);
	auto findIter = std::find( strlist.begin() , strlist.end() , str );
	return (findIter != strlist.end() ) ? true : false;
}

std::string IO::numberToString( const int number )
{
	char buff[10];
	memset(buff,0,10);
	sprintf_s(buff,10,"%.5d", number );
	return std::string( buff );
}
LONGLONG IO::toSectors( const LONGLONG data_val)
{
	LONGLONG sectors = data_val;
	sectors /= SECTOR_SIZE;
	return sectors;
}

void IO::read_table( const std::string & table_file ,  tablelist & table_list )
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if ( !IO::open_read( hFile , table_file ) )
		return;

	DWORD file_size = ::GetFileSize( hFile , NULL );
	if ( file_size > 0 )
	{
		BYTE * data = new BYTE[ file_size ];
		DWORD bytes_read = 0;

		if ( !IO::read_block( hFile , data , file_size , bytes_read ) )
			return;

		if ( bytes_read == 0 )
			return ;

		unsigned short table_value = 0;
		unsigned short * pAddr = ( unsigned short * ) data ;


		// set table from memory
		while ( pAddr < ( pAddr + file_size/sizeof( unsigned short ) ) )
		{
			table_value = *pAddr << 8 | *pAddr >>8;

			if ( table_value == 0 )
				break;

			table_list.push_back( table_value );


			++pAddr;
		}
		delete [] data;
	}

	CloseHandle( hFile );
}

void IO::gatherByTable( const std::string & source_file , const std::string & target_file , const tablelist * table_list )
{
	HANDLE hSource = INVALID_HANDLE_VALUE;
	HANDLE hTarget = INVALID_HANDLE_VALUE;

	if ( !IO::open_read( hSource , source_file ) )
		return;

	if ( !IO::create_file( hTarget , target_file ) )
		return;

	const int block_size = 2211840;
	const LONGLONG max_size = 9059696640;


	auto iter = table_list->begin();

	ULONGLONG source_offset = 0;
	ULONGLONG write_offset = 0;

	BYTE data[block_size];
	DWORD bytes_read = 0;
	DWORD bytes_write = 0;
	int iTargetPage = 0;

	auto tableValue = 0;

	while ( iter != table_list->end() )
	{
		tableValue = *iter;
		source_offset =   (ULONGLONG)tableValue * block_size ;
		write_offset =  (ULONGLONG)iTargetPage * block_size ;

		if ( source_offset < max_size )
		{
			IO::set_position( hSource , source_offset );
			if ( IO::read_block( hSource , data , block_size , bytes_read) )
				if ( bytes_read == 0 )
					break;

			IO::set_position( hTarget , write_offset );
			if ( IO::write_block( hTarget , data , block_size , bytes_write ) )
				if ( bytes_write == 0 )
					break;

		}

		++iTargetPage;
		++iter;
	}

	CloseHandle( hSource );
	CloseHandle( hTarget );



}

void IO::replaceBads(const std::string & withBad, const std::string & withoutBad, const std::string & target_file)
{
	DWORD max_size = 0;
	HANDLE hWithBad = INVALID_HANDLE_VALUE;
	HANDLE hWithoutBad = INVALID_HANDLE_VALUE;
	if (!IO::open_read(hWithBad, withBad))
	{
		printf("Error to open file %s",withBad.c_str());
		return;
	}
	if (!IO::open_read(hWithoutBad, withoutBad))
	{
		printf("Error to open file %s", withoutBad.c_str());
		return;
	}

	DWORD size_with_bad = ::GetFileSize( hWithBad , NULL );
	DWORD size_without_bad = ::GetFileSize(hWithoutBad, NULL);

	DWORD min_size = (size_without_bad < size_with_bad) ? size_without_bad : size_with_bad;

	HANDLE hTarget = INVALID_HANDLE_VALUE;
	if ( !IO::create_file(hTarget , target_file ))
	{
		printf("Error to create file %s", target_file.c_str());
		return;
	}

	DWORD bytesRead1 = 0;
	DWORD bytesRead2 = 0;
	DWORD bytesWritten = 0;
	const int block_count = 16;
	const int block_size = SECTOR_SIZE*block_count;
	BYTE buffer1[block_size];
	BYTE buffer2[block_size];
	BYTE target[block_size];

	bool bSecondEnd = false;

	bool bResult = false;

	while (true)
	{
		bResult = IO::read_block(hWithBad, buffer1, block_size, bytesRead1);
		if ( !bResult || (bytesRead1 == 0 ))
			break;

		if (!bSecondEnd)
		{
			bResult = IO::read_block(hWithoutBad, buffer2, block_size, bytesRead2);
			if (!bResult || (bytesRead2 == 0))
				bSecondEnd = true;
		}

		ZeroMemory(target, block_size);
		for (auto iSector = 0; iSector < block_size; iSector += SECTOR_SIZE)
		{
			if (memcmp(buffer1 + iSector, Signatures::bad_sector_header, SIZEOF_ARRAY(Signatures::bad_sector_header) ) != 0 )
				memcpy(target + iSector, buffer1 + iSector, SECTOR_SIZE);
			else
			{
				if (!bSecondEnd)
					if (IO::cmpSectorWithByte(buffer2 + iSector, 0x00))
						memcpy(target + iSector, buffer1 + iSector, SECTOR_SIZE);
					else
						memcpy(target + iSector, buffer2 + iSector, SECTOR_SIZE);
			}

		}
		bResult = IO::write_block(hTarget, target, block_size, bytesWritten);
		if (!bResult || (bytesWritten == 0))
			break;



	}



	CloseHandle(hWithBad);
	CloseHandle(hWithoutBad);
	CloseHandle(hTarget);

}

bool IO::cmpSectorWithByte( const BYTE * data , BYTE byte_value)
{
	for ( int i = 0; i < 512; ++i )
	{
		if ( data[i] != byte_value )
			return false;
	}
	return true;
}

bool IO::isBlockNot00andFF(const BYTE * data, DWORD size)
{
	for (DWORD nByte = 0; nByte < size; ++nByte)
	{
		if (data[nByte] != 0 && data[nByte] != 0xFF)
			return true;
	}
	return false;
}

bool IO::isDataSector(const BYTE * data)
{
	if ( !data )
		return false;

	if ( IO::cmpSectorWithByte( data , 0xFF ) )
		return false;
	
	if ( IO::cmpSectorWithByte( data , 0x00 ) )
		return false;

	return true;
}

void IO::SaveOnlyData( const std::string & source_file , const std::string & target_file )
{
	HANDLE hSource = INVALID_HANDLE_VALUE;
	HANDLE hTarget = INVALID_HANDLE_VALUE;

	if ( ! IO::open_read( hSource , source_file ) )
	{
		printf( "Error open %s to read", source_file.c_str() );
		return;
	}

	if ( ! IO::create_file( hTarget , target_file ) )
	{
		printf( "Error open %s to read", target_file.c_str() );
		return;
	}

	LARGE_INTEGER max_size = { 0 };
	if ( !::GetFileSizeEx(hSource , &max_size) )
	{
		printf("Error to get file size ");
		return;
	}
	if ( max_size.QuadPart == 0 )
	{
		printf("file size == 0");
		return;
		
	}


	const int BlockSize = 256*SECTOR_SIZE;
	BYTE read_data[ BlockSize ] ;
	BYTE write_data[ BlockSize ] ;
	DWORD bytesRead = 0;
	DWORD bytesWritten = 0;
	int write_offset = 0;
	LONGLONG source_offset = 0;
	LONGLONG target_offset = 0;

	while ( true )
	{
		if ( source_offset >= max_size.QuadPart )
			break;

		IO::set_position( hSource , source_offset );
		if ( !IO::read_block( hSource , read_data, BlockSize , bytesRead ) )
			break;
		if ( bytesRead == 0 )
			break;

		write_offset = 0;
		for ( DWORD iSector = 0 ; iSector < bytesRead ; iSector += SECTOR_SIZE )
		{
			if ( IO::isDataSector( &read_data[iSector] ) )
			{
				memcpy( write_data + write_offset , read_data + iSector , SECTOR_SIZE );
				write_offset += SECTOR_SIZE;
			}
		}

		if ( write_offset > 0 )
		{
			if ( !IO::write_block( hTarget , write_data , write_offset , bytesWritten ) )
				break;

		}

		source_offset += bytesRead;
	}

	CloseHandle( hSource );
	CloseHandle( hTarget );

}

//void IO::XorFiles(const std::string &file1, const std::string & file2, const std::string & target_file)
//{
	//HANDLE hFile1 = INVALID_HANDLE_VALUE;
	//HANDLE hFile2 = INVALID_HANDLE_VALUE;
	//HANDLE hTarget = INVALID_HANDLE_VALUE;

	//if ( ! IO::open_read(hFile1 , file1) )
	//{
	//	printf("Error open file: %s\r\n" , file1.c_str() );
	//	return;
	//}
	//if (!IO::open_read(hFile2, file2))
	//{
	//	printf("Error open file: %s\r\n", file2.c_str());
	//	return;
	//}
	//if (!IO::create_file(hTarget, target_file))
	//{
	//	printf("Error open file: %s\r\n", target_file.c_str());
	//	return;
	//}

	//DWORD bytesRead = 0;
	//DWORD bytesWritten = 0;

	//BYTE buff1[BLOCK_SIZE];
	//BYTE buff2[BLOCK_SIZE];
	//BYTE buff_write[BLOCK_SIZE];


	//LONGLONG pos = 0;

	//while (true)
	//{
	//	// file 1
	//	IO::set_position(hFile1 , pos );
	//	if ( !IO::read_block(hFile1 , buff1 , BLOCK_SIZE , bytesRead ) )
	//		break;

	//	if ( bytesRead == 0 )
	//		break;

	//	// file 1
	//	IO::set_position(hFile2, pos);
	//	if (!IO::read_block(hFile2, buff2, BLOCK_SIZE, bytesRead))
	//		break;

	//	if (bytesRead == 0)
	//		break;


	//	for ( DWORD iByte = 0 ; iByte < bytesRead; ++iByte)
	//		buff_write[iByte] = buff1[iByte]^buff2[iByte];

	//	IO::set_position(hTarget, pos);
	//	if (!IO::read_block(hTarget, buff_write, BLOCK_SIZE, bytesRead))
	//		break;

	//	if (bytesRead == 0)
	//		break;


	//}


	//CloseHandle(hFile1);
	//CloseHandle(hFile2);
	//CloseHandle(hTarget);


//}
void IO::JoinWithService(const std::string & data_file, const std::string & service_file, const std::string target_file)
{
	HANDLE hData = INVALID_HANDLE_VALUE;
	HANDLE hService = INVALID_HANDLE_VALUE;
	HANDLE hTarget = INVALID_HANDLE_VALUE;

	const int data_size = 8192;
	const int service_size = 8;
	const int page_size = data_size + service_size;

	if (!IO::open_read(hData, data_file))
	{
		printf("Error open file: %s\r\n", data_file.c_str());
		return;
	}
	if (!IO::open_read(hService, service_file))
	{
		printf("Error open file: %s\r\n", service_file.c_str());
		return;
	}
	if (!IO::create_file(hTarget, target_file))
	{
		printf("Error open file: %s\r\n", target_file.c_str());
		return;
	}

	DWORD bytesRead = 0;
	DWORD bytesWritten = 0;

	BYTE data_buffer[data_size];
	BYTE service_buffer[service_size];
	BYTE page_buffer[page_size];
	bool bResult = false;

	LONGLONG max_size = IO::getFileSize(hData);

	LONGLONG cur = 0;
	while (cur < max_size)
	{

		bResult = IO::read_block(hData , data_buffer , data_size , bytesRead );
		if ( ( bytesRead == 0 ) && !bResult )
			break;

		bytesRead = 0;
		bResult = IO::read_block(hService, service_buffer, service_size, bytesRead);
		if ((bytesRead == 0) && !bResult)
			break;

		memcpy(page_buffer , data_buffer , data_size);
		memcpy(page_buffer + data_size, service_buffer, service_size);

		bResult = IO::write_block(hTarget, page_buffer, page_size, bytesWritten);
		if ((bytesWritten == 0) & !bResult )
			break;

		cur += data_size;

	}



	CloseHandle(hData);
	CloseHandle(hService);
	CloseHandle(hTarget);
}

bool IO::isFileHeader00(const std::string & file_name)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	if ( !IO::open_read(hFile , file_name))
		return false;

	BYTE sector[SECTOR_SIZE];

	LONGLONG file_size = IO::getFileSize(hFile);

	if ( file_size < SECTOR_SIZE)
		return false;

	DWORD bytesRead = 0;
	bool bResult = IO::read_block(hFile, sector, SECTOR_SIZE, bytesRead);
	if ( !bResult )
		return false;
	if ( bytesRead == 0)
		return false;


	CloseHandle(hFile);
	return IO::cmpSectorWithByte(sector, 0x00);
}



const DWORD nameToValue( const char * name  )
{
	return * ( DWORD * ) name;
}
