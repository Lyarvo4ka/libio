#ifndef IOFUNCTIONS_H
#define IOFUNCTIONS_H

#include "iolibrary_global.h"

#include <windows.h>
#include <string>
#include <list>
#include <vector>

typedef std::list< std::string > stringlist;
typedef std::vector< unsigned short > tablelist;


inline std::string drivePathFromNumber(const DWORD number)
{
	std::string sDrivePath = "\\\\.\\PhysicalDrive";
	CHAR buff[10];
	memset(buff, 0, 10);
	sprintf_s(buff, 10, "%u", number);
	sDrivePath.append(buff);
	return sDrivePath;
}

namespace IO
{
	bool open_read( HANDLE & handle , const std::string & path );
	bool open_write( HANDLE & handle , const std::string & path );
	bool create_file( HANDLE & handle , const std::string & path );
	LONGLONG getFileSize(HANDLE & handle);


	void set_position( HANDLE & handle , LONGLONG position );
	bool read_block( HANDLE & handle , BYTE * buffer , DWORD size , DWORD & bytesRead );
	bool write_block( HANDLE & handle , BYTE * buffer , DWORD size , DWORD & bytesWritten );

	bool read_all(HANDLE & handle, BYTE * buffer, LONGLONG size);
	bool write_all(HANDLE & handle, BYTE * buffer, LONGLONG size);

	bool write_block_to_file( HANDLE & source , 
											   LONGLONG source_offset , 
											   DWORD block_size, 
											   HANDLE & target ,
											   LONGLONG target_offset = 0 );

	bool copy_to_file( HANDLE & source , 
										LONGLONG source_offset , 
										LONGLONG block_size, 
										HANDLE & target ,
										LONGLONG target_offset = 0 );

	DWORD BytesToCopy(LONGLONG current, LONGLONG max_size, DWORD block_size);

	std::string get_extension(const std::string & file_name);
	std::string file_path_number( const std::string & folder , DWORD number , const std::string & extension );
	std::string file_offset_name(const std::string & folder, LONGLONG number, const std::string & extension);

	bool isLastBackspace( const std::string str );
	void addBackspace( std::string & str );
	std::string addFolderName( const std::string & folder , std::string new_folder);
	std::string add_folder(const std::string & current_dir, std::string new_folder);
	std::string make_file_path( const std::string & folder , const std::string & file );

	bool isPresentInList( const std::list< std::string > & strlist , const std::string & text );



	// from 1 to "00001"
	std::string numberToString( const int number );


	LONGLONG toSectors( const LONGLONG data_val );

	void read_table( const std::string & table_file ,  tablelist & table_list );
	void gatherByTable( const std::string & source_file , const std::string & target_file , const tablelist * table_list );

	void replaceBads( const std::string & withBad , const std::string & withoutBad , const std::string & target_file );
	bool cmpSectorWithByte( const BYTE * data , BYTE byte_value);
	bool isBlockNot00andFF(const BYTE * data, DWORD size);
	bool isDataSector( const BYTE * data );
	void SaveOnlyData( const std::string & source_file , const std::string & target_file );
	//void XorFiles( const std::string &file1 , const std::string & file2 , const std::string & target_file);
	void JoinWithService( const std::string & data_file , const std::string & service_file, const std::string target_file);

	bool isFileHeader00( const std::string & file_name);


};

inline void to_big_endian(DWORD & val)
{
	_byteswap_ulong(val);
}
const DWORD nameToValue( const char * name  );


#endif // IO_FUNCTIONS_H