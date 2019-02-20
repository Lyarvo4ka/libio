#pragma once

#include "AbstractRaw.h"
#include "Factories.h"

namespace IO
{
#pragma pack( push, 1)
	struct cdw_header
	{
		uint8_t header[2];
		uint8_t unknown[6];
		uint32_t size;
	};
#pragma pack(pop)

	const uint32_t cdw_header_size = sizeof(cdw_header);

	class RawCWD 
		: public DefaultRaw
	{
		uint32_t cdw_1st_size_ = 0 ;
		uint32_t cdw_next_size_ = 0;

	public:
		RawCWD(IODevicePtr device)
			: DefaultRaw(device)
		{

		}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			uint64_t full_size = cdw_1st_size_ + cdw_next_size_;
			if (full_size > 0)
				return appendToFile(target_file, start_offset, full_size);
			return 0;
		}
		bool Specify(const uint64_t start_offset) override
		{
			DataArray buffer(this->getBlockSize());
			this->setPosition(start_offset);
			this->ReadData(buffer);

			cdw_header * pCDW_header = (cdw_header *)buffer.data();
			cdw_1st_size_ = pCDW_header->size;

			uint64_t offset = start_offset + cdw_1st_size_;

			this->setPosition(offset);
			this->ReadData(buffer);

			const uint32_t number_together = 5;
			uint32_t nTogeter = 0;

			uint32_t iPos = 0;

			for (iPos = 0; iPos < buffer.size() - number_together; ++iPos)
			{
				nTogeter = 0;
				for (auto i = 0; i < number_together; ++i)
				{
					if (buffer[iPos + i] == 0)
					{
						++nTogeter;
					}
				}

				if (nTogeter == number_together)
					break;
			}

			cdw_next_size_ = iPos + 3;

			return true;
		}
	};

	class RawCWDFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new RawCWD(device);
		}
	};


};
