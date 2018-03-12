#pragma once

#include "AbstractRaw.h"
#include "Factories.h"

namespace IO
{

	struct avi_header_struct
	{
		uint8_t fiff_name[4];
		uint32_t size;

	};
	const uint32_t avi_header_struct_size = sizeof(avi_header_struct);

	const uint32_t ONE_MB = 1 * 1024 * 1024;
	class RawAVI
		: public DefaultRaw
	{
	private:
		uint32_t minFileSize_ = ONE_MB;
	public:
		RawAVI(IODevicePtr device)
			: DefaultRaw(device)
		{
		}
		~RawAVI()
		{}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (!target_file.isOpen())
			{
				wprintf(L"Target file wasn't opened.\n");
				return 0;
			}

			avi_header_struct avi_struct = { 0 };
			readAviStruct(avi_struct, start_offset);
			wprintf(L"AVI write size: %d\n", avi_struct.size);
			if ( avi_struct.size > 0 )
			{
				avi_struct.size += avi_header_struct_size;

				return appendToFile(target_file, start_offset, avi_struct.size);
			}
			else
				wprintf(L"Size AVI is 0.\n");
			return 0;
		}

		bool Specify(const uint64_t start_offset) override
		{
			// Read header_sector if size is more than 1Mb return true
			avi_header_struct avi_struct = { 0 };
			readAviStruct(avi_struct, start_offset);

			if (avi_struct.size >= 0xFFFFFFFF - avi_header_struct_size)
				return false;

			//if (avi_struct.size > ONE_MB)
			//	return true;


			return true;
		}
		void setMinFileSize(const uint32_t minFileSize)
		{
			minFileSize_ = minFileSize;
		}
		uint32_t getMinFileSize() const
		{
			return minFileSize_;
		}
		void readAviStruct(avi_header_struct & avi_header , const uint64_t start_offset)
		{
			setPosition(start_offset);
			ReadData((ByteArray)&avi_header, sizeof(avi_header_struct));
		}
	};

	class RawAVIFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new RawAVI(device);
		}
	};
}
