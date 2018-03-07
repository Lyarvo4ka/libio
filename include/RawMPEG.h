#pragma once


#include "AbstractRaw.h"
#include "Factories.h"

#include "constants.h"

namespace IO
{
	const uint8_t mpeg_data[] = { 0x00 , 0x00 , 0x01 , 0xBA };
	const uint32_t mpeg_data_size = SIZEOF_ARRAY(mpeg_data);
	const uint32_t default_mpeg_data_size = 2048;
	const uint32_t DEFAULT_MPEG_SPECIFY_COUNT = 256;


#define PACK_START_CODE             ((unsigned int)0x000001ba)
#define SYSTEM_HEADER_START_CODE    ((unsigned int)0x000001bb)
#define SEQUENCE_END_CODE           ((unsigned int)0x000001b7)
#define PACKET_START_CODE_MASK      ((unsigned int)0xffffff00)
#define PACKET_START_CODE_PREFIX    ((unsigned int)0x00000100)
#define ISO_11172_END_CODE          ((unsigned int)0x000001b9)

	/* mpeg2 */
#define PROGRAM_STREAM_MAP 0x1bc
#define PRIVATE_STREAM_1   0x1bd
#define PADDING_STREAM     0x1be
#define PRIVATE_STREAM_2   0x1bf

#define AUDIO_ID 0xc0
#define VIDEO_ID 0xe0
#define H264_ID  0xe2
#define AC3_ID   0x80
#define DTS_ID   0x88
#define LPCM_ID  0xa0
#define SUB_ID   0x20

#define STREAM_TYPE_VIDEO_MPEG1     0x01
#define STREAM_TYPE_VIDEO_MPEG2     0x02
#define STREAM_TYPE_AUDIO_MPEG1     0x03
#define STREAM_TYPE_AUDIO_MPEG2     0x04
#define STREAM_TYPE_PRIVATE_SECTION 0x05
#define STREAM_TYPE_PRIVATE_DATA    0x06
#define STREAM_TYPE_AUDIO_AAC       0x0f
#define STREAM_TYPE_VIDEO_MPEG4     0x10
#define STREAM_TYPE_VIDEO_H264      0x1b
#define STREAM_TYPE_VIDEO_CAVS      0x42

#define STREAM_TYPE_AUDIO_AC3       0x81

#pragma pack( 1 )		// push pop
	struct mpeg_pack_t
	{
		uint32_t id;
		uint16_t size;
	};
#pragma pack( ) 

	inline bool isPackStartCode(const uint32_t mpeg_id)
	{
		return mpeg_id == PACK_START_CODE;
	}

	inline bool isMpegMarker(const uint32_t mpeg_id)
	{
		switch (mpeg_id & 0x0000FFFF)
		{
		case PROGRAM_STREAM_MAP:
		case PRIVATE_STREAM_1:
		case PADDING_STREAM:
		case PRIVATE_STREAM_2:
			return true;
		}

		switch (mpeg_id & 0x000000FF)
		{
		case AUDIO_ID:
		case VIDEO_ID:
		case H264_ID:
		case AC3_ID:
		case DTS_ID:
		case LPCM_ID:
		case SUB_ID:
			return true;

		}

		return false;
	}


	const uint32_t mpeg_pack_size = sizeof(mpeg_pack_t);
	const uint32_t VALUE_512Kb = 512 * 1024;


	class RawMPEG
		: public DefaultRaw
	{
	private:
		uint32_t mpegBlockSize_ = default_mpeg_data_size;
		uint32_t checkMaxSize_ = VALUE_512Kb;
	public:
		RawMPEG(IODevicePtr device)
			: DefaultRaw(device)
		{
		}
		~RawMPEG()
		{}

		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (!target_file.isOpen())
			{
				wprintf(L"Target file wasn't opened.\n");
				return 0;
			}
			if (mpegBlockSize_ == 0)
			{
				wprintf(L"Error mpeg_block_size is 0.\n");
				return 0;
			}

			uint32_t bytes_read = 0;
			if (getBlockSize() < mpegBlockSize_)
				setBlockSize(mpegBlockSize_);

			auto data_buffer = makeDataArray(getBlockSize());

			uint64_t offset = start_offset;
			uint64_t bytesWritten = 0;
			bool bEnd = false;
			uint32_t iBlock = 0;


			uint32_t sizeToWrite = 0;
			uint32_t end_pos = 0;
			while (offset < this->getSize())
			{ 
				setPosition(offset);
				bytes_read = ReadData(data_buffer->data(), data_buffer->size());
				if (bytes_read == 0 )
					break;

				sizeToWrite = data_buffer->size();

				if (bEnd = findEndFile(data_buffer->data(), data_buffer->size(), end_pos))
					sizeToWrite = end_pos;

				bytesWritten += appendToFile(target_file, offset, sizeToWrite);

				if (bEnd)
					break;

				offset += bytes_read;
			}

			return bytesWritten;
		}
		bool findEndFile(const ByteArray data , const uint32_t data_size, uint32_t & end_pos)
		{
			uint32_t end_code = ISO_11172_END_CODE;
			toBE32(end_code);

			uint32_t end_seq_code = SEQUENCE_END_CODE;
			toBE32(end_seq_code);
			const uint32_t marker_size = sizeof(uint32_t);

			for (uint32_t iBlock = 0; iBlock < data_size - marker_size; ++iBlock)
			{
				if ((iBlock % mpegBlockSize_) == 0)
					if (memcmp(data + iBlock, mpeg_data, mpeg_data_size) != 0)
					{
						end_pos = iBlock;
						printf("not header\r\n");
						return true;
					}

				if (memcmp(data + iBlock, &end_code, marker_size) == 0)
				{
					if (iBlock >= marker_size)
					if (memcmp(data + iBlock - marker_size, &end_seq_code, marker_size) == 0)
					{
						iBlock += marker_size;
						end_pos = iBlock;
						printf("End code.\r\n");
						return true;
					}
				}
			}
			return false;
		}
		bool Specify(const uint64_t start_offset) override
		{

			auto new_mpeg_size = pickUpBlockSize(start_offset, VALUE_512Kb);

			if (new_mpeg_size == 0)
			{
				//new_mpeg_size = calcMpegBlockSize(start_offset);
				//if ( new_mpeg_size == 0)
					return false;
			}

			new_mpeg_size /= default_mpeg_data_size;
			new_mpeg_size *= default_mpeg_data_size;

			mpegBlockSize_ = new_mpeg_size;
		
			uint32_t dataSize = VALUE_512Kb;

			auto data_array = makeDataArray(dataSize);

			setPosition(start_offset);
			if (ReadData(data_array->data(), data_array->size()) == 0)
				return false;

			for (uint32_t iBlock = 0; iBlock < data_array->size(); iBlock += mpegBlockSize_)
			{
				if (memcmp(data_array->data() + iBlock, mpeg_data, mpeg_data_size) != 0)
					return false;
			}

			return true;
		}
		void setMpegBlockSize(const uint32_t mpegBlockSize)
		{
			mpegBlockSize_ = mpegBlockSize;
		}

		uint32_t calcMpegBlockSize(const uint64_t start_offset)
		{
			// DataArray data_sector(getSectorSize());
			uint8_t data_sector[default_sector_size];
			setPosition(start_offset);
			ReadData(data_sector, default_sector_size);
			mpeg_pack_t * pMpeg_pack = (mpeg_pack_t *)(data_sector);

			// cmp 
			uint32_t mpeg_id = pMpeg_pack->id;
			toBE32(mpeg_id);
			if (!isPackStartCode(mpeg_id))
				return 0;

			bool bFound = false;
			uint32_t sys_header_pos = 0;

			for ( auto iByte = sizeof(uint32_t); iByte < default_sector_size - sizeof(uint32_t); ++iByte)
			{
				//ZeroMemory(pMpeg_pack, mpeg_pack_size);
				pMpeg_pack = (mpeg_pack_t *)(&data_sector[iByte]);
				mpeg_id = pMpeg_pack->id;
				toBE32(mpeg_id);
				if (mpeg_id == SYSTEM_HEADER_START_CODE)
				{
					bFound = true;
					sys_header_pos = iByte;
					break;
				}
			}
			if (!bFound)
				return 0;
			uint16_t toSize = pMpeg_pack->size;
			toBE16(toSize);
			uint32_t mpeg_block_size = sys_header_pos + mpeg_pack_size + toSize;
			uint64_t cur_pos = start_offset + mpeg_block_size;

			mpeg_pack_t mpeg_pack = { 0 };
			memcpy(&mpeg_pack, pMpeg_pack, mpeg_pack_size);
			
			uint32_t chunk_size = 0;
			
			while (mpeg_id != PACK_START_CODE)
			{
				setPosition(cur_pos);
				ReadData((ByteArray)&mpeg_pack, mpeg_pack_size);

				mpeg_id = mpeg_pack.id;
				toBE32(mpeg_id);

				if (isPackStartCode(mpeg_id))
					return mpeg_block_size;

				if (!isMpegMarker(mpeg_id))
					return 0;
				toSize = mpeg_pack.size;
				toBE16(toSize);
				chunk_size = toSize + mpeg_pack_size;
				mpeg_block_size += chunk_size;

				if (mpeg_block_size > VALUE_512Kb)
					return 0;

				cur_pos += chunk_size;
			}

			return mpeg_block_size;
		}

		uint32_t pickUpBlockSize(const uint64_t start_offset, const uint32_t max_block_size)
		{
			if (max_block_size < default_mpeg_data_size)
				return 0;

			if (max_block_size % default_mpeg_data_size != 0)
				return 0;

			auto data_buffer = makeDataArray(max_block_size);
			setPosition(start_offset);
			ReadData(data_buffer->data(), data_buffer->size());

			for (auto iBlock = default_mpeg_data_size; iBlock < max_block_size; iBlock *= 2)
			{
				if (memcmp(data_buffer->data() + iBlock, mpeg_data, mpeg_data_size) == 0)
					return iBlock;
			}

			return 0;
		}


	};

	class RawMPEGFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new RawMPEG(device);
		}
	};
};