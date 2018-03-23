#pragma once

#include "IO\StandartRaw.h"


namespace IO
{
	/*
	1. Header (2 bytes).
	2. Размер записи (4 bytes). 3 таких записи.
	*/
	class ZS2Raw
		: public StandartRaw
	{
	private:
		uint64_t writeSize_ = 0;
	public:
		explicit ZS2Raw(IODevicePtr device)
			: StandartRaw(device)
		{

		}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (writeSize_ > 0)
				return this->appendToFile(target_file, start_offset, writeSize_);
			return 0;
		}
		bool Specify(const uint64_t start_offset) override
		{
			uint32_t entry_size = 0;
			ByteArray pData = (ByteArray)&entry_size;
			const uint32_t HeaderSIZE = 2;
			const uint32_t NumEntries = 3;
			const uint32_t size = 4;
			uint64_t offset = start_offset + HeaderSIZE;
			uint32_t bytesRead = 0;
			for (uint32_t i = 0; i < NumEntries; ++i)
			{
				this->setPosition(offset);
				bytesRead = this->ReadData(pData, size);
				if (bytesRead == 0)
					break;
				offset += entry_size + size;
			}
			//offset += entry_size /*+ size*/;
			writeSize_ = offset - start_offset;
			if (writeSize_ > 2 * 1024)
				return false;

			return true;
		}

	};
	class ZS2RawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new ZS2Raw(device);
		}
	};


	/*
	1. Header (2 bytes).
	2. Размер записи (2 bytes) BE.
	*/
	class ZS2Raw_3082
		: public StandartRaw
	{
	private:
		uint64_t writeSize_ = 0;
	public:
		explicit ZS2Raw_3082(IODevicePtr device)
			: StandartRaw(device)
		{

		}		
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (writeSize_ > 0)
				return this->appendToFile(target_file, start_offset, writeSize_);
			return 0;
		}
		bool Specify(const uint64_t start_offset) override
		{
			uint16_t entry_size = 0;
			ByteArray pData = (ByteArray)&entry_size;

			uint64_t offset = start_offset + 2;
			this->setPosition(offset);
			auto bytesRead = this->ReadData(pData, 2);
			if (bytesRead == 0)
				return false;

			toBE16(entry_size);
			if (entry_size > 1 * 1024)
				return false;

			writeSize_ = entry_size + 2 + 2;

			return true;

		}
	};

	class ZS2Raw_3082Factory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new ZS2Raw_3082(device);
		}
	};
}