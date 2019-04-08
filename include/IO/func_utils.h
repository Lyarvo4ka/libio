#pragma once
#include "constants.h"
#include "IODevice.h"
namespace IO
{
	void joinDataWithService(const path_string & dataFilename, const path_string & serviceFilename, const path_string & targetFilename)
	{
		File dataFile(dataFilename);
		dataFile.OpenRead();
		File serviceFile(serviceFilename);
		serviceFile.OpenRead();
		File targetFile(targetFilename);
		targetFile.OpenCreate();

		const uint32_t data_size = 8192;
		const uint32_t data_page_size = 1024;
		const uint32_t service_size = 176;
		const uint32_t sa_size = 106;
		const uint32_t numPages = data_size / data_page_size ;
		const uint32_t target_size = 9216;

		uint64_t data_offset = 0;
		uint64_t service_offset = 0;
		DataArray data(data_size);
		DataArray service_data(service_size);

		DataArray page_data(target_size);

		const uint32_t sa_end_offset = 9040;
		
		while (true)
		{
			if (data_offset >= dataFile.Size())
				break;
			if (service_offset >= serviceFile.Size())
				break;

			dataFile.setPosition(data_offset);
			dataFile.ReadData(data);

			serviceFile.setPosition(service_offset);
			serviceFile.ReadData(service_data);

			memset(page_data.data(), 0x00, page_data.size());

			for (uint32_t iPage = 0; iPage < numPages; ++iPage)
			{
				memcpy(page_data.data() + iPage * (data_page_size + sa_size), data.data() + iPage * data_page_size, data_page_size);

				//memcpy(page_data.data() + data_page_size + iPage * (data_page_size + sa_size), service_data.data() + iPage * sa_size, sa_size);

			}
			//memcpy(page_data.data() + numPages *(data_page_size + sa_size), service_data.data() + sa_size * numPages, service_size - sa_size * numPages);
			memcpy(page_data.data() + sa_end_offset, service_data.data(), service_data.size());

			targetFile.WriteData(page_data.data(), page_data.size());


			data_offset += data_size;
			service_offset += service_size;

		}



	}


	/*
	This function compare each sector with marker signature "bad sector" and if found it then set file size
	*/
	inline void changeSizeIfFindMarker(const path_string & file_name)
	{
		File the_file(file_name);
		if (!the_file.Open(OpenMode::OpenRead))
		{
			wprintf_s(L"Error open file.\n");
			return;
		}
		wprintf_s(L"File %s has been opened.\n", file_name.c_str());

		uint32_t bytes_read = 0;
		uint32_t marker_pos = 0;
		uint64_t offset = 0;
		Buffer buffer(default_block_size);

		while (offset < the_file.Size())
		{
			bytes_read = the_file.ReadData(buffer.data, buffer.data_size);
			if (bytes_read == 0)
			{
				wprintf_s(L"Error read file.\n");
				return;

			}

			if (bytes_read <= Signatures::bad_sector_marker_size)
				break;

			for (uint32_t iSector = 0; iSector < bytes_read; iSector += default_sector_size)
			{
				if ((bytes_read - iSector) < Signatures::bad_sector_marker_size)
					break;

				if (memcmp(buffer.data + iSector, Signatures::bad_sector_marker, Signatures::bad_sector_marker_size) == 0)
				{
					uint64_t new_file_size = offset;
					new_file_size += iSector;
					the_file.setSize(new_file_size);
					wprintf_s(L"File size was modified.\n");

					return;
				}
			}
			offset += bytes_read;

		}
		wprintf_s(L"Not found bad sector marker.\n");
	}

	void addValuesToEachPage(const path_string & source_name, const path_string & target_name , uint8_t val_marker)
	{
		const uint32_t source_size = 17600;
		const uint32_t target_size = 17664;
		const uint32_t source_buff_size = source_size *default_sector_size;
		const uint32_t target_buff_size = target_size * default_sector_size;


		File source(source_name);
		if (!source.Open(OpenMode::OpenRead))
		{
			wprintf_s(L"Error open file. %s\n", source.getFileName().c_str());
			return;
		}

		File target(target_name);
		if (!target.Open(OpenMode::Create))
		{
			wprintf_s(L"Error open file. %s\n", target.getFileName().c_str());
			return;
		}
		uint64_t cur_pos = 0;
		uint64_t max_file_size = source.Size();
		uint32_t bytes_to_copy = 0;
		uint32_t bytes_to_write = 0;

		uint32_t bytes_read = 0;
		uint32_t bytes_written = 0;

		Buffer source_buffer(source_buff_size);
		Buffer target_buffer(target_buff_size);

		while (cur_pos < max_file_size)
		{
			bytes_to_copy = calcBlockSize(cur_pos, max_file_size, source_buff_size);
			bytes_read = source.ReadData(source_buffer.data, source_buffer.data_size);
			if (bytes_read == 0)
				break;

			memset(target_buffer.data, val_marker, target_buffer.data_size);
			for (uint32_t iPage = 0; iPage < default_sector_size; ++iPage)
			{
				memcpy(target_buffer.data + iPage * target_size, source_buffer.data + iPage*source_size, source_size);
			}
			bytes_to_write = (bytes_to_copy / source_size) * target_size;
			bytes_written = target.WriteData(target_buffer.data, target_buffer.data_size);
			if ( bytes_written == 0 )
				break;


		}

	}

	void XorFiles(const path_string & filename1, const path_string & filename2, const path_string & resultname)
	{
		IO::File file1(filename1);
		if (!file1.Open(IO::OpenMode::OpenRead))
		{
			wprintf(L"Error open file\n %s", file1.getFileName().c_str());
			return ;
		}

		IO::File file2(filename2);
		if (!file2.Open(IO::OpenMode::OpenRead))
		{
			wprintf(L"Error open file\n %s", file2.getFileName().c_str());
			return;
		}

		IO::File resFile(resultname);
		if (!resFile.Open(IO::OpenMode::Create))
		{
			wprintf(L"Error create file\n %s", resFile.getFileName().c_str());
			return;
		}

		auto file1_size = file1.Size();
		auto file2_size = file2.Size();

		auto theSize = (file1_size < file2_size) ? file1_size : file2_size;

		DataArray data1(theSize);
		DataArray data2(theSize);
		file1.ReadData(data1.data(), data1.size());
		file2.ReadData(data2.data(), data2.size());

		DataArray res_data(theSize);

		for (auto i = 0; i < theSize; ++i)
		{
			ByteArray res = (ByteArray) &res_data.data()[i];
			ByteArray d1= (ByteArray)&data1.data()[i];
			ByteArray d2 = (ByteArray)&data2.data()[i];
			*res = *d1 ^ *d2;
		}


		resFile.WriteData(res_data.data(), res_data.size());






	}
}