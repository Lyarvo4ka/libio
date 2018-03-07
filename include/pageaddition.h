#pragma once


#include "IODevice.h"

#pragma  pack (1)
struct service_struct
{
	uint8_t header;
	uint8_t bankNumnber_HighPageNumber;
	uint8_t blockNumber;
	uint8_t LowPageNumber;
};
#pragma  pack ()

struct OffsetNumbers
{
	uint8_t Bank;
	uint8_t Block;
	uint16_t Page;

};


inline bool isAdditionPage(uint8_t service_header)
{
	service_header = service_header & 0xF0;
	if (service_header == 0x60)
		return true;
	return false;
}

uint8_t toBankNumber(uint8_t bank)
{
	return bank & 0x3F;
}
uint16_t joinToPageNumber(uint8_t bank, uint8_t page)
{
	uint16_t page_number = ((uint16_t)(bank & 0xC0) ) << 2 | page;
	return page_number;
}


class PageAddition
{
private:
	uint32_t page_size_;
	uint32_t service_size_;
	uint32_t page_data_size_;
	uint32_t ecc_size_;
	IO::File * dump_file_;
	IO::File * image_file_;

public:
	PageAddition(const IO::path_string & dump_file)
		: page_size_(8832*2)
		, service_size_(32)
		, page_data_size_(1024)
		, ecc_size_(76)
		, dump_file_(new IO::File(dump_file))
		, image_file_(nullptr)
	{

	}
	~PageAddition()
	{
		if (dump_file_)
		{
			delete dump_file_;
			dump_file_ = nullptr;
		}
		remove_image_file();

	}

	void remove_image_file()
	{
		if (image_file_)
		{
			delete image_file_;
			image_file_ = nullptr;
		}
	}

	void execute(const IO::path_string & image_file)
	{
		remove_image_file();
		image_file_ = new IO::File(image_file);

		if (!image_file_->Open(IO::OpenMode::OpenWrite))
		{
			wprintf(L"Error open image file.\r\n");
			return;
		}

		if (!dump_file_->Open(IO::OpenMode::OpenRead))
		{
			wprintf(L"Error open dump file.\r\n");
			return;

		}

		uint64_t iPage = 0;
		IO::Buffer buffer(page_size_);
		while (true)
		{
			//iPage = 2633;
			auto bytesRead = ReadPage(iPage, buffer);
			if (bytesRead == 0 )
				break;
			analize_service(buffer);
			++iPage;
		}



	}

	uint32_t ReadPage(uint64_t page_number, IO::Buffer & data_buffer)
	{
		if (!dump_file_->isOpen())
			return 0;

		uint32_t bytes_read = 0;
		uint64_t page_offset = page_number * page_size_;
		if (page_offset >= dump_file_->Size())
			return 0;

		dump_file_->setPosition(page_offset);
		return dump_file_->ReadData(data_buffer.data, data_buffer.data_size);
	}



	void analize_service(IO::Buffer & data_buffer)
	{
		service_struct * pService = (service_struct *)&data_buffer.data[page_size_ - service_size_];
		if (pService->header != 0x66)
		if (isAdditionPage(pService->header))
		{
			OffsetNumbers numbers = { 0 };
			numbers.Bank = toBankNumber(pService->bankNumnber_HighPageNumber);
			numbers.Block = pService->blockNumber;
			numbers.Page = joinToPageNumber(pService->bankNumnber_HighPageNumber, pService->LowPageNumber);
			uint64_t write_offset = (uint64_t)numbers.Bank * (uint64_t)0x80000000;
			write_offset += numbers.Block * 0x800000;
			write_offset += numbers.Page * 0x4000;
			WriteDataToImage(data_buffer, write_offset);

		}

	}

	uint32_t WriteDataToImage(IO::Buffer & data_buffer, uint64_t write_offset)
	{
		if (!image_file_->isOpen())
			return 0;

		if (write_offset > image_file_->Size())
			return 0;



		uint32_t PageCount = page_size_ / (page_data_size_ + ecc_size_);
		auto data_size = page_data_size_*PageCount;
		IO::Buffer write_buffer(data_size);

		uint32_t iPage = 0;
		while (iPage < PageCount)
		{
			auto image_page_offset = iPage*page_data_size_;
			auto dump_page_offset = iPage*(page_data_size_ + ecc_size_);
			if (iPage >= PageCount / 2) 
				dump_page_offset += service_size_;

			memcpy(write_buffer.data + image_page_offset, data_buffer.data + dump_page_offset, page_data_size_);
			++iPage;
		}

		image_file_->setPosition(write_offset);
		return image_file_->WriteData(write_buffer.data, write_buffer.data_size);

	}

};

namespace IO
{
	void add2bytesInEachPage(const ByteArray source_data, const uint32_t source_size, ByteArray dst_data, uint32_t dst_size)
	{
		const uint32_t page_1110 = 1110;
		const uint32_t page_1102 = 1102;
		const uint8_t add_data[] = { 0xFF,0xFF };
		const uint32_t add_size = SIZEOF_ARRAY(add_data);
		const uint32_t pages_count = 15;
		const uint32_t end_size = 24;

		uint32_t src_offset = 0;
		uint32_t dst_offset = 0;

		memset(dst_data, 0xFF, dst_size);
		memcpy(dst_data, source_data, page_1110);

		for (uint32_t iPage = 0; iPage < pages_count; ++iPage)
		{
			src_offset = page_1110 + iPage * page_1102;
			dst_offset = page_1110 + iPage * (page_1102 + add_size);
			memcpy(dst_data + dst_offset, source_data + src_offset, page_1102);

		}
		src_offset = page_1110 + pages_count* page_1102;
		dst_offset = page_1110 + pages_count* (page_1102 + add_size);
		memcpy(dst_data + dst_offset, source_data + src_offset, end_size);

	}
	void add2BytesInFile(const path_string & src_name, const path_string & dst_name)
	{
		const uint32_t SourcePageSize = 17664;
		const uint32_t countBlock = 15;
		const uint32_t SourceBlockSize = SourcePageSize * countBlock;	//264660(bytes)

		const uint32_t DstPageSize = SourcePageSize + 30 + 2;
		const uint32_t DstBlockSize = DstPageSize* countBlock;


		IO::File src_file(src_name);
		if (!src_file.Open(IO::OpenMode::OpenRead))
			return;

		IO::File dst_file(dst_name);
		if (!dst_file.Open(IO::OpenMode::Create))
			return;


		DataArray src_block(SourceBlockSize);
		DataArray dst_block(DstBlockSize);

		uint32_t src_offset = 0;
		uint32_t dst_offset = 0;

		uint64_t src_pos = 0;
		uint64_t dst_pos = 0;

		uint32_t bytesRead = 0;
		uint32_t bytesWritten = 0;
		
		while (true)
		{
			// Read file
			src_file.setPosition(src_pos);
			bytesRead = src_file.ReadData(src_block.data(), src_block.size());
			if (bytesRead == 0)
				break;
			

		
			for (uint32_t iPage = 0; iPage < countBlock; ++iPage)
			{
				src_offset = SourcePageSize * iPage;
				dst_offset = DstPageSize * iPage;
				auto src_ptr = src_block.data() + src_offset;
				auto dst_ptr = dst_block.data() + dst_offset;
				add2bytesInEachPage(src_ptr, SourcePageSize, dst_ptr, DstPageSize);

			}
			// Write file
			dst_file.setPosition(dst_pos);
			bytesWritten = dst_file.WriteData(dst_block.data(), dst_block.size());
			if (bytesWritten == 0)
				break;

			src_pos += bytesRead;
			dst_pos += bytesWritten;
		}
		src_file.Close();
		dst_file.Close();

	}

	//void add2bytesInEachPage(const DataArray & src_block, DataArray & dst_block)

}

