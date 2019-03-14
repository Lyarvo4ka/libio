#pragma once

#include "AbstractRaw.h"
#include "Factories.h"

namespace IO
{
#pragma pack (push) 
#pragma pack (1)
	struct flp_header_struct
	{
		uint8_t FLhd_name[4];
		uint8_t reserved[14];
		uint32_t size;
	};
#pragma pack (pop)
	const uint32_t flp_header_size = sizeof(flp_header_struct);


	class RawFLP
		: public DefaultRaw
	{
		uint64_t flp_size_ = 0;

	public:
		RawFLP(IODevicePtr device)
			: DefaultRaw(device)
		{
		}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (flp_size_ > 0)
			{
				uint64_t write_size = flp_size_ + flp_header_size;
				return appendToFile(target_file, start_offset, write_size);
			}

			return 0;
		}

		bool Specify(const uint64_t start_offset) override
		{
			flp_header_struct flp_struct = { 0 };
			
			setPosition(start_offset);
			ReadData((ByteArray)&flp_struct, flp_header_size);

			flp_size_ = flp_struct.size;

			return (flp_size_ != 0) ? true : false;
		}


	};

	class RawFLPFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new RawFLP(device);
		}
	};


}