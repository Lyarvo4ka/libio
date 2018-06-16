#pragma once
#include "IO\StandartRaw.h"

namespace IO
{
	class PLNRaw :
		public StandartRaw
	{
	public:

		PLNRaw(IODevicePtr device)
			: StandartRaw(device)
		{
		}

		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			return StandartRaw::SaveRawFile(target_file,start_offset);
		}

		bool Specify(const uint64_t start_offset) override
		{
			DataArray sector_data(getSectorSize());
			this->setPosition(start_offset);
			auto bytes_read = this->ReadData(sector_data.data(), sector_data.size());
			if (bytes_read != getSectorSize())
				return 0;
			const uint32_t numBytes = 5;
			DataArray fiveBytes(numBytes);
			memcpy(fiveBytes.data(), sector_data.data(), fiveBytes.size());
			this->setFooter(&fiveBytes,512);
			return true;
		}
		bool Verify(const IO::path_string & file_path) override
		{
			return StandartRaw::Verify(file_path);
		}
	};

	class PLNRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new PLNRaw(device);
		}
	};
}
