#ifndef RAW_MTS_H
#define RAW_MTS_H


#include "AbstractRaw.h"
#include "Factories.h"


namespace IO
{
	const uint8_t marker_0x47 = 0x47;
	const uint32_t marker_0x47_offset = 4;
	const uint32_t DEFAULT_MTS_SPECIFY_COUNT = 10;
	const uint32_t FRAME_SIZE = 192;

	class RawMTS
		: public DefaultRaw
	{

	private:
		uint32_t frames_count_ = DEFAULT_MTS_SPECIFY_COUNT;

	public:
		RawMTS( IODevicePtr device)
			: DefaultRaw(device)
		{
			setBlockSize( getSectorSize() * FRAME_SIZE);
		}
		~RawMTS()
		{
		}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (!target_file.isOpen())
			{
				wprintf(L"Target file wasn't opened.\n");
				return 0;
			}
			uint32_t bytes_read = 0;
			auto data_buffer = makeDataArray(getBlockSize());

			uint64_t offset = start_offset;

			uint32_t iFrame = 0;

			bool bEnd = false;
			uint32_t bytesWritten = 0;

			while (offset < this->getSize())
			{
				setPosition(offset);
				bytes_read = ReadData(data_buffer->data(), data_buffer->size());
				if (bytes_read == 0)
					break;

				for (iFrame = 0; iFrame < bytes_read; iFrame += FRAME_SIZE)
				{
					if (data_buffer->data()[iFrame + marker_0x47_offset] != marker_0x47)
					{
						//wprintf(L"Found in mts incorrect marker.(It's not 0x47).\n");
						bEnd = true;
						break;
					}
				}
				if (iFrame == 0)
				{
					break;
				}
				bytesWritten = appendToFile(target_file, offset, iFrame);
				if (bytesWritten == 0)
					break;

				if (bEnd)
					break;

				offset += bytes_read;
			}

			return offset - start_offset;
		}
		bool Specify(const uint64_t header_offset) override
		{
			const uint32_t specify_size = frames_count_ * getBlockSize();
			auto specify_buffer = makeDataArray(specify_size);
			uint32_t bytes_read = 0;

			setPosition(header_offset);
			if (ReadData(specify_buffer->data(), specify_buffer->size()) == 0)
				return false;

			for (auto iFrame = marker_0x47_offset; iFrame < specify_buffer->size(); iFrame += FRAME_SIZE)
			{
				if (specify_buffer->data()[iFrame] != marker_0x47)
					return false;
			}
			return true;
		}
		void setFramesCountToSpecify(const uint32_t frames_count)
		{
			frames_count_ = frames_count;
		}

	};

	class RawMTSFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new RawMTS(device);
		}
	};

}

#endif