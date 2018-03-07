#ifndef MLV_RAW_H
#define MLV_RAW_H

#include "constants.h"
#include "iofunctions.h"
#include "AbstractRaw.h"
#include "Finder.h"


const uint32_t mlv_keyword_size = 4;

#pragma pack(1)
struct mlv_block_t
{
	uint8_t block_type[mlv_keyword_size];
	uint32_t block_size;
};
#pragma pack()

const int mlv_struct_size = sizeof(mlv_block_t);

inline bool isMlvBlock(const mlv_block_t * pMlvBlock)
{
	for (auto iKeyword = 0; iKeyword < MLVKeywords::mlv_array_size; ++iKeyword)
		if (memcmp(pMlvBlock, MLVKeywords::mlv_array[iKeyword], mlv_keyword_size) == 0)
			return true;

	return false;
}

uint32_t read_block_drive(HANDLE & hDrive, uint64_t offset, uint8_t * data_buffer, uint32_t read_size)
{
	if (hDrive == INVALID_HANDLE_VALUE)
		return 0;

	DWORD bytesRead = 0;

	uint32_t sector_offset = offset % SECTOR_SIZE;
	uint32_t sector_to_read = (sector_offset + read_size) / SECTOR_SIZE + 1;
	uint32_t bytes_to_read = sector_to_read * SECTOR_SIZE;
	if (bytes_to_read == 0)
		bytes_to_read = SECTOR_SIZE;

	uint8_t * sector_buffer = new uint8_t[bytes_to_read];

	LARGE_INTEGER liPos = { 0 };
	liPos.QuadPart = offset;
	::SetFilePointerEx(hDrive, liPos, NULL , FILE_BEGIN);

	if (::ReadFile(hDrive, sector_buffer, bytes_to_read, &bytesRead, NULL))
		if (bytesRead > 0)
		{
			memcpy(data_buffer, sector_buffer + sector_offset, read_size);
			bytesRead = read_size;
		}

	delete[] sector_buffer;

	return bytesRead;

}

uint32_t read_block_file(HANDLE & hFile, uint64_t offset, uint8_t * data_buffer, uint32_t read_size)
{
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	IO::set_position(hFile, offset);
	DWORD bytesRead = 0;
	if (!IO::read_block(hFile, data_buffer, read_size, bytesRead))
		return 0;

	return bytesRead;

}


void save_only_1in10_mlv_clusters(const std::string & file_name, const std::string & new_file, const int cluster_size)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hWrite = INVALID_HANDLE_VALUE;
	if (!IO::open_read(hFile, file_name))
	{
		printf("Error open file to read\r\n");
		return;
	}
	if (!IO::create_file(hWrite, new_file))
	{
		printf("Error create file \r\n");
		return;
	}

	BYTE * cluster_buffer = new BYTE[cluster_size];

	uint32_t byte_read = 0;
	uint64_t offset = 15677952;
	uint64_t file_size = IO::getFileSize(hFile);
	DWORD bytesWritten = 0;

	while (offset < file_size)
	{
		byte_read = read_block_file(hFile, offset, cluster_buffer, cluster_size);
		if ( byte_read == 0 )
			break;

		if (memcmp(cluster_buffer, MLVKeywords::mlv_array[0], mlv_keyword_size) == 0)
			offset += cluster_size * 9;

		if (!IO::write_block(hWrite, cluster_buffer, cluster_size, bytesWritten))
			break;
		if (bytesWritten == 0)
			break;

		offset += cluster_size;
	}
	
	delete[] cluster_buffer;
	CloseHandle(hFile);
	CloseHandle(hWrite);
}


class MLV_raw
	: public AbstractRaw
{
private:
	std::string target_folder_;
public:
	MLV_raw(DWORD drive_number, const std::string target_folder)
		: AbstractRaw(drive_number)
		, target_folder_(target_folder)
	{

	}
	MLV_raw(const std::string file_name, const std::string target_folder)
		: AbstractRaw(file_name)
		, target_folder_(target_folder)
	{

	}

	void execute() override
	{
		HANDLE *hDrive = this->getHandle();
		const uint64_t drive_max = IO::getFileSize(*hDrive);

		if (!this->isReady())
		{
			printf("Error open drive\r\n");
			return;
		}

		uint32_t counter = 0;

		uint64_t offset = 0;
		uint64_t header_offset = 0;


		DWORD bytesRead = 0;

		

		offset = 0;
		//offset = 0xE4B06EAA00;

		uint64_t next_header_offset = 0;
		uint64_t write_file_size = 0;
		
		header_offset = search_header(hDrive, offset);

		while (true)
		{
			if ( header_offset == ERROR_RESULT )
				break;

			std::string target_name = IO::file_path_number(target_folder_, counter++, ".mlv");

			next_header_offset = search_header(hDrive, header_offset + (uint64_t)SECTOR_SIZE);
			if (next_header_offset == ERROR_RESULT)
			{
				save_raw_mlv(header_offset, write_file_size, target_name);
				break;
			}

			write_file_size = next_header_offset - header_offset;

			save_raw_mlv(header_offset, write_file_size, target_name);
			header_offset = next_header_offset;

			//std::string target_name = IO::file_path_number(target_folder_, counter++, ".mlv");
			//offset = saveMLV(target_name, hDrive, header_offset);
			//offset += SECTOR_SIZE;

		}




	}

	void save_raw_mlv(uint64_t start_offset, uint64_t size, const std::string & target_name)
	{
		HANDLE hTarget = INVALID_HANDLE_VALUE;
		if (!IO::create_file(hTarget, target_name))
		{
			printf("Error create target file\r\n");
			return ;
		}

		IO::copy_to_file(*this->getHandle(),
					 start_offset,
					 size,
					 hTarget);

		CloseHandle(hTarget);
	}

	uint64_t saveMLV(const std::string & target_name,  HANDLE * hDrive, uint64_t header_offset)
	{
		

		HANDLE hTarget = INVALID_HANDLE_VALUE;
		if (!IO::create_file(hTarget, target_name))
		{
			printf("Error create target file\r\n");
			return header_offset;
		}

		uint64_t keyword_offset = header_offset;

		while (true)
		{
			mlv_block_t mlv_blocl_header = { 0 };
			if (uint32_t bytes_read = read_block_file(*hDrive, keyword_offset, (uint8_t*)&mlv_blocl_header, mlv_struct_size))
			{
				if (isMlvBlock((const mlv_block_t*)&mlv_blocl_header))
				{
					//uint32_t full_block_size = mlv_blocl_header.block_size + mlv_struct_size;
					if ((mlv_blocl_header.block_size > 0) && (mlv_blocl_header.block_size < 5*1024*1024))
					{

						uint8_t * block_data = new uint8_t[mlv_blocl_header.block_size];
						DWORD bytesWritten = 0;
						bool bResult = false;
						if (bytes_read = read_block_file(*hDrive, keyword_offset, block_data, mlv_blocl_header.block_size))
						{
							bResult = IO::write_block(hTarget, block_data, bytes_read, bytesWritten);
							if (!bResult || (bytesWritten == 0))
							{
								delete[] block_data;
								printf("Error write to file.\r\n");
								break;
							}

							keyword_offset += bytesWritten;
						}

						delete[] block_data;
					}
					else
						break;
				}
				else
					break;
			}
			else
				break;

		}

		CloseHandle(hTarget);
		keyword_offset /= SECTOR_SIZE;
		keyword_offset *= SECTOR_SIZE;
		return keyword_offset;
	}

	uint64_t search_header(HANDLE * hDrive, uint64_t start_offset)
	{
		uint8_t read_buffer[BLOCK_SIZE];
		DWORD bytesRead = 0;

		uint64_t header_offset = 0;

		while (true)
		{
			bytesRead = read_block_drive(*hDrive, start_offset, read_buffer, BLOCK_SIZE);
			if (bytesRead == 0)
			{
				printf("Error read drive\r\n");
				break;
			}

			for (auto iSector = 0; iSector < BLOCK_SIZE; iSector += SECTOR_SIZE)
			{
				if (memcmp(read_buffer + iSector, MLVKeywords::mlv_array[0], mlv_keyword_size) == 0)
				{
					header_offset = start_offset;
					header_offset += iSector;
					return header_offset;
				}
			}
			start_offset += bytesRead;
		}
		return ERROR_RESULT;
	}

};


enum class Direction { Up , Down };

const uint32_t NOT_FOUND = UINT_MAX - 1;
const char NULL_sign[] = { 'N','U','L','L' };
const char VIDF_sign[] = { 'V','I','D','F' };

class Mlv_repair
{
	HANDLE hFile_;
	uint32_t file_size_;
public:
	void repair_files(const std::string & folder)
	{
		//stringlist mlv_ext = { ".mlv" };
		//FileFinder finder;
		//finder.FindFiles(folder, mlv_ext);

		//auto file_list = finder.getFileNames();
		//for (auto mlv_file : file_list)
		//{
		//	repair_mlv_file(mlv_file);
		//	repair_file_size(mlv_file);
		//}

	}
	void repair_file_size(const std::string & mlv_file)
	{
		hFile_ = INVALID_HANDLE_VALUE;
		if (!IO::open_write(hFile_, mlv_file))
		{
			printf("Error to open file.%s\r\n", mlv_file.c_str());
			return;
		}

		uint32_t new_file_size = 0;

		while (true)
		{
			mlv_block_t mlv_blocl_header = { 0 };
			if (uint32_t bytes_read = read_block_file(hFile_, new_file_size, (uint8_t*)&mlv_blocl_header, mlv_struct_size))
			{
				if (isMlvBlock((const mlv_block_t*)&mlv_blocl_header))
				{
					new_file_size += mlv_blocl_header.block_size;
				}
				else
					break;
			}
			else
				break;

		}

		IO::set_position(hFile_, new_file_size);
		::SetEndOfFile(hFile_);

		CloseHandle(hFile_);
	}
	void repair_mlv_file(const std::string & mlv_file)
	{
		printf("Start repair file %s\r\n", mlv_file.c_str());

		hFile_ = INVALID_HANDLE_VALUE;
		if (!IO::open_write(hFile_, mlv_file))
		{
			printf("Error to open file.%s\r\n", mlv_file.c_str());
			return;
		}
		file_size_ = IO::getFileSize(hFile_);

		uint32_t offset = 0;
		uint32_t marker0x27_offset = 0;
		uint32_t null_offset = 0;
		uint32_t vidf_offset = 0;

		DWORD bytesWritten = 0;

		

		while (true)
		{
			marker0x27_offset = find_by_signature(offset, Signatures::marker_0x27, Signatures::marker_0x27_size); 
			if (marker0x27_offset == NOT_FOUND)
				break;

			printf("Found 0x27 marker.\r\n");

			// search UP to find 'NULL' keyword;
			null_offset = find_by_signature(marker0x27_offset, (const uint8_t *)NULL_sign, SIZEOF_ARRAY(NULL_sign), Direction::Up);
			if ( null_offset == NOT_FOUND)
				break;

			// search DOWN to find 'VIDF' keyword;
			vidf_offset = find_by_signature(marker0x27_offset, (const uint8_t *)VIDF_sign, SIZEOF_ARRAY(VIDF_sign), Direction::Down);
			if ( vidf_offset == NOT_FOUND)
				break;

			uint32_t new_null_size = vidf_offset - null_offset;
			mlv_block_t mlv_block = {0};


			if (read_block_file(hFile_, null_offset, (uint8_t*)&mlv_block, mlv_struct_size) == 0)
				break;

			mlv_block.block_size = new_null_size;

			IO::set_position(hFile_, null_offset);
			if (!IO::write_block(hFile_, (uint8_t*)&mlv_block, mlv_struct_size, bytesWritten))
				break;
			if (bytesWritten == 0)
				break;

			printf("File has been changed...\t\t\t");


			offset = vidf_offset;
		}



		printf("Finished repair file %s\r\n", mlv_file.c_str());

		CloseHandle(hFile_);
	}
	uint32_t find_by_signature(uint32_t offset, const uint8_t * signature, const int sign_size, Direction direction = Direction::Down)
	{
		IO::set_position(hFile_, offset);

		uint32_t new_position = offset;

		BYTE buffer[default_block_size];
		

		while (true)
		{
			if (read_block_file(hFile_, new_position, buffer, default_block_size) == 0)
				break;

			for (int iByte = 0; iByte < default_block_size - sign_size; ++iByte)
			{
				if (memcmp(buffer + iByte, signature, sign_size) == 0)
				{
					return new_position + iByte;
				}
			}


			if (direction == Direction::Down)
				new_position += default_block_size;
			else
			{
				if ( new_position == 0)
					break;

				if (new_position < default_block_size)
					new_position = 0;
				else
					new_position -= default_block_size;
			}


		}

		return NOT_FOUND;
	}
};




#endif