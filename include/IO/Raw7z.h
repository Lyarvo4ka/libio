#pragma once

#include "IO/AbstractRaw.h"

namespace IO
{
	class Raw7z :
		public DefaultRaw
	{
	public:
		Raw7z(IODevicePtr device)
			:DefaultRaw(device)
		{

		}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			return 0;
		}
		bool Specify(const uint64_t start_offset) override
		{
			return false;
		}


	};

}
