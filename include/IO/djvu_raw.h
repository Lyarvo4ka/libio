#ifndef DJVU_RAW_H
#define DJVU_RAW_H

#include "AbstractRaw.h"
#include "constants.h"

#include <conio.h>


struct DjvuHeader
{
	char at_t[Signatures::at_t_header_size];
	DWORD size;
	char djv[Signatures::djvu_header_size];
	bool isDjvu() const
	{
		if ( memcmp( at_t , Signatures::at_t_header , Signatures::at_t_header_size ) == 0 )
			if ( memcmp( djv , Signatures::djvu_header , Signatures::djvu_header_size ) == 0 )
				return true;

		return false;
	}
};

const LONGLONG start_sector = 729073328;
const LONGLONG size_sectors = 711602175;

class DjvuRaw
	: public AbstractRaw
{
	
public :
	DjvuRaw( const DWORD drive_number , const std::string & output_folder )
		: AbstractRaw( drive_number )
		, folder_( output_folder )
	{
	
	}

	void execute() override
	{
		if ( ! this->isReady() )
		{
			printf( "Error device isn't ready. \r\nProgram will be closed." ) ;
			_getch();
			return;
		}

		auto drive = this->getHandle();

		BYTE buffer[BLOCK_SIZE];

		LONGLONG start_pos = (LONGLONG) ( start_sector * SECTOR_SIZE );
		LONGLONG scan_size = (LONGLONG) ( size_sectors * SECTOR_SIZE );
		LONGLONG last_pos = start_pos + scan_size;
		LONGLONG pos = start_pos;

		DWORD bytesRead = 0;

		bool bResult = false;

		DWORD sector_offset = 0;

		DWORD counter = 0;
		bool found = false;

		DjvuHeader * pDJVU = nullptr;

		while ( pos < last_pos )
		{
			IO::set_position( *drive , pos );
			bResult = IO::read_block( *drive , buffer , BLOCK_SIZE , bytesRead );
			if ( ( !bResult ) && ( bytesRead != 0 ) )
				break;

			found = false;

			for ( DWORD iSector = 0 ; iSector < bytesRead; ++iSector )
			{
				pDJVU = ( DjvuHeader * ) &buffer[iSector];
				if ( pDJVU->isDjvu() )
				{
					sector_offset = iSector;
					found = true;
					break;
				}
			}

			if ( found )
			{
					LONGLONG source_pos = pos;
					source_pos += sector_offset;
					printf( "Found djvu header: %lld (sectors) \r\n" , IO::toSectors(source_pos) ); 

					to_big_endian( pDJVU->size );

					std::string write_file ( IO::file_path_number( folder_ , counter++ , ".djvu") );

					HANDLE hWriteFile = INVALID_HANDLE_VALUE ;
					if ( IO::create_file( hWriteFile , write_file ) )
					{
						

						if ( IO::write_block_to_file( *drive , source_pos, pDJVU->size + 12 , hWriteFile ) )
						{
							printf( "Write to file %s OK\r\n", write_file.c_str() );
							pos = source_pos;
							DWORD sizeInSectors = pDJVU->size;
							sizeInSectors /= SECTOR_SIZE;
							sizeInSectors *= SECTOR_SIZE;

							pos += sizeInSectors;
							CloseHandle( hWriteFile );
							continue;
						}

						CloseHandle( hWriteFile );
					}

			}
			
			pos += bytesRead;

		}	// end while
	

	}

private:
	std::string folder_;
};

#endif