#pragma once
#include "StandartRaw.h"

#include "AbstractRaw.h"
#include "Factories.h"
namespace IO
{
	class RawIMD
		: public DefaultRaw
	{
	public:
		RawIMD(IODevicePtr device)
			: DefaultRaw(device)
		{
		}
		~RawIMD()
		{}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (!target_file.isOpen())
			{
				wprintf(L"Target file wasn't opened.\n");
				return 0;
			}
			uint32_t file_size = 0;
			setPosition(start_offset);
			auto bytesRead = ReadData((IO::ByteArray)&file_size, sizeof(uint32_t));
			
			if (bytesRead != sizeof(uint32_t))
			{
				wprintf(L"File size is 0: \n");
				return 0;
			}
			if (file_size > 0)
			{
				file_size += 8;

				return appendToFile(target_file, start_offset, file_size);
			}
			else
				wprintf(L"Size is 0.\n");
			return 0;
		}

		uint32_t read_file_size(const uint64_t start_offset)
		{
			uint32_t file_size = 0;
			auto bytesRead = ReadData((IO::ByteArray)&file_size, sizeof(uint32_t));

			if (bytesRead != sizeof(uint32_t))
			{
				wprintf(L"Error read : \n");
				return 0;
			}

			return file_size;

		}

		bool Specify(const uint64_t start_offset) override
		{

			if ( read_file_size(start_offset) > 0 )
				return true;
			
			return false;


		}
	};

	class RawIMDFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new RawIMD(device);
		}
	};
}