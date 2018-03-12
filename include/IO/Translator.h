#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "iolibrary_global.h"

#include "iofunctions.h"
#include <map>
#include <vector>




void cut_dump_page(const std::string & source_dump, const std::string target_dump, int src_page_size , int dst_page_size)
{
	HANDLE hSource = INVALID_HANDLE_VALUE;
	HANDLE hTarget = INVALID_HANDLE_VALUE;

	//const int src_page_size = 9696;
	//const int dst_page_size = 9216;
	if (src_page_size <= 0)
	{
		printf("Error. source page size is 0");
		return;
	}
	if (dst_page_size <= 0)
	{
		printf("Error. target page size is 0");
		return;
	}

	if (!IO::open_read(hSource, source_dump))
	{
		printf("Error open source dump %s\r\n", source_dump.c_str());
		return;
	}
	if (!IO::create_file(hTarget, target_dump))
	{
		printf("Error create target dump %s\r\n", target_dump.c_str());
		return;
	}

	BYTE * source_buffer = new BYTE[src_page_size];
	BYTE *target_buffer = new BYTE[dst_page_size];

	DWORD bytes_read = 0;
	DWORD bytes_written = 0;
	bool bResult = false;
	LONGLONG offset = 0;

	while (true)
	{
		bResult = IO::read_block(hSource, source_buffer, src_page_size, bytes_read);
		if (!bResult || (bytes_read == 0))
		{
			printf("Error read block\r\n");
			break;
		}

		int not_zero_pos = 0;
		for (int iByte = src_page_size - 1; iByte > 0; --iByte)
			if (source_buffer[iByte] != 0)
			{
				not_zero_pos = iByte;
				break;
			}

		ZeroMemory(target_buffer, dst_page_size);
		int copy_offset = not_zero_pos - dst_page_size + 1;
		if (copy_offset > 0)
		{
			memcpy(target_buffer, source_buffer + copy_offset, dst_page_size);
		}
		else
		{
			printf("%lld (bytes). Copy is less 0. Will write only 0x00\r\n",offset);
			memcpy(target_buffer, source_buffer, dst_page_size);
		}

		bResult = IO::write_block(hTarget, target_buffer, dst_page_size, bytes_written);
		if (!bResult || (bytes_written == 0))
		{
			printf("Error write block\r\n");
			break;
		}


		offset += bytes_read;
	}
	delete[] source_buffer;
	delete[] target_buffer;
	CloseHandle(hTarget);
	CloseHandle(hSource);

}


void insertInEachPage(
			const std::string & source_file, 
			const std::string & target_file, 
			const int src_page_size, 
			const int dst_page_size, 
			const int page_count,
			const int block_size)
{
	const int service_size = 32;
	HANDLE hSource = INVALID_HANDLE_VALUE;
	HANDLE hTarget = INVALID_HANDLE_VALUE;

	if (src_page_size <= 0)
	{
		printf("Error. source page size is 0");
		return;
	}
	if (dst_page_size <= 0)
	{
		printf("Error. target page size is 0");
		return;
	}

	if (!IO::open_read(hSource, source_file))
	{
		printf("Error open source dump %s\r\n", source_file.c_str());
		return;
	}
	if (!IO::create_file(hTarget, target_file))
	{
		printf("Error create target dump %s\r\n", target_file.c_str());
		return;
	}

	BYTE * source_buffer = new BYTE[block_size];
	BYTE *target_buffer = new BYTE[block_size];


	auto max_size = IO::getFileSize(hSource);

	LONGLONG offset = 0;
	DWORD bytesRead = 0;
	DWORD bytesWritten = 0;
	bool bResult = false;

	while (offset < max_size)
	{
		bResult = IO::read_block(hSource, source_buffer, block_size, bytesRead);
		if (!bResult || bytesRead == 0)
		{
			printf("Error. Read source file.");
			break;
		}

		ZeroMemory(target_buffer, block_size);
		for (auto iPage = 0; iPage < page_count; ++iPage)
			memcpy(target_buffer + (iPage*dst_page_size), source_buffer + (iPage* src_page_size), src_page_size);
		memcpy(target_buffer + (page_count* dst_page_size), source_buffer + (page_count* src_page_size), service_size);

		bResult = IO::write_block(hTarget, target_buffer, block_size, bytesWritten);
		if (!bResult || bytesRead == 0)
		{
			printf("Error. Write to target file.");
			break;
		}
		offset += bytesRead;
	}



	delete[] source_buffer;
	delete[] target_buffer;


}
namespace Translator
{
class FlyTranslator
{
	typedef std::map<WORD, LONGLONG> Page_Offset;
	typedef std::pair<WORD, LONGLONG> Page_Offset_pair;
	typedef std::map<WORD, Page_Offset > FlashTable;
	typedef std::pair<WORD, Page_Offset > TablePair;


private:
	std::string dump_;
	HANDLE hDump_;
	FlashTable table_;

	DWORD page_size_;
	DWORD data_size_;
	DWORD service_size_;
	WORD block_marker_;
	WORD page_marker_;
public:
	FlyTranslator(const std::string &dump_file)
		: dump_(dump_file)
		, hDump_(INVALID_HANDLE_VALUE)
		, page_size_(0)
		, data_size_(0)
		, service_size_(0)
	{

	}
	void setPageParams(const DWORD page_size, const DWORD data_size, const DWORD service_size)
	{
		page_size_ = page_size;
		data_size_ = data_size;
		service_size_ = service_size;
	}
	void setMarkers(const WORD block_marker, const WORD page_marker)
	{
		block_marker_ = block_marker;
		page_marker_ = page_marker;
	}
	bool open()
	{
		return IO::open_read(hDump_, dump_);
	}
	void close()
	{
		CloseHandle(hDump_);
	}
	void make_table()
	{
		if (!open())
		{
			printf("Error open file");
			return; 
		}

		bool bResult = false;

		BYTE * page_data = new BYTE[page_size_];
		DWORD bytes_read = 0;
		WORD * pBlock_marker = nullptr;
		WORD * pPage_marker = nullptr;
		FlashTable::iterator table_iter;
		LONGLONG offset = 0;
		WORD block_marker = 0;
		WORD page_marker = 0;
		offset = 0;

		while (true)
		{
			IO::set_position(hDump_, offset);
			bResult = IO::read_block(hDump_, page_data, page_size_, bytes_read);
			if ( !bResult)
				break;
			if ( bytes_read == 0)
				break;

			pBlock_marker = (WORD *)&page_data[block_marker_];
			pPage_marker = (WORD *)&page_data[page_marker_];
			block_marker = *pBlock_marker;
			page_marker = *pPage_marker;

			if ((block_marker != 0xFFFF) && (page_marker != 0xFFFF))
			{

				table_iter = table_.find(block_marker);
				if (table_iter != table_.end())
				{
					Page_Offset::iterator page_iter;
					page_iter = table_iter->second.find(page_marker);
					if (page_iter == table_iter->second.end())
						table_iter->second.insert(Page_Offset_pair(page_marker, offset));
					else
						page_iter->second = offset;

				}
				else
				{
					Page_Offset page_table;
					page_table.insert(Page_Offset_pair(*pPage_marker, offset));
					table_.insert(TablePair(*pBlock_marker, page_table));
				}
			}

			offset += bytes_read;
		}

		delete[] page_data;
		close();
	}
	void save_data()
	{
		if (!open())
		{
			printf("Error open file");
			return;
		}
		HANDLE hImage = INVALID_HANDLE_VALUE;
		if ( !IO::create_file(hImage, "d:\\incoming\\fly\\image.dump"))
		{
			printf("Error create file");
			return;
		}
		LONGLONG dump_offset = 0;

		BYTE * data_page = new BYTE[page_size_];
		BYTE * write_page = new BYTE[page_size_];
		DWORD bytesRead = 0;
		DWORD bytesWritten = 0;

		while (true)
		{
			if (!IO::read_block(hDump_, data_page, page_size_, bytesRead))
				break;
			if (bytesRead == 0)
				break;
			memset(write_page, 0xFF, 2112);
			memcpy(write_page, data_page, 2048);
			memcpy(write_page + 2048, data_page + 2048 + 2, 34);
			memcpy(write_page + 2048 + 34 + 4, data_page + 2086, 26);

			if (!IO::write_block(hImage, write_page, bytesRead, bytesWritten))
				break;
			if (bytesWritten == 0)
				break;

		}
		CloseHandle(hImage);
		//FlashTable::iterator table_iter = table_.begin();
		//while (table_iter != table_.end())
		//{
		//	Page_Offset::iterator page_iter = table_iter->second.begin();
		//	while (page_iter != table_iter->second.end())
		//	{
		//		dump_offset = page_iter->second;
		//		IO::set_position(hDump_, dump_offset);
		//		if ( !IO::read_block(hDump_, data_page, data_size_, bytesRead))
		//			break;
		//		if ( bytesRead == 0 )
		//			break;

		//		if ( !IO::write_block(hImage , data_page , bytesRead , bytesWritten))
		//			break;
		//		if (bytesWritten == 0 )
		//			break;

		//		++page_iter;
		//	}

		//	

		//	++table_iter;
		//}
		delete[] data_page;
		close();
	}
};

class iPhone1
{
private:
	HANDLE hRead_;
private:
	struct MarkerData
	{
		DWORD marker1;
		WORD marker2;
		ULONGLONG offset;
	};
public:
	iPhone1( )
		: hRead_ ( INVALID_HANDLE_VALUE )
	{
	}
	~iPhone1( )
	{
		CloseHandle(hRead_);
	}

	bool open( const std::string & file )
	{
		return IO::open_read( hRead_ , file );
	}
	void close( )
	{
		CloseHandle( hRead_ );
	}

	void execute( const std::string & target_file )
	{
		const int PageSize = 4224;
		const int DataSize = 4096;
		const int Offset1 = 0;
		const int Offset2 = 4;
		const int MaxPages = 0x200000;


		HANDLE hWrite  = INVALID_HANDLE_VALUE;
		if ( ! IO::create_file( hWrite , target_file ) )
		{
			printf("Error to create file.\r\n");
			return;
		}

		std::vector< MarkerData* > table;
		table.resize( MaxPages );


		DWORD bytesRead = 0;
		BYTE buffer[PageSize];

		ULONGLONG offset = 0;

		DWORD Marker1 = 0;

		while ( true )
		{
			IO::set_position( hRead_ , offset );
			if ( !IO::read_block( hRead_ , buffer , PageSize , bytesRead ) )
				break;
			if ( bytesRead == 0 )
				break;

			DWORD * pMarker1 = (DWORD *) &buffer[ DataSize ];
			Marker1 = * pMarker1;
			WORD * pMarker2 = (WORD *) &buffer[ DataSize + Offset2 ];

			/*to_big_endian( Marker1 );*/

			if (Marker1 < MaxPages )
			{
				if ( table[ Marker1 ] == nullptr )
				{
					MarkerData * pMarkerData = new MarkerData;
					pMarkerData->marker1 = Marker1;
					pMarkerData->marker2 = * pMarker2;
					pMarkerData->offset = offset;
					table[ Marker1 ] = pMarkerData;

				}
				else
				{
					if ( table[ Marker1 ]->marker1 <= * pMarker1 )
					{
						auto pMarkerData = table[ Marker1 ];
						pMarkerData->marker1 = Marker1;
						pMarkerData->marker2 = * pMarker2;
						pMarkerData->offset = offset;
					}
				}
			}
		
			offset += PageSize;
		}

		offset = 0;

		ULONGLONG target_offset = 0;
		DWORD bytesWritten = 0;

		for ( std::size_t iTable = 0; iTable < table.size() ; ++iTable )
		{
			if ( auto pMarkerData = table[iTable] )
			{
				IO::set_position( hRead_ , pMarkerData->offset );
				if ( !IO::read_block( hRead_ , buffer , DataSize , bytesRead ) )
					break;
				if ( bytesRead == 0 )
					break;

				target_offset = (LONGLONG)pMarkerData->marker1 * (LONGLONG)DataSize;
				IO::set_position( hWrite , target_offset );
				if ( ! IO::write_block(hWrite , buffer , DataSize , bytesWritten ) )
					break;
				if ( bytesWritten == 0 )
					break;

			}

		}



		for ( size_t i = 0 ; i < table.size() ; ++i )
			if( table[i] != nullptr )
			{
				delete table[i];
				table[i] = nullptr;
			}


		CloseHandle( hWrite );
		




	}



};

}


#endif