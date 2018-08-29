#pragma once

#include "QuickTime.h"


//using namespace IO;
namespace IO
{

	/*
		Восстановление фрагментированных видео файлов для GoPro.
		Сначала идет видео большого расширения потом 16 кластеров маленького.
		Для кажного кластера считаем энтопию до заголовка QT.
		Удаляем 16 кластеров которе меньше граници заданной энтропии.


	*/
	struct NullsInCluster
	{
		uint32_t cluster_number;
		uint32_t number_nulls;
	};
	class GoProFile
	{
		std::vector<uint32_t> clusterMap_;
		DataArray::Ptr endChunk_;
		DataArray::Ptr moovData_;
	public:
		void addNumberNulls(const uint32_t cluster_number, const uint32_t number_nulls)
		{
			clusterMap_.push_back(number_nulls);
		}
		void setEndChunk(DataArray::Ptr end_chunk)
		{
			endChunk_ = std::move(end_chunk);
		}
		void setMoovData(DataArray::Ptr moov_data)
		{
			moovData_ = std::move(moov_data);
		}


	};

	std::string clusterNullsToSstring(const uint32_t cluster_number, const uint32_t number_nulls)
	{
		auto cluster_str = std::to_string(cluster_number);
		auto number_nulls_str = std::to_string(number_nulls);

		return  cluster_str + " \t" + number_nulls_str;
	}

	class GoProRaw
		: public QuickTimeRaw
	{
		const uint32_t GP_CLUSTER_SIZE = 32768;
		const uint32_t GP_LRV_SKIP_COUNT = 16;
	private:
		uint32_t nulls_border_ = 187;

	public:
		GoProRaw(IODevicePtr device)
			: QuickTimeRaw(device)
		{
			setBlockSize(GP_CLUSTER_SIZE);
		}
		GoProFile SaveTempFile(File & temp_file, File & txt_file , const uint64_t start_offset)
		{
			uint64_t offset = start_offset;

			const auto cluster_size = this->getBlockSize();

			uint32_t bytesRead = 0;

			DataArray data_array(cluster_size);

			uint32_t number_nulls = 0;
			//uint32_t cluster_number = 0;

			std::vector<uint32_t> cluster_map;
			uint32_t moov_pos = 0;

			GoProFile gpFile;
			uint32_t cluster = 0;
			std::string_view endLine = "\n";

			while (offset < this->getSize())
			{
				setPosition(offset);
				bytesRead = ReadData(data_array);

				if (findMOOV_signature(data_array, moov_pos))
				{
					uint32_t qt_block_pos = moov_pos;
					if (moov_pos > qt_keyword_size) 
						qt_block_pos = moov_pos - qt_keyword_size;

					auto endData = makeDataArray(qt_block_pos);
					memcpy(endData->data(), data_array.data(), qt_block_pos);
					gpFile.setEndChunk(std::move(endData));

					qt_block_t * moov_block = (qt_block_t *)(data_array.data() + qt_block_pos);
					auto moov_block_size = moov_block->block_size;
					toBE32(moov_block_size);

					DataArray::Ptr moov_data = makeDataArray(moov_block_size);
					uint64_t moov_offset = offset + qt_block_pos;
					setPosition(moov_offset);
					ReadData(moov_data->data(), moov_data->size());
					gpFile.setMoovData(std::move(moov_data));

					break;
				}
				temp_file.WriteData(data_array.data(), data_array.size());
				number_nulls = calc_nulls(data_array);
				gpFile.addNumberNulls(0, number_nulls);

				auto str_toWrite = clusterNullsToSstring(cluster , number_nulls);
				txt_file.WriteText(str_toWrite);
				if (number_nulls > nulls_border_)
					txt_file.WriteText(" -skipped");
				txt_file.WriteText(endLine);

				++cluster;
				offset += bytesRead;
			}

			return gpFile;
		}

		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			// 1. Saving to temp file 
			// 2. Calculate to each cluster number of nulls
			auto temp_file_name = target_file.getFileName() + L".temp";
			File tempFile(temp_file_name);
			tempFile.OpenCreate();

			auto txt_file_name = target_file.getFileName() + L".txt";
			File txtFile(txt_file_name);
			txtFile.OpenCreate();

			auto gpFile = SaveTempFile(tempFile, txtFile, start_offset);

			int k = 1;
			k = 2;

			return 0;
		}
		bool Specify(const uint64_t start_offset) override
		{
			return true;
		}

	};

	class GoProRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new GoProRaw(device);
		}
	};


	//class GoPro

	//{
	//	const uint16_t val_0x4750 = 0x5047;
	//	const uint8_t jpg_sing[3] = { 0xFF, 0xD8 , 0xFF };
	//private:
	//	IODevice * device_;
	//	uint32_t cluster_size_;
	//public:
	//	GoPro(IODevice * device)
	//		: device_(device)
	//		, cluster_size_(0)
	//	{

	//	}
	//	~GoPro()
	//	{
	//		if (device_)
	//		{
	//			delete device_;
	//			device_ = nullptr;
	//		}
	//	}
	//	void setClusterSize(const uint32_t cluster_size)
	//	{
	//		this->cluster_size_ = cluster_size;
	//	}
	//	bool ReadCluster(const uint32_t number, Buffer * buffer)
	//	{
	//		if (!buffer)
	//			return false;
	//		uint64_t offset = (uint64_t)number * (uint64_t)cluster_size_;
	//		if (offset >= device_->Size())
	//			return false;

	//		device_->setPosition(offset);
	//		auto bytes_read = device_->ReadData(buffer->data, buffer->data_size);
	//		if (bytes_read != cluster_size_)
	//			return false;
	//		return true;
	//	}
	//	uint32_t SaveFile_WithoutLRV(const uint32_t cluster_number, path_string target_name)
	//	{
	//		File write_file(target_name);
	//		if (!write_file.Open(OpenMode::Create))
	//		{
	//			wprintf(L"Error create file.\n");
	//			return cluster_number;
	//		}

	//		const uint32_t max_count = 6;

	//		uint32_t number = cluster_number;
	//		Buffer buffer(this->cluster_size_);
	//		if (!ReadCluster(number++, &buffer))
	//			return number;

	//		write_file.WriteData(buffer.data, buffer.data_size);

	//		while (ReadCluster(number, &buffer))
	//		{
	//			//if (!isQuickTimeHeader((qt_block_t *) buffer.data) )
	//			//{
	//			//	if (memcmp(buffer.data, jpg_sing, 3) == 0)
	//			//		break;

	//			//	if (calc0x4750(&buffer) < max_count)
	//			//		write_file.WriteData(buffer.data, buffer.data_size);
	//			//}
	//			//else
	//			//wprintf_s(L"Found new qt_header\n");

	//			++number;
	//		}
	//		return number;
	//	}
	//	uint32_t calc0x4750(const Buffer * buffer)
	//	{
	//		uint32_t val_pos = 0;
	//		uint32_t counter = 0;
	//		while (val_pos < buffer->data_size - 1)
	//		{
	//			uint16_t* buff_val = (uint16_t*)(buffer->data + val_pos);
	//			if (*buff_val == val_0x4750)
	//				++counter;

	//			++val_pos;
	//		}
	//		return counter;
	//	}
	//	void execute(const path_string & folder)
	//	{
	//		if (!device_->Open(OpenMode::OpenRead))
	//		{
	//			wprintf(L"Error open.\n");
	//			return;
	//		}
	//		if (cluster_size_ == 0)
	//		{
	//			wprintf(L"Cluster size is 0");
	//			return;
	//		}

	//		uint32_t counter = 0;

	//		uint32_t cluster_number = 508916;
	//		Buffer buffer(cluster_size_);
	//		while (ReadCluster(cluster_number, &buffer))
	//		{
	//			qt_block_t * pQt_block = (qt_block_t *)buffer.data;
	//			//if (isQuickTimeHeader(pQt_block))
	//			{
	//				path_string target_name = toFullPath(folder, counter++, L".mp4");

	//				/*cluster_number =*/ SaveFile_WithoutLRV(cluster_number, target_name);

	//			}
	//			++cluster_number;
	//		}

	//	}

	//};
};

//namespace IO
//{
//	class GoPro2_raw
//	{
//	private:
//		IODevice device_;
//
//		GoPro2_raw(IODevice * device)
//			:device_(device)
//		{
//		}
//
//
//	};
//};