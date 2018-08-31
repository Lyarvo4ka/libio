#pragma once

#include "QuickTime.h"

#include <string>
#include <algorithm>
#include <iterator>
#include <vector>

#include <experimental/filesystem>

namespace IO
{

	struct NullsInCluster
	{
		uint32_t cluster_number;
		uint32_t number_nulls;

	};

	struct StartAmountSkip
	{
		uint32_t start_pos;
		uint32_t amount_skip;
	};

	const uint32_t default_nulls_boder = 187;
	const uint32_t default_number_together = 16;

	class GoProFile
	{
		std::vector<uint32_t> clusterMap_;
		std::vector<bool> bitmap_;
		DataArray::Ptr endChunk_;
		DataArray::Ptr moovData_;
		uint32_t number_together_ = default_number_together;
		bool lowQuality_ = false;
		bool valid_ = false;
		uint32_t nulls_border_ = default_nulls_boder;
	public:
		void setCluserMap(std::vector<uint32_t> cluster_map)
		{
			clusterMap_ = cluster_map;
		}
		void addNumberNulls(const uint32_t cluster_number, const uint32_t number_nulls)
		{
			clusterMap_.push_back(number_nulls);
		}
		void setEndChunk(DataArray::Ptr end_chunk)
		{
			endChunk_ = std::move(end_chunk);
		}
		DataArray * getMoovData() const
		{
			return moovData_.get();
		}
		void setMoovData(DataArray::Ptr moov_data)
		{
			moovData_ = std::move(moov_data);
		}
		bool isLowQuality() const
		{
			return lowQuality_;
		}
		void setNullsBorder(const uint32_t nulls_border)
		{
			nulls_border_ = nulls_border;
		}
		bool isValid() const
		{
			return valid_;
		}
		void analyzeToLowQuility()
		{
			uint32_t lowQualityCount = 0;
			for (uint32_t i = 0; i < number_together_; ++i)
			{
				if (clusterMap_.at(i) > nulls_border_)
					++lowQualityCount;
			}

			if (lowQualityCount > 10)
			{
				lowQuality_ = true;
				return;
			}
		}
		std::vector<bool> getBitMap() const
		{
			return bitmap_;
		}
		// <<<<<<<<<<<<<<<
		bool findBackwardNumberOf(const uint32_t start, uint32_t & number_of)
		{
			const uint32_t WindowSize = 3;
			if (start < WindowSize)
				return false;
			uint32_t amountTo = start -1;
			uint32_t curr_pos = start;

			uint32_t prev = clusterMap_[curr_pos--];
			uint32_t curr = clusterMap_[curr_pos];
			uint32_t next = 0;

			uint32_t toCheck = number_together_ - 1;///????
			uint32_t window_size = 0;
			uint32_t skip_count = 1;	///????

			for (uint32_t i = 0; i < amountTo; ++i)
			{
				if (toCheck == 0)
					break;

				next = clusterMap_[--curr_pos];
				window_size = 0;

				if (prev > nulls_border_)
					++window_size;
				if (curr > nulls_border_)
					++window_size;
				if (next > nulls_border_)
					++window_size;

				if (window_size >= 2)
				{
					++skip_count;
					// to skip
				}
				else
					break;

				prev = curr;
				curr = next;

				--toCheck;
			}


			number_of = skip_count;
			return true;
		}

		bool findFromEnd(const uint32_t start , uint32_t & found_pos)
		{
			 //Start from end to analyze number together
			auto rPos = clusterMap_.size() -1 - start;
			auto findIter = std::find_if(clusterMap_.rbegin() + rPos, clusterMap_.rend(), [=](const uint32_t nulls_val) {
				return nulls_val > nulls_border_;
			}); 
			if (findIter == clusterMap_.rend())
				return false;
			found_pos = clusterMap_.size() -1 - std::distance(clusterMap_.rbegin(), findIter);
			return true;

		}


		void analyze( )
		{
			valid_ = false;
			auto number_clusters = clusterMap_.size();
			// if first 16 clusters is less then border then skip file
			if (number_clusters < number_together_)
				return;

			analyzeToLowQuility();
			if (isLowQuality())
				return;

			uint32_t pos = 0;
			bitmap_.resize(clusterMap_.size(), true);
			auto bFound = findFromEnd(clusterMap_.size() - 1, pos);
			if (!bFound)
				return;
			uint32_t number_of = 0;
			auto bResult = findBackwardNumberOf(pos, number_of);
			if (bResult)
				if (number_of > 1)
					for (auto i = 0; i < number_of; ++i)
						bitmap_[pos  + 1- number_of + i] = false;
			//pos = pos - number_of - 1;
			do
			{
				pos = pos - number_of;
				bResult = findBackwardNumberOf(pos, number_of);
				if (bResult)
					if (number_of>1)
						for ( auto i = 0 ; i < number_together_ ; ++i)
							bitmap_[pos - number_together_ + i] = false;
			} while (bResult);

		}


	};

	std::string clusterNullsToSstring(const uint32_t cluster_number, const uint32_t number_nulls)
	{
		auto cluster_str = std::to_string(cluster_number);
		auto number_nulls_str = std::to_string(number_nulls);

		return  cluster_str + " \t" + number_nulls_str;
	}


namespace fs = std::experimental::filesystem;

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
			gpFile.analyze();
			if (gpFile.isLowQuality())
			{
				tempFile.Close();
				fs::remove(tempFile.getFileName());
				txtFile.Close();
				fs::remove(txtFile.getFileName());
				return 0;
			}

			uint64_t src_pos = 0;
			

			DataArray data_array(getBlockSize());
			auto bitmap = gpFile.getBitMap();
			for (auto iCluster = 0; iCluster < bitmap.size(); ++iCluster)
			{
				if (bitmap[iCluster] )
				{
					src_pos = iCluster * getBlockSize();
					tempFile.setPosition(src_pos);
					tempFile.ReadData(data_array);
					target_file.WriteData(data_array.data() , data_array.size());
				}
			}
			target_file.Close();
			auto target_ptr = makeFilePtr(target_file.getFileName());
			target_ptr->OpenWrite();
			QuickTimeRaw qt_raw(target_ptr);
			target_file.OpenWrite();
			auto ftypAtom = qt_raw.readQtAtom(0);
			if (ftypAtom.isValid())
			{
				auto mdatAtom = qt_raw.readQtAtom(ftypAtom.size());
				if (mdatAtom.isValid())
				{
					auto moov_pos = ftypAtom.size() + mdatAtom.size();
					target_ptr->setPosition(moov_pos);
					target_ptr->WriteData(gpFile.getMoovData()->data(), gpFile.getMoovData()->size());

				
				}
			}
				




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


};

