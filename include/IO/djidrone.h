#pragma once

#include "IO/GoPro.h"

using namespace IO;

const uint8_t THM_VIDEO_MAGIC[] = { 0x00 , 0x00 , 0x00 , 0x01 , 0x09 , 0x10 , 0x00 , 0x00 , 0x00 , 0x01 , 0x09 , 0x10 , 0x00 , 0x00 };
const uint8_t THM_VIDEO_MAGIC_SIZE = SIZEOF_ARRAY(THM_VIDEO_MAGIC);

const uint32_t default_cluster_size = 32768;
const uint32_t max_togher_const = 32;


class DjjDronAnalyzer
	: public FileAnalyzer
{


};

class QtTempFile
{

	DataArray::Ptr endChunk_;
	DataArray::Ptr moovData_;
	std::vector<bool> bitmap_;
public:
	void setEndChunk(DataArray::Ptr end_chunk)
	{
		endChunk_ = std::move(end_chunk);
	}
	DataArray * getEndChunk() const
	{
		return endChunk_.get();
	}
	DataArray * getMoovData() const
	{
		return moovData_.get();
	}
	void setMoovData(DataArray::Ptr moov_data)
	{
		moovData_ = std::move(moov_data);
	}
	void addCluster(bool bValue)
	{
		bitmap_.push_back(bValue);
	}
	void setMarketTogther()
	{
		if (max_togher_const > bitmap_.size())
			return;

		const uint32_t nCount = bitmap_.size() - max_togher_const;

		for (uint32_t iCluster = 0; iCluster < nCount; ++iCluster)
		{
			if (bitmap_[iCluster] == false)
			{
				uint32_t nFill = max_togher_const;
				if (iCluster + max_togher_const > nCount)
				{
					nFill = iCluster + max_togher_const - nCount;
				}

				for (uint32_t i = 0; i < nFill; ++i)
					bitmap_[iCluster + i] = false;

				iCluster += nFill;
			}
		}
	}
	std::vector<bool> getBitMap() const
	{
		return bitmap_;
	}

};

class DjiDroneRaw
	: public RawAlgorithm
{
	IODevicePtr device_;
	uint32_t cluster_size_ = default_cluster_size;
public:
	explicit DjiDroneRaw(IODevicePtr device)
		: device_(device)
	{
	}
	
	bool cmpToThmVideo(const DataArray & cluster)
	{
		for (uint32_t i = 0; i < cluster.size() - THM_VIDEO_MAGIC_SIZE; ++i)
		{
			if (memcmp(cluster.data() + i, THM_VIDEO_MAGIC, THM_VIDEO_MAGIC_SIZE) == 0)
				return true;
		}
		return false;
	}
	bool isFTYP(const DataArray & cluster)
	{
		qt_block_t * pQtBlock = (qt_block_t *)cluster.data();
		return cmp_keyword(*pQtBlock, s_ftyp);
	}
	

	uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
	{
		// Save to temp file

		auto tmp_filename = target_file.getFileName() + L".tmp";
		File tmp_file(tmp_filename);
		tmp_file.OpenCreate();

		auto txt_filename = target_file.getFileName() + L".tmp.txt";
		File txt_file(txt_filename);
		txt_file.OpenCreate();

		uint64_t offset = start_offset;

		DataArray cluster(cluster_size_);
		std::string txtToWrite;

		uint32_t moov_pos = 0;

		uint32_t numberMoovSkip = 2;

		device_->setPosition(offset);
		device_->ReadData(cluster.data(), cluster.size());
		txtToWrite = "0";
		tmp_file.WriteData(cluster.data(), cluster.size());
		txtToWrite += "\n";
		txt_file.WriteText(txtToWrite);
		offset += cluster_size_;
		QtTempFile qtTempFile;
		qtTempFile.addCluster(true);

		uint32_t cluster_number = 1;

		std::vector<bool> bitmap;

		bool bValue = false;

		while (offset < device_->Size())
		{
			device_->setPosition(offset);
			device_->ReadData(cluster.data(), cluster.size());

			bValue = true;
			txtToWrite = "0";
			if (isFTYP(cluster) || cmpToThmVideo(cluster))
			{
				bValue = false;
				txtToWrite = "1";
			}

			if (findMOOV_signature(cluster, moov_pos))
			{
				--numberMoovSkip;
				if (numberMoovSkip == 0)
				{
					uint32_t qt_block_pos = moov_pos;
					if (moov_pos > qt_keyword_size)
						qt_block_pos = moov_pos - qt_keyword_size;

					auto endData = makeDataArray(qt_block_pos);
					memcpy(endData->data(), cluster.data(), qt_block_pos);
					qtTempFile.setEndChunk(std::move(endData));

					qt_block_t * moov_block = (qt_block_t *)(cluster.data() + qt_block_pos);
					auto moov_block_size = moov_block->block_size;
					toBE32(moov_block_size);

					DataArray::Ptr moov_data = makeDataArray(moov_block_size);
					uint64_t moov_offset = offset + qt_block_pos;
					device_->setPosition(moov_offset);
					device_->ReadData(moov_data->data(), moov_data->size());
					qtTempFile.setMoovData(std::move(moov_data));

					break;
				}
			}

			qtTempFile.addCluster(bValue);

			tmp_file.WriteData(cluster.data(), cluster.size());
			txtToWrite += "\n";
			txt_file.WriteText(txtToWrite);
			offset += cluster_size_;
			++cluster_number;
		}

		qtTempFile.setMarketTogther();

		GoProRaw gpRAW(device_);
		gpRAW.setBlockSize(cluster_size_);
		uint64_t file_size = gpRAW.SaveUsingBitmap(target_file, tmp_file, qtTempFile.getBitMap());

		target_file.Close();

		auto target_file_name = target_file.getFileName();

		auto target_ptr = makeFilePtr(target_file_name);
		target_ptr->OpenWrite();
		QuickTimeRaw qt_raw(target_ptr);

		uint64_t cur_offset = 0;
		auto ftypAtom = qt_raw.readQtAtom(0);
		if (ftypAtom.compareKeyword(s_ftyp))
		if (ftypAtom.isValid())
		{
			cur_offset += ftypAtom.size();
			auto wideAtom = qt_raw.readQtAtom(cur_offset);
			if ( wideAtom.isValid())
			if (wideAtom.compareKeyword(s_wide))
			{
				cur_offset += wideAtom.size();
				auto mdatAtom = qt_raw.readQtAtom(cur_offset);
				if (mdatAtom.isValid())
				if (mdatAtom.compareKeyword(s_mdat))
				{
					auto moov_pos = cur_offset + mdatAtom.size();
					target_ptr->setPosition(moov_pos);
					target_ptr->WriteData(qtTempFile.getMoovData()->data(), qtTempFile.getMoovData()->size());
					file_size += qtTempFile.getMoovData()->size();

					if (moov_pos > qtTempFile.getEndChunk()->size())
					{
						target_ptr->setPosition(moov_pos - qtTempFile.getEndChunk()->size());
						target_ptr->WriteData(qtTempFile.getEndChunk()->data(), qtTempFile.getEndChunk()->size());
						file_size += qtTempFile.getEndChunk()->size();
					}
					}
			}
		}


		return file_size;
	}
	bool Specify(const uint64_t start_offset)
	{
		return true;
	}

};

class DjiDroneRawFactory
	: public RawFactory
{
public:
	RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
	{
		return new DjiDroneRaw(device);
	}
};