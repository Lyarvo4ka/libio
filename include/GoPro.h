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
	class GoProRaw
		: public QuickTimeRaw
	{
		const uint32_t GP_CLUSTER_SIZE = 32768;
		const uint32_t GP_LRV_SKIP_COUNT = 16;
	private:
		uint32_t cluster_size_ = GP_CLUSTER_SIZE;
		std::list<EntropyCluster> listEntopies_;
		FTYP_start ftyp_start_;
	public:
		GoProRaw(IODevicePtr device)
			: QuickTimeRaw(device)
		{}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			const double entropy_border = 7.993;

			if (!target_file.isOpen())
			{
				if (!target_file.Open(OpenMode::Create))
					return 0;
			}

			if (!ftyp_start_.bFound)
			{
				printf("Not found ftyp start header.");
				return 0;
			}

			uint64_t written_size = save_QT_header(target_file, ftyp_start_.offset);
			uint64_t offset = start_offset;

			auto iter = listEntopies_.begin();
			auto tmpIter = listEntopies_.begin();
			uint32_t counter = 0;
			uint32_t numCmp = GP_LRV_SKIP_COUNT;
			while (iter != listEntopies_.end())
			{
				if (iter->entropy < entropy_border)
				{
					printf("# %I64d %lf\r\n", iter->cluster_offset / cluster_size_, iter->entropy);
					numCmp = GP_LRV_SKIP_COUNT - 1;
					counter = 0;
					++counter;

					tmpIter = iter;
					++tmpIter;
					while (tmpIter != listEntopies_.end())
					{
						printf(" # %I64d %lf\r\n", tmpIter->cluster_offset / cluster_size_, tmpIter->entropy);
						--numCmp;
						if (tmpIter->entropy < entropy_border)
							++counter;
						++tmpIter;
						if (numCmp == 0)
							break;
					}

					if (counter > 10)
					{
						uint64_t start = iter->cluster_offset / cluster_size_;
						uint64_t end = tmpIter->cluster_offset / cluster_size_;
						printf("\r\n\r\n");
						printf("%I64d - skip\r\n", start);
						printf("%I64d - count\r\n", end - start);
						iter = tmpIter;
						continue;
					}
					written_size += appendToFile(target_file, iter->cluster_offset, cluster_size_);
				}
				else
				{
					//setPosition(iter->cluster_offset);
					written_size += appendToFile(target_file, iter->cluster_offset, cluster_size_);
				}
				++iter;
			}



			return written_size;
		}
		uint32_t save_QT_header(File & target_file, const uint64_t start_offset)	// save only ftyp and moov
		{
			uint32_t write_size = 0;
			ListQtBlock listQtBlocks;
			readQtAtoms(start_offset, listQtBlocks);
			if (listQtBlocks.size() > 2)
			{
				auto iter = listQtBlocks.begin();
				auto qt_block = *iter;
				if (cmp_keyword(qt_block, s_ftyp))
				{
					write_size = qt_block.block_size;
					++iter;
					qt_block = *iter;
					if (cmp_keyword(qt_block, s_moov))
					{
						write_size += qt_block.block_size + qt_block_struct_size;
						return appendToFile(target_file, start_offset, write_size);
					}
				}
			}
			return 0;
		}
		bool isFTYP_keyword(const qt_block_t & qtBlock) const
		{
			return (memcmp(qtBlock.block_type, s_ftyp, qt_keyword_size) == 0);
		}
		bool Specify(const uint64_t start_offset) override
		{
			uint32_t bytes_read = 0;
			uint64_t offset = start_offset;
			DataArray data_cluster(cluster_size_);

			qt_block_t * pQtBlock = nullptr;

			//1. read cluster
			//2. calc entropy
			//3. add to table
			while (offset < this->getSize())
			{
				setPosition(offset);
				bytes_read = this->ReadData(data_cluster.data(), data_cluster.size());
				if (bytes_read == 0)
					break;

				pQtBlock = (qt_block_t *)data_cluster.data();

				if (isFTYP_keyword(*pQtBlock))
				{
					ftyp_start_.bFound = true;
					ftyp_start_.offset = offset;
					return true;
				}
				if (memcmp(data_cluster.data(), jpg_signature, SIZEOF_ARRAY(jpg_signature)) != 0)
				{
					EntropyCluster entropy_cluster(offset, calcEntropy(data_cluster.data(), data_cluster.size()));
					listEntopies_.push_back(entropy_cluster);
				}

				offset += cluster_size_;
			}

			return false;
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