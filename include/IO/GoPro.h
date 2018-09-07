#pragma once

#include "QuickTime.h"

#include <string>
#include <algorithm>
#include <iterator>
#include <vector>

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace IO
{

	const uint32_t default_nulls_boder = 230;//187;
	const uint32_t default_number_together = 16;
	const uint32_t GP_TOGHER_LIMIT = 5;

	inline static std::string clusterNullsToSstring(const uint32_t cluster_number, const uint32_t number_nulls)
	{
		auto cluster_str = std::to_string(cluster_number);
		auto number_nulls_str = std::to_string(number_nulls);

		return  cluster_str + " \t" + number_nulls_str;
	}

#pragma pack(1)
	struct STCO_Table
	{
		qt_block_t stco_block;
		uint8_t version;
		uint8_t flags[3];
		uint32_t number_of_endries;
	};
#pragma pack()

	inline void createMarkerMapSTCO(const path_string & fileName, const uint32_t cluster_size)
	{
		const std::string_view gp_keyword = "GP";

		auto txt_file_name = fileName + L".txt";
		File txt_file(txt_file_name);
		txt_file.OpenCreate();

		File qtFile(fileName);
		qtFile.OpenRead();

		DataArray cluster_data(cluster_size);

		auto num_clusters = qtFile.Size() / cluster_size;

		uint64_t offset = qtFile.Size() - cluster_size;
		uint32_t bytesRead = 0;
		uint32_t table_pos = 0;
		bool bFoundTable = false;

		while (true)
		{
			qtFile.setPosition(offset);
			bytesRead = qtFile.ReadData(cluster_data);
			if (findTextTnBlockFromEnd(cluster_data, stco_table_name, table_pos))
			{
				table_pos = offset + cluster_size - table_pos - 4;
				bFoundTable = true;
				break;
			}

			if (offset <= cluster_size)
				break;
			offset -= cluster_size;
		}
		if (!bFoundTable)
			return;

		// read table
		if (table_pos < qt_keyword_size)
			return;

		qtFile.setPosition(table_pos - qt_keyword_size);

		qt_block_t stco_block = { 0 };
		qtFile.ReadData((ByteArray)&stco_block, sizeof(qt_block_t));
		toBE32(stco_block.block_size);

		DataArray stco_data(stco_block.block_size);
		qtFile.setPosition(table_pos - qt_keyword_size);
		qtFile.ReadData(stco_data);

		STCO_Table * pSCTO_Table = (STCO_Table *)stco_data.data();
		uint32_t * posPointer = (uint32_t *)(stco_data.data() + sizeof(STCO_Table));

		auto numberOf = pSCTO_Table->number_of_endries;

		toBE32(numberOf);

		DataArray two_bytes(2);

		std::vector<uint32_t> clusters(num_clusters + 1, 0);

		uint32_t cur_pos = 0;
		for (auto i = 0; i < numberOf; ++i)
		{
			cur_pos = posPointer[i];
			toBE32(cur_pos);
			if (cur_pos < qtFile.Size())
			{
				qtFile.setPosition(cur_pos);
				qtFile.ReadData(two_bytes);
				if (memcmp(two_bytes.data(), gp_keyword.data(), two_bytes.size()) == 0)
				{
					++clusters[cur_pos / cluster_size];
				}
			}
		}

		uint64_t pos = 0;

		std::vector<uint32_t> nullsInCluster(num_clusters + 1, 0);
		for (auto i = 0; i < num_clusters; ++i)
		{
			qtFile.setPosition(pos);
			qtFile.ReadData(cluster_data);
			nullsInCluster[i] = calc_nulls(cluster_data);
			pos += cluster_size;
		}


		for (auto i = 0; i < clusters.size(); ++i)
		{
			std::string write_str = std::to_string(i) + " \t";
			write_str += std::to_string(nullsInCluster[i]) + " \t";
			if (clusters[i] > 0)
				write_str += " GP marker " + std::to_string(clusters[i]);
			write_str += "\n";
			//if ( i != 0)
			//if (i % 15 == 0)
			//	write_str += '\n';
			txt_file.WriteText(write_str);

		}

		//std::vector<uint32_t> markerMap()



	}
	class GoProAnalyzer :
		public FileAnalyzer
	{
	private:
		path_string fileName_;
		uint32_t block_size_ = default_block_size;
		File goProFile_;
		bool bValid_ = false;
	public:
		GoProAnalyzer(const path_string & file_name)
			: fileName_(file_name)
			, goProFile_(file_name)
		{
		}
		~GoProAnalyzer()
		{
			Close();
		}
		uint32_t getBlockSize() const
		{
			return block_size_;
		}
		bool isValid() const
		{
			return bValid_;
		}
		void setValid()
		{
			bValid_ = true;
		}
		void Close()
		{
			goProFile_.Close();
		}
		bool find_stco_table(uint32_t & table_offset)
		{

			uint64_t offset = goProFile_.Size() - getBlockSize();
			uint32_t bytesRead = 0;
			uint32_t table_pos = 0;

			DataArray block_data(getBlockSize());

			while (true)
			{
				goProFile_.setPosition(offset);
				bytesRead = goProFile_.ReadData(block_data);
				if (findTextTnBlockFromEnd(block_data, stco_table_name, table_pos))
				{
					table_offset = offset + getBlockSize() - table_pos - 4;
					return true;
				}

				if (offset <= getBlockSize())
					break;
				offset -= getBlockSize();
			}
			return false;
		}

		void Analyze(const IO::path_string & file_name) override
		{
			const std::string_view gp_keyword = "GP";
			
			goProFile_.OpenRead();

			auto file_size = goProFile_.Size();
			if (file_size < getBlockSize())
				return;

			uint32_t table_pos = 0;
			if (!find_stco_table(table_pos))
				return;

			// read table
			if (table_pos < qt_keyword_size)
				return;

			goProFile_.setPosition(table_pos - qt_keyword_size);

			qt_block_t stco_block = { 0 };
			goProFile_.ReadData((ByteArray)&stco_block, sizeof(qt_block_t));
			toBE32(stco_block.block_size);

			DataArray stco_data(stco_block.block_size);
			auto table_offset = table_pos - qt_keyword_size;
			if (table_offset >= file_size)
				return;
			if (table_offset + stco_data.size() > file_size)
				return;

			goProFile_.setPosition(table_pos - qt_keyword_size);
			goProFile_.ReadData(stco_data);

			STCO_Table * pSCTO_Table = (STCO_Table *)stco_data.data();
			uint32_t * posPointer = (uint32_t *)(stco_data.data() + sizeof(STCO_Table));

			auto numberOf = pSCTO_Table->number_of_endries;

			toBE32(numberOf);

			DataArray two_bytes(2);


			uint32_t cur_pos = 0;
			for (auto i = 0; i < numberOf; ++i)
			{
				cur_pos = posPointer[i];
				toBE32(cur_pos);
				if (cur_pos > file_size)
					return;
				goProFile_.setPosition(cur_pos);
				goProFile_.ReadData(two_bytes);
				if (memcmp(two_bytes.data(), gp_keyword.data(), two_bytes.size()) != 0)
					return;
			}

			setValid();
		}
	};


	class GoProTempFile
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

		uint32_t countOfSixteen(std::vector<uint32_t>::iterator startIter)
		{
			return std::count_if(startIter, startIter + number_together_, [=](const uint32_t nulls_val) {
				return nulls_val > nulls_border_;
			});
			return 0;
		}
		uint32_t countOfSixteen(const size_t position)
		{
			uint32_t number_of = 0;
			for (uint32_t i = 0; i < number_together_; ++i)
			{
				if (clusterMap_[position + i] > nulls_border_)
					++number_of;
			}
			return number_of;
		}

		void analyze()
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

			const uint32_t countLimit = GP_TOGHER_LIMIT;

			for (size_t iCluster = 1; iCluster < clusterMap_.size(); ++iCluster)
			{
				if ((iCluster % number_together_) == 0)
				{
					if (iCluster + number_together_ > clusterMap_.size())
						break;
					auto number_of = countOfSixteen(iCluster);
					if (number_of >= countLimit)
					{
						for (auto i = 0; i < number_together_; ++i)
							bitmap_[iCluster + i] = false;
						iCluster += number_together_ - 1;
					}
				}

			}
		}
	};





	class GoProRaw
		: public QuickTimeRaw
	{
		const uint32_t GP_CLUSTER_SIZE = 32768;
		const uint32_t GP_LRV_SKIP_COUNT = 16;

	private:
		uint32_t nulls_border_ = default_nulls_boder;//187

	public:
		GoProRaw(IODevicePtr device)
			: QuickTimeRaw(device)
		{
			setBlockSize(GP_CLUSTER_SIZE);
		}
		GoProTempFile SaveTempFile(File & temp_file, File & txt_file , const uint64_t start_offset)
		{
			uint64_t offset = start_offset;

			const auto cluster_size = this->getBlockSize();

			uint32_t bytesRead = 0;

			DataArray data_array(cluster_size);

			uint32_t number_nulls = 0;
			//uint32_t cluster_number = 0;

			std::vector<uint32_t> cluster_map;
			uint32_t moov_pos = 0;

			GoProTempFile gpFile;
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

				std::string str_toWrite = clusterNullsToSstring(cluster , number_nulls);
				txt_file.WriteText(str_toWrite);
				if (number_nulls > nulls_border_)
					txt_file.WriteText(" -skipped");
				txt_file.WriteText(endLine);

				++cluster;
				offset += bytesRead;
			}

			return gpFile;
		}
		uint64_t SaveUsingBitmap(File & target_file , File & temp_file, const std::vector<uint32_t>  & bitmap)
		{
			DataArray data_array(getBlockSize());
			uint64_t file_size = 0;
			uint64_t src_pos = 0;
			for (auto iCluster = 0; iCluster < bitmap.size(); ++iCluster)
			{
				if (bitmap[iCluster] )
				{
					src_pos = iCluster * getBlockSize();
					tempFile.setPosition(src_pos);
					tempFile.ReadData(data_array);
					target_file.WriteData(data_array.data() , data_array.size());
					file_size += getBlockSize();
				}
			}
			return file_size;
	
		}
		uint64_t SaveMoov(const path_string & target_file_name)
		{
			uint64_t file_size = 0;
			QuickTimeRaw qt_raw(target_file_name);
			target_file.OpenWrite();
			auto ftypAtom = qt_raw.readQtAtom(0);
			if (ftypAtom.compareKeyword(s_ftyp))
			if (ftypAtom.isValid())
			{
				auto mdatAtom = qt_raw.readQtAtom(ftypAtom.size());
				if (mdatAtom.isValid())
				if (mdatAtom.compareKeyword(s_mdat))
				{
					auto moov_pos = ftypAtom.size() + mdatAtom.size();
					target_ptr->setPosition(moov_pos);
					target_ptr->WriteData(gpFile.getMoovData()->data(), gpFile.getMoovData()->size());
					file_size += gpFile.getMoovData()->size();

					if ( moov_pos > gpFile.getEndChunk()->size())
					{
						target_ptr->setPosition(moov_pos - gpFile.getEndChunk()->size());
						target_ptr->WriteData(gpFile.getEndChunk()->data(), gpFile.getEndChunk()->size());
						file_size += gpFile.getEndChunk()->size();
					}
				}
			}
			return file_size;
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

			
			auto file_size = SaveUsingBitmap(targer_file , tempFile , gpFile.getBitMap());
			target_file.Close();
			file_size +=SaveMoov(target_file.getFileName());  

			return file_size;
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

