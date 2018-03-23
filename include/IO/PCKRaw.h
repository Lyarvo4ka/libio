#pragma once

#include "IO\StandartRaw.h"


namespace IO
{
	/*
	1. Header (2 bytes).
	2. Размер записи (2 bytes). 2 таких записи.
	*/
	class PCKRaw
		: public StandartRaw
	{
	private:
		uint64_t writeSize_ = 0;
	public:
		explicit PCKRaw(IODevicePtr device)
			: StandartRaw(device)
		{
		}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (writeSize_ > 0)
				return appendToFile(target_file, start_offset, writeSize_);
			else return 0;
		}
		bool Specify(const uint64_t start_offset) override
		{
			const uint32_t headerSize = 256;
			const uint32_t NumEntries = 2;
			const uint32_t size = 2;
			uint16_t entry_size = 0;

			uint64_t offset = start_offset + headerSize;
			uint32_t bytesRead = 0;
			ByteArray pData = (ByteArray)&entry_size;

			for (uint32_t i = 0; i < NumEntries; ++i)
			{
				this->setPosition(offset);
				bytesRead = this->ReadData(pData, size);

				offset += entry_size + size;
			}
			writeSize_ = offset - start_offset;
			if (writeSize_ > 5 * 1024)
				return false;
			return true;
		}
	};

	class PCKRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new PCKRaw(device);
		}
	};


}
