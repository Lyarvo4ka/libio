#ifndef DVR_RAW_H
#define DVR_RAW_H

#include "AbstractRaw.h"
#include "constants.h"
#include <conio.h>

class DVR_raw
	: public AbstractRaw
{
private:
	std::string folder_;
public:

	DVR_raw( const std::string & file_name , const std::string output_folder )
		: AbstractRaw( file_name )
		, folder_ ( output_folder )
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

		auto hSource = this->getHandle();

		BYTE read_buffer[BLOCK_SIZE];
		bool bReadResult = false;

		DWORD bytesRead = 0;

		LONGLONG pos = 0;
		LONGLONG header_pos = 0;
		DWORD block_offset = 0;

		bool bHeader = false;
		DWORD counter = 0;
		while ( true )
		{
			IO::set_position( *hSource , pos );
			bReadResult = IO::read_block( *hSource , read_buffer , BLOCK_SIZE , bytesRead );
			if ( ( !bReadResult ) && ( bytesRead != 0 ) )
				break;
	
			bHeader = false;

			for ( block_offset = 0 ; block_offset < bytesRead ; block_offset += SECTOR_SIZE )
			{
				if ( memcmp( &read_buffer[block_offset] , Signatures::dvr_header , Signatures::dvr_header_size ) == 0 ) 
				{
					header_pos = pos;
					header_pos += block_offset;
					bHeader = true ;
					printf( "Found dvr_header : %lld\r\n", (LONGLONG) (header_pos / SECTOR_SIZE) );
					break;
				}

			}
			
			if ( bHeader )
			{
				LONGLONG size_pos = header_pos;
				size_pos += 1420;
				DWORD file_size = 0;
				IO::set_position( *hSource , size_pos );
				DWORD bytes_read = 0;

				bReadResult = IO::read_block( *hSource , (BYTE*)&file_size , sizeof( DWORD) , bytes_read );
				if ( ( !bReadResult ) && ( bytesRead != 0 ) )
					break;

				HANDLE hTarget = INVALID_HANDLE_VALUE;
				std::string file_path = IO::file_path_number( folder_ , counter++ , ".dvr" );
				if ( IO::create_file( hTarget , file_path ) )
					if ( IO::write_block_to_file ( *hSource , header_pos , file_size , hTarget ) )
					{
						printf( "Write to file %s OK.\r\n" ,  file_path.c_str() );
						CloseHandle( hTarget );


						pos  = (LONGLONG) (header_pos + SECTOR_SIZE);
						continue;

					}


			}

			pos += bytesRead;
		}

	}
};

#endif