#pragma once

#include "IO/IODevice.h"

namespace IO
{
	void remove_0xff(const path_string & source_name, const path_string & target_name)
	{
		const uint32_t first_size = 0x1FC000;
		const uint32_t chunk_size = 0x200000;

		File src_file(source_name);
		src_file.OpenRead();
		auto src_size = src_file.Size();

		File dst_file(target_name);
		dst_file.OpenCreate();

		uint32_t bytes_read = 0;
		uint32_t bytes_written = 0;

		uint64_t src_offset = 0;
		uint64_t dst_offset = 0;
		DataArray data_array(chunk_size);

		src_file.setPosition(src_offset);
		src_file.ReadData(data_array.data(), first_size);
		dst_file.setPosition(dst_offset);
		dst_file.WriteData(data_array.data(), first_size);
		src_offset += first_size + default_sector_size;
		dst_offset += first_size;


		while (true)
		{
			src_file.setPosition(src_offset);
			src_file.ReadData(data_array.data(), chunk_size);
			dst_file.setPosition(dst_offset);
			dst_file.WriteData(data_array.data(), chunk_size);

			src_offset += chunk_size + default_sector_size;
			dst_offset += chunk_size;
			if (src_offset + chunk_size > src_size)
				break;
		}
	}
}