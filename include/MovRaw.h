#ifndef MOVRAW_H
#define MOVRAW_H

//#include "AbstractRaw.h"
//#include "constants.h"
//
//#include <conio.h>
//#include <stddef.h>  
//#include <memory>
//
//#include "iolibrary_global.h"
//
//
//#include "AbstractRaw.h"
//
////#define to_big_endian64(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))
//inline void to_big_endian64(uint64_t & val)
//{
//	const int type_size = sizeof(uint64_t);
//	uint8_t * byte_buffer = (uint8_t *)&val;
//	uint8_t temp = 0; 
//	for (int iByte = 0; iByte < type_size / 2; ++iByte)
//	{
//		temp = byte_buffer[iByte];
//		byte_buffer[iByte] = byte_buffer[type_size - iByte-1];
//		byte_buffer[type_size - iByte - 1] = temp;
//	}
//}
//
//class AbstractReader
//{
//public:
//	AbstractReader(){};
//	virtual ~AbstractReader(){};
//
//	virtual bool open() = 0;
//	virtual void close() = 0;
//	virtual int read_data(uint8_t * data, int read_size, uint64_t offset) = 0;
//
//};
//
//class FileReader
//	: public AbstractReader 
//{
//private:
//	HANDLE hFile_;
//	bool bOpened_;
//	std::string file_name_;
//public:
//	FileReader(const std::string & file_name)
//		: hFile_(INVALID_HANDLE_VALUE)
//		, bOpened_(false)
//		, file_name_(file_name)
//	{
//
//	}
//	bool open() override
//	{
//		bOpened_ = IO::open_read(hFile_, file_name_);
//		return bOpened_;
//	}
//	void close()
//	{
//		CloseHandle(hFile_);
//		hFile_ = INVALID_HANDLE_VALUE;
//		bOpened_ = false;
//	}
//	int read_data(uint8_t * data, int read_size, uint64_t offset) override
//	{
//		if (!bOpened_)
//			return 0;
//
//		if (read_size == 0)
//			return 0;
//
//		int bytes_read = 0;
//
//		IO::set_position(hFile_, offset);
//		if (::ReadFile(hFile_, data, read_size, (LPDWORD)&bytes_read, NULL) == FALSE)
//			return 0;
//
//		return bytes_read;
//	}
//
//};
//
//
//class PhysicalReader
//	: public AbstractReader
//{
//private:
//	HANDLE hDrive_;
//	bool bOpened_;
//	int sector_size_;
//	int number_sectors_;
//	uint8_t *buffer_;
//	std::string path_;
//public:
//	PhysicalReader(const int drive_number)
//		: hDrive_(INVALID_HANDLE_VALUE)
//		, bOpened_(false)
//		, sector_size_(default_sector_size)
//		, number_sectors_(defalut_number_sectors)
//		, buffer_(nullptr)
//	{
//		path_ = drivePathFromNumber(drive_number);
//		int buffer_size = number_sectors_*sector_size_;
//		buffer_ = new uint8_t[buffer_size];
//		ZeroMemory(buffer_, buffer_size);
//	}
//	~PhysicalReader()
//	{
//		close();
//		remove_buffer();
//	}
//	bool open() override
//	{
//		bOpened_ = IO::open_read(hDrive_, path_);
//		return bOpened_;
//	}
//	void close()
//	{
//		CloseHandle(hDrive_);
//	}
//	int read_data(uint8_t * data, int read_size, uint64_t offset)
//	{
//		if (!bOpened_)
//			return 0;
//
//		int buffer_size = sector_size_ * number_sectors_;
//
//		DWORD numByteRead = 0;
//		int sector_offset = offset % sector_size_;
//
//		if (sector_offset == 0 && read_size == buffer_size)
//		{
//			IO::set_position(hDrive_, offset);
//			if (::ReadFile(hDrive_, data, buffer_size, &numByteRead, NULL) == TRUE)
//				if (numByteRead > 0)
//					return numByteRead;
//		}
//		else
//		{
//			int sector_to_read = (sector_offset + read_size) / sector_size_ + 1;
//			int bytes_to_read = sector_to_read * sector_size_;
//			if (bytes_to_read > 0)
//			{
//				uint8_t * temp_buffer = new uint8_t[bytes_to_read];
//
//				uint64_t aling_offset = offset / sector_size_;
//				aling_offset *= sector_size_;
//				IO::set_position(hDrive_, aling_offset);
//				if (::ReadFile(hDrive_, temp_buffer, bytes_to_read, &numByteRead, NULL) == TRUE)
//					if (numByteRead > 0)
//					{
//						memcpy(data, temp_buffer + sector_offset, read_size);
//						numByteRead = read_size;
//					}
//				delete[] temp_buffer;
//			}
//		}
//
//		return numByteRead;
//	}
//private:
//	void remove_buffer()
//	{
//		if (buffer_)
//		{
//			delete[] buffer_;
//			buffer_ = nullptr;
//		}
//	}
//
//};
//
//
//bool copy_data_to_file(AbstractReader * pReader, HANDLE * target_file, uint64_t offset, uint64_t copy_size)
//{
//	if (!pReader)
//		return false;
//	if (target_file == INVALID_HANDLE_VALUE)
//		return false;
//
//	int block_size = default_block_size;
//	uint8_t buffer[default_block_size];
//	ZeroMemory(buffer, default_block_size);
//
//	uint64_t cur_pos = 0;
//	uint64_t read_pos = 0;
//	int bytes_to_copy = block_size;
//	DWORD bytesWritten = 0;
//	
//	while (cur_pos < copy_size)
//	{
//		if ((cur_pos + block_size) < copy_size)
//			bytes_to_copy = block_size;
//		else
//			bytes_to_copy = (int)(copy_size - cur_pos);
//
//		read_pos = offset;
//		read_pos += cur_pos;
//		pReader->read_data(buffer, bytes_to_copy, read_pos);
//
//		if (!IO::write_block(*target_file, buffer, bytes_to_copy, bytesWritten) )
//			return false;
//		if (bytesWritten == 0)
//			return false;
//				
//
//		cur_pos += bytesWritten;
//	}
//
//
//	return true;
//}
//
//const int qt_keyword_size = 4;
//#pragma pack(1)
//struct qt_block_t
//{
//	DWORD block_size;
//	char block_type[qt_keyword_size];
//};
//#pragma pack()
//
//
//inline bool isQuickTime(const qt_block_t * pQtBlock)
//{
//	for (auto iKeyword = 0; iKeyword < qt_array_size; ++iKeyword)
//		if (memcmp(pQtBlock->block_type, qt_array[iKeyword], qt_keyword_size) == 0)
//			return true;
//
//	return false;
//}
//
//bool isQuickTimeHeader(const qt_block_t * pQtBlock)
//{
//	for (auto iKeyword = 0; iKeyword < 3; ++iKeyword)
//		if (memcmp(pQtBlock->block_type, qt_array[iKeyword], qt_keyword_size) == 0)
//			return true;
//	return false;
//}
//
//
//class QuickTimeRaw
//{
//private:
//	AbstractReader * readerPtr_;
//public:
//	QuickTimeRaw(const int drive_number)
//	{
//		readerPtr_ = new PhysicalReader(drive_number);
//	}
//	QuickTimeRaw(const std::string & file_name)
//	{
//		readerPtr_ = new FileReader(file_name);
//	}
//	~QuickTimeRaw()
//	{
//		if (readerPtr_)
//		{
//			delete readerPtr_;
//			readerPtr_ = nullptr;
//		}
//	}
//
//	void execute(const std::string & target_folder)
//	{
//
//		int block_size = default_block_size;
//
//		uint64_t offset = 0;
//		uint64_t header_offset = 0;
//		DWORD counter = 0;
//
//		if (!this->readerPtr_->open())
//		{
//			printf("Error to open physical drive\r\n");
//			return;
//		}
//
//		while (true)
//		{
//			header_offset = search_header_offset(offset);
//			if (header_offset == ERROR_RESULT)
//				break;
//
//			std::string target_name = IO::file_path_number(target_folder, counter++, ".mov");
//			offset = save_to_file(header_offset, target_name);
//			offset += default_sector_size;
//
//		}
//	}
//
//	uint64_t save_to_file(uint64_t header_offset, const std::string & target_name)
//	{
//		HANDLE hTarget = INVALID_HANDLE_VALUE;	
//		if (!IO::create_file(hTarget, target_name))
//		{
//			printf("Error create target file\r\n");
//			return header_offset;
//		}
//
//		uint64_t keyword_offset = header_offset;
//
//		bool isBeenMDAT = false;
//
//		while (true)
//		{
//			qt_block_t qt_block = { 0 };
//			int bytes_read = readerPtr_->read_data((uint8_t*)&qt_block, sizeof(qt_block_t), keyword_offset);
//			if (bytes_read == 0)
//				return keyword_offset;
//			if (qt_block.block_size == 0 )
//				break;
//
//			to_big_endian(qt_block.block_size);
//
//			if (!isQuickTime(&qt_block))
//				break;
//
//			if (memcmp(qt_block.block_type, qt_array[2], qt_keyword_size) == 0)
//				isBeenMDAT = true;
//
//			uint64_t write_size = qt_block.block_size;
//
//			if (qt_block.block_size == 1)
//			{
//				uint64_t ext_size = 0;
//				uint64_t ext_size_offset = keyword_offset;
//				ext_size_offset += sizeof(qt_block_t);
//				readerPtr_->read_data((uint8_t*)&ext_size, sizeof(uint64_t), ext_size_offset);
//				to_big_endian64(ext_size);
//				write_size = ext_size;
//			}
//
//			copy_data_to_file(readerPtr_, &hTarget, keyword_offset, write_size);
//
//			keyword_offset += write_size;
//		}
//
//		CloseHandle(hTarget);
//		keyword_offset /= default_sector_size;
//		keyword_offset *= default_sector_size;
//		if (!isBeenMDAT)
//			keyword_offset = header_offset;
//		return keyword_offset;
//
//	}
//
//	uint64_t search_header_offset(uint64_t offset)
//	{
//		int bytes_read = 0;
//		int block_size = default_block_size;
//		uint8_t buffer[default_block_size];
//		uint64_t header_offset = 0;
//
//		while (true)
//		{
//			bytes_read = readerPtr_->read_data(buffer, block_size, offset);
//			if (bytes_read == 0)
//			{
//				printf("Error read drive\r\n");
//				break;
//			}
//
//			for (int iSector = 0; iSector < bytes_read; iSector += default_sector_size)
//			{
//				qt_block_t * pQt_block = (qt_block_t *) &buffer[iSector];
//				if ( isQuickTimeHeader(pQt_block) )
//				{
//					header_offset = offset;
//					header_offset += iSector;
//					return header_offset;
//				}
//			}
//			offset += bytes_read;
//		}
//		return ERROR_RESULT;
//	}
//
//};
//
//#pragma pack( 1 )
//struct QTAtom
//{
//	DWORD qt_size;
//	DWORD qt_type;
//};
//#pragma pack( )
//
//
//
//class QTRaw
//	: public AbstractRaw
//{
//
//
//};
//
//#pragma pack( 1 )
//struct QtHeader
//{
//	BYTE header[Signatures::qt_header_size];
//	BYTE qt__CAEP[8];	// skip this
//	//BYTE wide[4];
//	DWORD data_size;
//	BYTE mdat[Signatures::mdat_size];
//};
//#pragma pack( )
//
//#pragma pack( 1 )
//struct moov_data
//{
//	DWORD moov_size;
//	BYTE moov_text[4];
//};
//#pragma pack( )
//
//
//
//
//
//struct InfoPartData
//{
//	LONGLONG offset;
//	DWORD size;
//
//	LONGLONG getLastPos() const
//	{
//		return (LONGLONG ) ( offset + size );
//	}
//};
//
//
//
//
//
//inline bool isQtHeader( QtHeader * pHeader , BYTE * pSignature )
//{
//	//if ( SIZEOF_ARRAY( pSignature ) != Signatures::qt_header_size )
//	//	return false;
//
//	return ( memcmp( pHeader->header , pSignature , Signatures::qt_header_size ) == 0 )	? true : false;
//
//}
//
//inline bool isMoovText( moov_data * pMoov_data)
//{
//	auto size = Signatures::moov.size();
//	auto data = Signatures::moov.data();
//	//return ( memcmp( pMoov_data->moov_text , data , size == 0 ) ) ? true : false;
//	for ( DWORD i = 0 ; i < size ; ++i )
//	{
//		if ( pMoov_data->moov_text[i] != data[i] ) 
//			return false;
//	}
//
//	return true;
//}


//
//class MovRaw
//	: public AbstractRaw
//{
//private:
//	std::string folder_;
//public:
//	MovRaw(const std::string & file_name , const std::string & output_folder )
//		: AbstractRaw( file_name )
//		, folder_ ( output_folder )
//	{
//	}
//	MovRaw(const DWORD drive_number , const std::string & output_folder )
//		: AbstractRaw( drive_number )
//		, folder_ ( output_folder )
//	{
//	}
//
//	virtual bool cmp_header(const BYTE * data ) 
//	{
//		QtHeader * pQtHeader = ( QtHeader * ) data;
//		return isQtHeader( pQtHeader , ( BYTE *) Signatures::qt_header );
//
//	}
//	bool find_header( const BYTE * data , const int data_size , int & header_offset )
//	{
//			for ( int iSector = 0 ; iSector < data_size ; iSector += SECTOR_SIZE )
//			{
//				if ( cmp_header( &data[iSector] ) )
//				{
//					header_offset = iSector;
//					return true;
//				}
//
//			}
//			return false;
//	}
//
//	DWORD getDataSize( const BYTE * data )
//	{
//		QtHeader * pQtHeader = ( QtHeader * ) data;
//		DWORD size = pQtHeader->data_size;
//		to_big_endian(size);
//		return size;
//	}	
//
//	void show_found_header( LONGLONG offset_bytes )
//	{
//		printf( "Found header : %lld (sectors)\r\n", (LONGLONG) (offset_bytes / SECTOR_SIZE) );
//	}
//
//	void execute() override
//	{
//		if ( ! this->isReady() )
//		{
//			printf( "Error device isn't ready. \r\nProgram will be closed." ) ;
//			_getch();
//			return;
//		}
//
//		auto hSource = this->getHandle();
//
//		BYTE read_buffer[BLOCK_SIZE];
//		const int moov_data_size = sizeof( moov_data ) ;
//		BYTE moov_buffer[1024];
//
//		DWORD bytesRead = 0;
//
//		bool bReadResult = false;
//
//
//		LONGLONG pos = 0x149190000;
//		DWORD mdat_offset = 24;
//		LONGLONG moov_pos = 0;
//
//		DWORD counter = 0;
//
//		bool bHeader = false;
//
//		HANDLE hTarget = INVALID_HANDLE_VALUE;
//		int block_offset = 0;
//		LONGLONG lSector = 0;
//
//		while ( true )
//		{
//			IO::set_position( *hSource , pos );
//			bReadResult = IO::read_block( *hSource , read_buffer , BLOCK_SIZE , bytesRead );
//			if ( ( !bReadResult ) && ( bytesRead != 0 ) )
//				break;
//
//
//			if ( find_header( read_buffer , bytesRead , block_offset ) )
//			{
//				InfoPartData HeaderDataPart;
//				HeaderDataPart.offset = pos + (LONGLONG)block_offset;
//				HeaderDataPart.size = getDataSize( &read_buffer[block_offset]);
//				show_found_header( HeaderDataPart.offset );
//			//}
//
//			//if ( bHeader )
//			//{
//
//				moov_pos = HeaderDataPart.offset;
//				moov_pos += HeaderDataPart.size;
//				moov_pos += mdat_offset;
//				lSector = moov_pos;
//				lSector /= SECTOR_SIZE;
//				lSector *= SECTOR_SIZE;
//				DWORD moov_offset = (DWORD) (moov_pos - lSector);
//
//				IO::set_position( *hSource , lSector );
//				DWORD bytes_read = 0;
//				bReadResult =  IO::read_block( *hSource , moov_buffer , 1024 , bytes_read );
//				if ( ( !bReadResult ) && ( bytes_read != 0 ) )
//					break;
//
//				moov_data * pMoov_data = ( moov_data *) &moov_buffer[moov_offset];
//				if ( isMoovText( pMoov_data ) )
//				{
//					to_big_endian( pMoov_data->moov_size);
//					DWORD file_size = mdat_offset + HeaderDataPart.size /*+ moov_data_size*/ + pMoov_data->moov_size;
//					std::string file_path = IO::file_path_number( folder_ , counter++ , ".mov");
//					if ( IO::create_file( hTarget , file_path ) )
//						if ( IO::write_block_to_file ( *hSource , HeaderDataPart.offset , file_size , hTarget ) )
//						{
//							printf( "Write to file %s OK.\r\n" ,  file_path.c_str() );
//							CloseHandle( hTarget );
//
//							file_size /= SECTOR_SIZE ;
//							file_size *= SECTOR_SIZE;
//							pos  = (LONGLONG) (HeaderDataPart.offset + file_size);
//							continue;
//						}
//						else
//							printf( "Error to write file  %s.\r\n" , file_path.c_str() );
//				}
//				else
//					printf( "Not found moov_text.\r\n" );
//
//			}
//
//			pos += bytesRead;
//		}
//	}
//	std::string folder() const 
//	{
//		return folder_;
//	}
//};
//
//
//class IOLIBRARY_EXPORT CanonMovRaw
//	: public MovRaw
//{
//private:
//	DWORD cluster_size_;
//	LONGLONG offset_;
//public:
//	CanonMovRaw(const std::string & file_name , const std::string & output_folder );
//	CanonMovRaw(const DWORD number , const std::string & output_folder );
//
//	void show_error( const std::string & error_text);
//
//	bool cmp_mdat( const BYTE * data );
//	bool find_mdat( const BYTE * data , const int data_size , int & offset );
//	DWORD getMdatSize( const BYTE * data )
//	{
//		DWORD * pSize = ( DWORD *) data;
//		DWORD mdat_size = *pSize;
//		to_big_endian( mdat_size );
//		return mdat_size;
//	}
//	void setClusterSize(const DWORD cluster_size);
//	void setOffset(const LONGLONG offset);
//	void execute() override;
//
//	bool check_parameters();
//
//};


#endif