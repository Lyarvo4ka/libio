#pragma once

#include "IO/IODevice.h"
#include "IO/utility.h"

namespace IO
{

	const uint32_t acronis_data_size = 262144;
	const uint32_t acronis_header_size = 28;

#pragma pack( 1 )
	struct AcronisBlock
	{
		uint8_t crc[4];
		uint8_t eigthFF[8];
		uint8_t data[acronis_data_size];
	};
#pragma pack( )
	class TibExtractor
	{
		path_string damaged_file_;
	public:
		TibExtractor(const path_string & damaged_file)
			: damaged_file_(damaged_file)
		{

		}
		bool isEigthFF(const AcronisBlock & acronis_block)
		{
			for (uint32_t i = 0; i < 8; ++i)
				if (acronis_block.eigthFF[i] != 0xFF)
					return false;

			return true;
		}

		void extractTo(const path_string & target_folder)
		{
			File srcFile(damaged_file_);
			if (!srcFile.Open(OpenMode::OpenRead))
				return;

			uint64_t offset = acronis_header_size;

			AcronisBlock * pBlock = new AcronisBlock();
			const uint32_t block_size = sizeof(AcronisBlock);
			uint32_t bytesRead = 0;

			auto targetFileName = offsetToPath(target_folder, offset, L".bin");
			File dstFile(targetFileName);
			if (!dstFile.Open(OpenMode::Create))
				return;

			uint64_t target_offset = 0;

			while (true)
			{
				srcFile.setPosition(offset);
				bytesRead = srcFile.ReadData((ByteArray)pBlock, block_size);
				if (bytesRead == 0)
					break;
				//if (!isEigthFF(*pBlock))
				//{
				//	int k = 0;
				//	k = 1;
				//}
				dstFile.setPosition(target_offset);
				dstFile.WriteData(pBlock->data, acronis_data_size);

				target_offset += acronis_data_size;

				offset += block_size;
			}
			delete pBlock;

		}

	};

}