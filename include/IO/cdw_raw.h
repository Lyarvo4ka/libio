#pragma once

#include "IODevice.h"

namespace IO
{
	const uint8_t cdw_header1[] = { 0x4B , 0x46 };
	const uint8_t cdw_header2[] = { 0x01 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 };
	const uint32_t cdw_header2_offset = 16;
	const uint32_t one_mb =  70 * 1024;




	class cdw_raw
	{
	private:
		IODevice * device_;
		uint32_t block_size_;
	public:
		cdw_raw(IODevice * device)
			: device_(device)
			, block_size_(default_block_size)
		{

		}

		bool isHeader(const uint8_t * data)
		{
			if (memcmp(data, cdw_header1, SIZEOF_ARRAY(cdw_header1)) == 0)
				if (memcmp(data + cdw_header2_offset, cdw_header2, SIZEOF_ARRAY(cdw_header2)) == 0)
					return true;
			return false;
		}
		bool FindHeader(const uint64_t start_offset, uint64_t & header_offset)
		{
			Buffer buffer(block_size_);
			uint32_t bytes_read = 0;

			uint64_t offset = start_offset;

			while (offset < device_->Size())
			{
				device_->setPosition(offset);
				bytes_read = device_->ReadData(buffer.data, buffer.data_size);
				if (bytes_read != buffer.data_size)
				{
					wprintf_s(L"Error read\n");
					return false;
				}

				for (uint32_t iSector = 0; iSector < buffer.data_size; iSector += default_sector_size)
				{
					if (isHeader(buffer.data + iSector))
					{
						header_offset = offset;
						header_offset += iSector;
						wprintf_s(L"Found header \n");
						return true;
					}
				}

				offset += bytes_read;
			}
			return false;
		}

		uint64_t saveToFile(const uint64_t header_offset, path_string file_name , uint64_t &new_header_offset)
		{
			File write_file(file_name);
			if (!write_file.Open(OpenMode::Create))
				return ERROR_OPEN_FILE;

			uint32_t bytes_read = 0;
			uint32_t bytes_written = 0;
			uint32_t bytes_to_read = 0;

			Buffer buffer(default_block_size);

			uint32_t file_offset = 0;
			uint64_t source_offset = header_offset;
			uint64_t size = source_offset + one_mb;
			uint32_t header_count = 0;


			while (source_offset < size)
			{
				bytes_to_read = calcBlockSize(source_offset, size, block_size_);
				device_->setPosition(source_offset);
				bytes_read = device_->ReadData(buffer.data, bytes_to_read);
				if (bytes_read != bytes_to_read)
				{
					wprintf_s(L"Error read\n");
					return ERROR_READ_FILE;
				}

				for (uint32_t iSector = 0; iSector < bytes_read; iSector += default_sector_size)
				{
					if (isHeader(buffer.data + iSector))
					{
						++header_count;
						if (header_count > 1)
						{
							if ( iSector > 0)
								write_file.WriteData(buffer.data, iSector);
							new_header_offset = source_offset;
							new_header_offset += iSector;
							return FOUND_NEW_HEADER;
						}
					}
				}

				bytes_written = write_file.WriteData(buffer.data, bytes_read);
				if (bytes_written != bytes_read)
				{
					wprintf_s(L"Error write\n");
					return ERROR_READ_FILE;
				}
				source_offset += bytes_read;
			}

			return source_offset;
		}

		void execute(const path_string & target_folder)
		{
			if (!device_->Open(OpenMode::OpenRead))
			{
				wprintf_s(L"Error source device wasn't open\n");
				return;
			}
			uint32_t block_size = default_block_size;
			uint64_t offset = 0;
			uint64_t header_offset = 0;
			uint64_t new_header_offset = 0;
			uint32_t counter = 0;
			uint64_t new_offset = 0;


			while (offset < device_->Size())
			{
				if (!FindHeader(offset, header_offset))
				{
					wprintf_s(L"Not found header.\n");
				}

				while(true)
				{
					auto target_file = toFullPath(target_folder, counter++, L".frw");
					new_offset = saveToFile(header_offset, target_file, new_header_offset);
					if (new_offset != FOUND_NEW_HEADER)
						break;

					header_offset = new_header_offset;


				}
				if (new_offset == ERROR_READ_FILE)
					break;

				offset = new_offset;
				offset += default_sector_size;
			}



		}
	};
};
