#ifndef TRANSLATOR_H
#define TRANSLATOR_H
#include "IO\Finder.h"

namespace IO
{
#pragma pack( 1 )
	struct AddressMarker
	{
		uint32_t addr;
		uint32_t marker;
	};
#pragma pack()
//
struct DumpPage
{
	DumpPage(uint32_t page_size, uint32_t service_size)
		: pageSize(page_size)
		, serviceSize(service_size)
	{
		dumpSize = page_size + service_size;
	}
	uint32_t pageSize;
	uint32_t serviceSize;
	uint32_t dumpSize;
	uint32_t markerOffset;
};

struct ImagePage
{
	ImagePage(const uint32_t image_page_size)
		: imageData(image_page_size)
	{

	}
	DataArray imageData;
	uint32_t start_pos = 0;
	uint32_t count = 0;
};

	class NumberTranslator
	{
	public:
		ImagePage calcPagesTogether(const DataArray & dumpData , const uint32_t start_pos )
		{
			//for (uint32_t iPage = 0 ; iPage < dumpData.size() ; iPage += )
			return ImagePage(0);
		}
		bool saveToFile(File src_file, IODevicePtr dst)
		{
			const uint32_t PAGE_SIZE = 4096;
			const uint32_t SERVICE_SIZE = 512;
			const uint32_t DUMP_PAGE_SIZE = PAGE_SIZE + SERVICE_SIZE;
			const uint32_t COUNT_PER_BLOCK = 64;
			const uint32_t DUMP_BLOCK_SIZE = DUMP_PAGE_SIZE * COUNT_PER_BLOCK;

			const uint32_t IMAGE_BLOCK_SIZE = PAGE_SIZE * COUNT_PER_BLOCK;

			const uint32_t MAX_FILE_SIZE = 0x04000000;
			const uint32_t MARKER_92FFFFFF = 0x92FFFFFF;


			uint64_t src_file_size = src_file.Size();

			DataArray dump_buffer(DUMP_BLOCK_SIZE);

			uint64_t offset = 0;
			uint32_t bytes_to_read = DUMP_BLOCK_SIZE;
			uint32_t bytes_read = 0;

			uint32_t addrMarkerPos = 0;

			uint64_t copy_offset = 0;
			bool bWritten = 0;

			while (true)
			{
				src_file.setPosition(offset);
				bytes_to_read = calcBlockSize(offset, src_file_size, DUMP_BLOCK_SIZE);
				if (bytes_to_read == 0)
					break;

				bytes_read = src_file.ReadData(dump_buffer.data(), dump_buffer.size());
				if (bytes_read == 0)
					break;

				
				for (uint32_t iPage = 0; iPage < bytes_read; iPage += DUMP_PAGE_SIZE)
				{
					addrMarkerPos = iPage + PAGE_SIZE;
					if ((addrMarkerPos + sizeof(AddressMarker)) < bytes_read)
					{
						AddressMarker * pAddrMarker = (AddressMarker *)(dump_buffer.data() + addrMarkerPos);
						if (pAddrMarker->addr < MAX_FILE_SIZE)
							if (pAddrMarker->marker == MARKER_92FFFFFF)
							{
								uint64_t dst_offset = (uint64_t)pAddrMarker->addr * (uint64_t)PAGE_SIZE;
								dst->setPosition(dst_offset);
								uint32_t bytes_written = dst->WriteData(dump_buffer.data() + iPage, PAGE_SIZE);
								if (bytes_written == 0)
									break;
								bWritten = true;
								copy_offset = dst_offset;
							}
					}
				}
				if (bWritten)
					wprintf(L"%I64u (sector)\r\n", copy_offset / 512);
				bWritten = false;
				offset += DUMP_BLOCK_SIZE;
			}


			return true;
		}
		bool execute(const path_string & folder_dumps, IODevicePtr device_ptr)
		{


			Finder finder;
			finder.add_extension(L".bin");

			finder.FindFiles(folder_dumps);
			auto fileList = finder.getFiles();

			if (!device_ptr->Open(OpenMode::Create))
				return false;

			for (auto & theFile : fileList)
			{
				File src_file(theFile);
				wprintf(L"File %s\r\n", theFile.c_str());
				if (!src_file.Open(OpenMode::OpenRead))
					return false;

				if (!saveToFile(src_file, device_ptr))
					return false;
			}

			return true;
		}
	};

}

#endif