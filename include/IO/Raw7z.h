#pragma once

#include "IO/AbstractRaw.h"
#include "IO/Factories.h"

namespace IO
{
#pragma pack( push, 1)
	struct Header7z
	{
		uint8_t signature[6];
		uint16_t format_version;
		uint32_t crc_header;		//	CRC of the following 20 bytes
		uint64_t offset_end_header;	//	relative offset of End Header
		uint64_t length_end_header;	//	the length of End Header
		uint32_t crc_end_header;	//	CRC of the End Header
	};
#pragma pack(pop)

	const uint32_t Header7z_size = sizeof(Header7z);
	const uint16_t End7zSignature = 0x0617;

	class Raw7z :
		public DefaultRaw
	{
		uint64_t targetFileSize_ = 0;
	public:
		Raw7z(IODevicePtr device)
			:DefaultRaw(device)
		{

		}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			return 0;
		}
		bool isValid(const Header7z & header7z) const
		{
			if (header7z.offset_end_header > this->getSize()
				|| header7z.offset_end_header == 0
				|| header7z.length_end_header == 0)
				return false;
			return true;
		}
		bool Specify(const uint64_t start_offset) override
		{
			Header7z header7z;
			ByteArray block = (ByteArray)&header7z;
			this->ReadData(block, Header7z_size);

			if (!isValid(header7z))
				return 0;

			auto end_header_offset = start_offset + Header7z_size + header7z.offset_end_header;

			DataArray end_header(header7z.length_end_header);
			this->ReadData(end_header.data(), end_header.size());

			if (memcmp(end_header.data(), &End7zSignature, sizeof(End7zSignature)) == 0)
			{
				targetFileSize_ = Header7z_size + header7z.offset_end_header + header7z.length_end_header;
				return true;
			}

			

			return false;
		}


	};
	class Raw7zFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new Raw7z(device);
		}
	};

}
