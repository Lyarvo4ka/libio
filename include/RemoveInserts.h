#ifndef REMOVEINSERTS_H
#define REMOVEINSERTS_H


#include "iolibrary_global.h"

#include "iofunctions.h"
#include <conio.h>
#include "constants.h"

#include <bitset>

const int SKIP_NARKER = 0xFF;
const int BITS_IN_BYTE = 8;


inline void mapToPage( const BYTE * map_data , const int map_size ,  BYTE * page_data , const int page_size)
{
	ZeroMemory( page_data , page_size );

	for ( int iByte = 0; iByte < map_size ; ++iByte )
	{
		if ( map_data[iByte] != 0)
		{
			std::bitset<BITS_IN_BYTE> bits( map_data[iByte] );
			
			for( int iBit = 0; iBit < BITS_IN_BYTE ; ++iBit )
			{
				if ( bits[iBit] != 0 )
					page_data[ iByte*BITS_IN_BYTE + ( BITS_IN_BYTE -  iBit - 1 ) ] = SKIP_NARKER;
			}
		}

	}

}

inline void copyPageWithMap( const BYTE * source , const BYTE * map , BYTE * target , const int page_size )
{
	memset( target , 0xFF , page_size );

	int numToCopy = 0;
	int source_pos = 0;
	int target_pos = 0;

	for ( int iMap = 0 ; iMap < page_size; ++iMap )
	{
		if ( map[iMap] == 0 )
		{
			if ( numToCopy == 0 )
				source_pos = iMap;
			++numToCopy;

		}
		else
		{
			if ( numToCopy > 0 )
			{
				memcpy( target + target_pos, source + source_pos , numToCopy);
				target_pos += numToCopy;
				source_pos = iMap;
				numToCopy = 0;
			}
		}

	}
	if ( numToCopy > 0 )
	{
		memcpy( target + target_pos, source + source_pos , numToCopy);
	}
}

inline bool Read_Page( HANDLE & hFile , BYTE * data , const LONGLONG page_number , const int page_size)
{
	LONGLONG pos = page_number * page_size;
	DWORD bytesRead = 0;
	IO::set_position( hFile , pos );
	if ( IO::read_block( hFile , data , page_size , bytesRead) )
		if ( bytesRead != 0 )
			return true;

	return false;
}

class RemoveInserts
{

	static const int PAGE_SIZE = 9216;
	static const int MAP_PAGE_SIZE = 9216 / BITS_IN_BYTE;

public:
	RemoveInserts( const std::string & source_file , const std::string & bit_map )
		: hSource_( INVALID_HANDLE_VALUE )
		, hBitMap_( INVALID_HANDLE_VALUE )
		, bReady_( false )
	{
		if ( bReady_ = IO::open_read( hSource_ , source_file ) )
			if ( bReady_ = IO::open_read( hBitMap_ , bit_map ) )
			{
				printf( "Files opened OK");
			}

	}

	void execute( const std::string & target_file )
	{
		if ( ! this->bReady_ )
		{
			printf( "Error device isn't ready. \r\nProgram will be closed." ) ;
			_getch();
			return;
		}

		bool read_ok = false;

		LONGLONG page_number = 0;

		BYTE map_data[ MAP_PAGE_SIZE ];
		BYTE source_data[ PAGE_SIZE ];
		BYTE page_data[ PAGE_SIZE ];
		BYTE target_data[ PAGE_SIZE ];

		HANDLE hWrite = INVALID_HANDLE_VALUE;
		DWORD bytes_written = 0;

		if ( ! IO::create_file( hWrite , target_file ) )
		{
			printf( "Error to create file %s\r\n", target_file.c_str() );
			return;
		}

		printf("It\'s working. Relax\r\n");

		while ( true )
		{

			if (! Read_Page( hBitMap_ , map_data , page_number , MAP_PAGE_SIZE ) )
				break;

			mapToPage( map_data , MAP_PAGE_SIZE , page_data , PAGE_SIZE );

			if (! Read_Page( hSource_ , source_data , page_number , PAGE_SIZE ) )
				break;

			copyPageWithMap( source_data , page_data , target_data , PAGE_SIZE );

			if ( !IO::write_block( hWrite , target_data , PAGE_SIZE , bytes_written ) )
				break;
			if ( bytes_written == 0 )
				break;

			++page_number;
		}

		CloseHandle( hWrite );
	}

private:
	HANDLE hSource_;
	HANDLE hBitMap_;
	bool bReady_;
};


//short pair_table[] = {  2 , 138 , 150 , 252 , 274 , 386 , 398 , 430 , 448 , 450 ,
//	486 , 514 , 546 , 576 , 754 , 800 , 898 , 996 , 1172, 1378,
//	1982, 2114, 2138, 2400, 2690, 2692, 2734, 2856, 3064, 3206,
//	3422, 3434, 3442, 3612, 3898, 4122, 4474, 4486, 4496, 4532,
//	4550,4570,4606,4614,4706,5190,5266,5370,5482,5634,6054,6180,6286,6476,6500,6778,6868,6896,6978,7094,7174,7264,7302,7330,7458,7538,7650,7746,8042,8248,8562,8606,8716,9124
//};
//short unpair_table[] = {58,154,266,334,354,490,602,644,646,822,858,906,938,950,1074,1158,1462,1574,1580,1594,1648,1734,1754,1818,1884,1916,2310,2738,2814,2862,2968,3004,3036,3276,3376,3534,3676,3690,3704,3902,4010,4144,4322,5200,5262,5276,5300,5504,5726,5762,5768,5776,6140,6334,6754,6816,6978,7544,7848,8062,8194,8242,8246,8258,8668,8732,8766,8794,8798,8834,9054,9116,9120,9162};

short pair_table[] =	{234, 976, 1214, 3846, 4042, 5266, 5444, 7614, 8584 };
short unpair_table[] =	{114, 266, 690, 2354, 5074, 6094, 7470, 7710, 8136, 8508};

short block_1[] = { 104, 252, 1282, 1586, 1630, 1954, 2186, 2456, 2586, 3074, 3306, 4650, 4756, 4854, 5130, 5642, 5774, 5950, 6146, 6498, 7266, 7274, 7610, 8286, 8400, 8490, 8546, 8558, 8772, 8994 };
short block_2[] = { 548, 862, 1068, 1084, 1218, 1410, 1858, 2018, 2110, 2140, 2542, 2586, 2780, 3106, 3818, 4222, 4416, 4768, 5538, 6848, 7082, 7250, 8274, 8926, 8938, 9112 };
short block_3[] = { 832, 940, 992, 1248, 1510, 2256, 2870, 2928, 3082, 3300, 3518, 3588, 4958, 5140, 5454, 6292, 6522, 7098, 7486, 7648, 7758, 8256, 8612 };
short block_4[] = { 124, 590, 964, 2742, 2762, 4856, 5168, 5548, 5640, 6654, 8574, 8796, 8870, 9000, 9004 };

bool isPairValue( const int iValue )
{
	return ( ( iValue % 2 ) == 0 );
}

short * tableByFour( const int iValue)
{
	int iBlock = iValue % 4;
	switch ( iBlock )
	{
	case 0 : 
		return block_1;
	case 1:
		return block_2;
	case 2:
		return block_3;
	case 3: 
		return block_4;
	};
	return nullptr;
}

int getSizeOfArray(const int iValue)
{
	int iBlock = iValue % 4;
	switch (iBlock)
	{
	case 0:
		return SIZEOF_ARRAY(block_1);
	case 1:
		return SIZEOF_ARRAY(block_2);
	case 2:
		return SIZEOF_ARRAY(block_3);
	case 3:
		return SIZEOF_ARRAY(block_4);

	};
	return -1;
}

void removeBytesInPage( BYTE * source_page , BYTE * target_page , int block_number , int page_size )
{
	short prev_value = 0;
	short copy_size = 0;
	short target_offset = 0;

	
	
	//short * pTable = isPairValue(block_number)  ? pair_table : unpair_table ;
	//int size_table = isPairValue(block_number) ? SIZEOF_ARRAY( pair_table) : SIZEOF_ARRAY( unpair_table );
	short * pTable = tableByFour(block_number);
	int size_table = getSizeOfArray(block_number);

	memset( target_page , 0xFF , page_size );

	for ( short iTable = 0 ; iTable < size_table ; ++iTable )
	{
		copy_size = pTable[iTable] - prev_value;
		if ( copy_size > 0 )
			memcpy( target_page + target_offset , source_page + prev_value , copy_size );

		target_offset += copy_size;

		prev_value = pTable[iTable] + 2;
	}
	
	copy_size = page_size - prev_value;

	if ( copy_size > 0 )
		memcpy( target_page + target_offset , source_page + prev_value , copy_size );
}

void RemoveBytesByBitMap(const std::string & source_file , const std::string targe_file)
{
	HANDLE hSource = INVALID_HANDLE_VALUE;
	HANDLE hTarget = INVALID_HANDLE_VALUE;

	if ( ! IO::open_read( hSource , source_file ) )
	{
		printf("Error open file %s to read.\r\n" , source_file.c_str() );
		return;
	}

	if ( ! IO::create_file( hTarget , targe_file ) )
	{
		printf("Error create file %s.\r\n" , targe_file.c_str() );
		return;
	}

	const int page_size = 9216;
	const int page_count = 258;
	const int block_size = page_size * page_count;

	BYTE block_data[block_size];
	BYTE write_data[block_size];

	DWORD bytesRead = 0;
	DWORD bytesWritten = 0;
	DWORD block_number = 0;

	while (true)
	{
		if ( !IO::read_block(hSource , block_data , block_size , bytesRead))
			break;
		if ( bytesRead == 0 )
			break;

		for ( int iPage = 0 ; iPage < page_count; ++iPage )
			removeBytesInPage( &block_data[ iPage * page_size ] , &write_data[ iPage * page_size ] , block_number , page_size);

		++block_number;

		if ( !IO::write_block( hTarget , write_data , block_size , bytesWritten ) )
			break;
		if ( bytesWritten == 0 )
			break;


	}

	CloseHandle( hSource );
	CloseHandle( hTarget );

}

BYTE change_tetrada(const BYTE tetrada)
{
	BYTE new_tetrada = tetrada;
	new_tetrada &= 0x0F;
	new_tetrada |= 0xD0;
	return new_tetrada;
}

void change_tetrada_dump(const std::string &source_file, const std::string &target_file)
{
	HANDLE hSource = INVALID_HANDLE_VALUE;
	HANDLE hTarget = INVALID_HANDLE_VALUE;
	if (!IO::open_read(hSource, source_file))
	{
		printf("error to open file.\r\n");
		return;
	}
	if (!IO::create_file(hTarget, target_file))
	{
		printf("error to open file.\r\n");
		return;
	}

	LONGLONG offset = 0;
	LONGLONG file_size = IO::getFileSize(hSource);

	const int page_size = 9216;
	const int block_size = page_size * 258;
	const int tetrada_pos = 9040;
	BYTE buffer[block_size];
	DWORD bytesRead = 0;
	DWORD bytesWritten = 0;

	BYTE tmp = 0;
	while (offset < file_size )
	{
		IO::set_position(hSource , offset);
		if ( !IO::read_block(hSource, buffer, block_size, bytesRead) )
			break;
		if ( bytesRead == 0 )
			break;

		BYTE * pByte = buffer + 9040;

		tmp = change_tetrada(*pByte);
		*pByte = tmp;

		IO::set_position(hTarget, offset);
		if (!IO::write_block(hTarget, buffer, block_size, bytesWritten))
			break;
		if (bytesWritten == 0)
			break;

		offset += block_size;
	}

}

#endif 