#ifndef DBF_H
#define DBF_H

#include <windows.h>

#include "StandartRaw.h"


namespace IO
{
#pragma pack( push, 1)
struct dbf_header
{
	uint8_t valid_base;
	uint8_t yymmdd[3];
	uint32_t numRecords;
	uint16_t header_size;
	uint16_t record_size;

}; 
#pragma pack(pop)
const std::size_t dbf_header_size = sizeof(dbf_header);

inline bool hasNulls(IO::ByteArray data, const uint32_t size)
{
	for (uint32_t i = 0; i < size; ++i)
		if (data[i] == 0)
			return true;

	return false;
}

inline void fixDBF(const IO::path_string & file_path)
{
	auto dbf_file = IO::makeFilePtr(file_path);
	if (!dbf_file->Open(IO::OpenMode::OpenWrite))
	{
		wprintf(L"Error open file %s." , file_path.c_str());
		return;
	}
	auto file_size = dbf_file->Size();

	dbf_header dbf_data = { 0 };
	auto bytes_read = dbf_file->ReadData((IO::ByteArray) &dbf_data, dbf_header_size);

	if (dbf_data.header_size == 0)
	{
		wprintf(L"Header size is 0.");
		return;
	}
	if (dbf_data.record_size == 0)
	{
		wprintf(L"Record size is 0.");
		return;
	}

	auto data_buffer = IO::makeDataArray(dbf_data.record_size);
	uint32_t offset = dbf_data.header_size;
	auto tmp_path = file_path + L".tmp";
	auto tmp_file = IO::makeFilePtr(tmp_path);
	if (!tmp_file->Open(IO::OpenMode::Create))
	{
		wprintf(L"Error open file %s.", file_path.c_str());
		return;
	}

	uint32_t write_offset = 0;
	auto header_data = IO::makeDataArray(dbf_data.header_size);
	dbf_file->setPosition(0);
	bytes_read = dbf_file->ReadData(header_data->data(), header_data->size());
	if (bytes_read != dbf_data.header_size)
	{
		wprintf(L"Error Read header.");
		return;
	}
	tmp_file->WriteData(header_data->data(), header_data->size());
	write_offset += dbf_data.header_size;


	for (uint32_t i = 0; i < dbf_data.numRecords; ++i)
	{
		dbf_file->setPosition(offset);
		bytes_read = dbf_file->ReadData(data_buffer->data(), data_buffer->size());
		if (bytes_read != dbf_data.record_size)
		{
			wprintf(L"Error Read record.");
			return;
		}


		if (!hasNulls(data_buffer->data(), data_buffer->size()))
		{
			tmp_file->setPosition(write_offset);
			tmp_file->WriteData(data_buffer->data(), data_buffer->size());
			write_offset += data_buffer->size();
		}
		offset += data_buffer->size();
	}

	uint32_t tmp_size = (uint32_t)tmp_file->Size();
	uint32_t numRecords = 0;
	numRecords = tmp_size - dbf_data.header_size - 1;
	numRecords /= dbf_data.record_size + 1;
	tmp_file->setSize(dbf_data.header_size + numRecords * dbf_data.record_size + 1);

	tmp_file->setPosition(tmp_file->Size() - 1);
	uint8_t lastByte = 0;
	tmp_file->ReadData((IO::ByteArray)&lastByte, 1);
	if (lastByte != 0x1a)
	{
		lastByte = 0x1a;
		tmp_file->setPosition(tmp_file->Size() - 1);
		tmp_file->WriteData((IO::ByteArray)&lastByte, 1);
	}

	dbf_data.numRecords = numRecords;
	tmp_file->setPosition(0);
	tmp_file->WriteData((IO::ByteArray)&dbf_data, dbf_header_size);



	
}


	class DBFRaw
		: public StandartRaw
	{
	private:
		uint32_t counter_ = 0;
	public:
		DBFRaw(IODevicePtr device)
			: StandartRaw(device)
		{

		}
		bool isValidDate(const dbf_header & header)
		{
			uint8_t year = header.yymmdd[0];
			uint8_t month = header.yymmdd[1];
			uint8_t day = header.yymmdd[2];

			if (year >= 15 && year <= 16)

			if (month > 0 && month <= 12)
			if (day > 0 && day <= 31)
				return true;
			return false;
		}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			dbf_header header = { 0 };
			setPosition(start_offset);
			auto bytes_read = ReadData((ByteArray)&header, dbf_header_size);
			if (bytes_read == 0)
				return 0;

			if (!isValidDate(header))
				return 0;
			
			static uint32_t counter = 0;
			++counter;
			counter_ = counter;

			wchar_t buff[18] = {0};
			swprintf_s(buff, 18, L"%.2d-%.2d-%.2d", header.yymmdd[0], header.yymmdd[1], header.yymmdd[2]);
			path_string date_str(buff);

			path_string filename = addBackSlash(target_file.getFileName()) + date_str + L"-" + toNumberExtension(counter_++, L".dbf");


			uint32_t write_size = header.header_size + header.record_size*header.numRecords + 1;
			if (write_size <= 1)
				return 0;

			FilePtr file = std::make_shared<File>(filename);
			if (!file->Open(OpenMode::Create))
			{
				wprintf(L"Error create file\n");
				return 0;
			}
			 
			
			return appendToFile(target_file, start_offset, write_size);;
		}

	};
}
#endif
