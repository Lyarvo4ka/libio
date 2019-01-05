#pragma once


 
#include "constants.h"
#include "iofs.h"
#include "StandartRaw.h"
#include "Entropy.h"

#include <iostream>

#include <boost\filesystem.hpp>



namespace IO
{

	const char s_ftyp[] = "ftyp";
	const char s_moov[] = "moov";
	const char s_mdat[] = "mdat";
	const char s_wide[] = "wide";
	const char s_skip[] = "skip";
	const char s_free[] = "free";
	const char s_pnot[] = "pnot";
	const char s_prfl[] = "prfl";
	const char s_mvhd[] = "mvhd";
	const char s_clip[] = "clip";
	const char s_trak[] = "trak";
	const char s_udta[] = "udta";
	const char s_ctab[] = "ctab";
	const char s_cmov[] = "cmov";
	const char s_rmra[] = "rmra";
	const char s_uuid[] = "uuid";
	const char s_meta[] = "meta";

	const std::string_view stco_table_name = "stco";


	const int qt_keyword_size = 4;

	using array_keywords = std::vector<const char *>;


	const array_keywords qt_array = { s_ftyp, s_moov, s_mdat, s_wide , s_free, s_skip, s_pnot, s_prfl,
									  s_mvhd, s_clip, s_trak, s_udta, s_ctab, s_cmov, s_rmra , s_uuid, s_meta };



#pragma pack(1)
	struct qt_block_t
	{
		uint32_t block_size;
		char block_type[qt_keyword_size];
	};
#pragma pack()

	//using ListQtBlock = std::list<qt_block_t>;
	inline ByteArray toByteArray(qt_block_t & qtBlock)
	{
		return reinterpret_cast<ByteArray>(&qtBlock);
	}
	inline ByteArray toByteArray(uint64_t value64bit)
	{
		return reinterpret_cast<ByteArray>(value64bit);
	}
	const uint32_t qt_block_struct_size = sizeof(qt_block_t);

	inline bool isQuickTimeKeyword(const qt_block_t & pQtBlock , const char * keyword_name )
	{
		return (memcmp(pQtBlock.block_type, keyword_name, qt_keyword_size) == 0);
	}

	inline bool verify_region(const uint64_t start, const uint64_t size)
	{
		return start < size;
	}
	inline bool isQuickTime(const qt_block_t & pQtBlock)
	{
		for ( auto keyword_name : qt_array)
			if (isQuickTimeKeyword(pQtBlock, keyword_name)) 
				return true;

		return false;
	}

	inline bool isPresentInArrayKeywords(const array_keywords & keywords , const char * key_val)
	{
		for (auto theKeyword : keywords)
		{
			if (memcmp(theKeyword, key_val, qt_keyword_size) == 0)
				return true;
		}
		return false;
	}
	inline bool cmp_keyword(const qt_block_t & qt_block, const char * keyword_name)
	{
		return (memcmp(qt_block.block_type, keyword_name, qt_keyword_size) == 0);
	}

	enum class SearchDirection {kForward , kBackward};

	inline bool findTextTnBlock(const DataArray & data_array, std::string_view textToFind, uint32_t & position , SearchDirection searchDirection = SearchDirection::kForward)
	{
		uint32_t temp_pos = 0;
		for (uint32_t pos = 0; pos < data_array.size() - textToFind.length(); ++pos)
		{
			if (searchDirection == SearchDirection::kForward) 
				temp_pos = pos;
			else
				temp_pos = data_array.size() - static_cast<uint32_t>(textToFind.length()) - pos;

			if (memcmp(data_array.data() + temp_pos, textToFind.data(), textToFind.length()) == 0)
			{
				position = pos;
				return true;
			}
		}
		return false;
	}

	inline bool findTextTnBlockFromEnd(const DataArray & data_array, std::string_view textToFind, uint32_t & position)
	{
		return findTextTnBlock(data_array, textToFind, position, SearchDirection::kBackward);
	}


	inline bool findMOOV_signature(const DataArray & data_array, uint32_t & position)
	{
		return findTextTnBlock(data_array, s_moov, position);
	}

	class QtHandle
	{
		qt_block_t qtBlock_ = qt_block_t();
		uint64_t offset_ = 0;
		uint64_t size_ = 0;
		bool valid_ = false;
	public:
		QtHandle()
		{}
		QtHandle(const uint64_t offset, const uint64_t size)
			: offset_(offset)
			, size_(size)
		{
			if (size != 0)
				setValid();
		}
		//QtHandle operator=(const QtHandle & new_handle)
		//{
		//	memcpy(&qtBlock_, &new_handle, sizeof(qt_block_t));
		//}
		//QtHandle operator=(const QtHandle && new_handle)
		//{
		//	memcpy(&qtBlock_, &new_handle, sizeof(qt_block_t));
		//}
		qt_block_t * getBlock()
		{
			return &qtBlock_;
		}
		const char * block_type() const
		{
			return qtBlock_.block_type;
		}
		void setBlock(const qt_block_t & qt_block)
		{
			memcpy(&qtBlock_, &qtBlock_, qt_block_struct_size);
		}
		void setOffset(const uint64_t offset)
		{
			offset_ = offset;
		}
		bool compareKeyword(const std::string & keyword_name)
		{
			if (keyword_name.length() != qt_keyword_size)
			{
				LOG_MESSAGE("Error keyword length is not equal to 4 bytes.");
				return false;
			}
			return (memcmp(keyword_name.c_str(), qtBlock_.block_type, qt_keyword_size) == 0);
			
		}
		uint64_t offset() const
		{
			return offset_;
		}
		void setSize(const uint64_t size)
		{
			size_ = size;
			if (size_ != 0)
				setValid();
		}
		uint64_t size() const
		{
			return size_;
		}
		void setValid()
		{
			valid_ = true;
		}
		bool isValid() const
		{
			return valid_;
		}
	};
	using QuickTimeList = std::list<QtHandle>;


	

	class QuickTimeRaw
		: public StandartRaw
	{
	private:
		QuickTimeList keywordsList_;
		uint64_t sizeToWrite_ = 0;
	public:
		explicit QuickTimeRaw(IODevicePtr device)
			: StandartRaw(device)
		{
		}
		explicit QuickTimeRaw(const path_string & fileName )
			: StandartRaw(makeFilePtr(fileName))
		{
		}

		virtual ~QuickTimeRaw()
		{
		}
		uint64_t readQtAtom(const uint64_t start_offset, qt_block_t & qt_block)
		{
			if ( (start_offset + qt_block_struct_size) > this->getSize())
				return 0;
			this->setPosition(start_offset);
			auto bytes_read = this->ReadData(toByteArray(qt_block), qt_block_struct_size);
			if (bytes_read != qt_block_struct_size)
				return 0;

			if (!isQuickTime(qt_block))
				return 0;

			if (qt_block.block_size == 0)
				return 0;

			toBE32(qt_block.block_size);

			return readQtAtomSize(qt_block.block_size, start_offset);
		}
		QtHandle readQtAtom(const uint64_t start_offset)
		{
			QtHandle atom_handle;
			atom_handle.setOffset(start_offset);
			auto atom_size = readQtAtom(start_offset, *atom_handle.getBlock());
			if  (atom_size == 0)
				return QtHandle();

			atom_handle.setSize(atom_size);
			return atom_handle;
		}
		uint64_t readAllQtAtoms(const uint64_t start_offset, QuickTimeList & keywordsList)
		{
			uint64_t keyword_offset = start_offset;
			uint64_t write_size = 0;
			uint64_t full_size = 0;

			while (true)
			{
				auto qt_handle = readQtAtom(keyword_offset);
				if (!qt_handle.isValid())
					break;

				full_size += qt_handle.size();
				keywordsList.push_back(qt_handle);
				keyword_offset += qt_handle.size();
			}

			return full_size;
		}
		uint64_t readQtAtomSize(const qt_block_t &qt_block, uint64_t keyword_offset)
		{
			return readQtAtomSize(qt_block.block_size, keyword_offset);
		}
		uint64_t readQtAtomSize(const uint32_t block_size, uint64_t keyword_offset)
		{
			uint64_t write_size = block_size;

			if (write_size == 1)
			{
				uint64_t ext_size = 0;
				uint64_t ext_size_offset = keyword_offset + qt_block_struct_size;
				if (ext_size_offset + sizeof(uint64_t) >= this->getSize())
					return 0;

				this->setPosition(ext_size_offset);
				this->ReadData(toByteArray(ext_size), sizeof(uint64_t));

				toBE64(ext_size);
				write_size = ext_size;
			}
			return write_size;
		}

		QtHandle findQtKeyword(const uint64_t start_offset, const std::string & keyword_name)
		{
			if (keyword_name.length() != qt_keyword_size)
			{
				LOG_MESSAGE("Error keyword length is not equal to 4 bytes.");
				return QtHandle();
			}

			uint64_t keyword_pos = start_offset;
			DataArray data_array(this->getBlockSize());
			uint32_t bytesRead = 0;
			uint32_t bytesToRead = 0;

			while (keyword_pos < this->getSize())
			{
				bytesToRead = calcBlockSize(keyword_pos, this->getSize(), data_array.size());
				this->setPosition(keyword_pos);
				bytesRead = this->ReadData(data_array.data(), bytesToRead);

				for (uint32_t iSector = 0; iSector < bytesRead; ++iSector /*+= default_sector_size*/)
				{
					qt_block_t * pQt_block = reinterpret_cast<qt_block_t*>(data_array.data() + iSector);
					if (cmp_keyword(*pQt_block, keyword_name.c_str()))
					{
						auto keyword_block = readQtAtom(keyword_pos + iSector);
						if (keyword_block.isValid())
							return keyword_block;
					}
				}
				keyword_pos += bytesRead;
			}


			return QtHandle();
		}

		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (!target_file.isOpen())
			{
				wprintf(L"Target file wasn't opened.\n");
				return 0;
			}
			
			if (sizeToWrite_ > 0)
				return appendToFile(target_file, start_offset, sizeToWrite_);

			return 0;
		}
		bool Specify(const uint64_t start_offset) override
		{
			keywordsList_.clear();
			
			auto sizeKeywords = readAllQtAtoms(start_offset, keywordsList_);
			if (isPresentMainKeywords(keywordsList_))
			{
				sizeToWrite_ = sizeKeywords;
				return true;
			}
			return false;
		}
		bool isPresentMainKeywords(const QuickTimeList & keywordsList_)
		{
			bool bmdat = false;
			bool bmoov = false;

			for (auto & refQtHandle : keywordsList_)
			{
				if (memcmp(refQtHandle.block_type(), s_mdat, qt_keyword_size) == 0)
					bmdat = true;
				else if (memcmp(refQtHandle.block_type(), s_moov, qt_keyword_size) == 0)
					bmoov = true;

				if (bmdat && bmoov)
					return true;
			}
			
			return false;
		}

	};


	class QuickTimeRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new QuickTimeRaw(device);
		}
	};

	const uint8_t mdat_header_start[] = { 0x00, 0x00, 0x00, 0x02, 0x09, 0x10, 0x00, 0x00 };

	inline bool isDigitOrAlpha(char symbol)
	{
		if (isdigit(symbol))
			return true;

		if (isalpha(symbol))
			return true;

		if (symbol == '_')
			return true;

		return false;
	}

#include<ctype.h>

	inline std::string ReadFileName(const path_string file_path)
	{
		const uint32_t file_enty_size = 18;
		char entry_buff[file_enty_size + 1];
		const uint32_t name_end_offset = 0x1D0; // 0x01CA; // 0x1C9; //0x1D0

		const uint32_t end_1024 = 1024;

		auto qtfile = makeFilePtr(file_path);
		qtfile->OpenRead();
		QuickTimeRaw qt_raw(qtfile);

		uint64_t offset = 0;

		auto qt_handle = qt_raw.readQtAtom(offset);



		if (qtfile->Size() > end_1024)
		{
			auto start_search = qtfile->Size() - end_1024;

			auto skip_handle = qt_raw.findQtKeyword(start_search , s_skip);
			if (skip_handle.isValid())
			{
				uint64_t skip_offset = skip_handle.offset();
				uint64_t name_offset = skip_offset - name_end_offset;
				DataArray name_data(file_enty_size);
				qtfile->setPosition(name_offset);
				qtfile->ReadData(name_data);

				ZeroMemory(entry_buff, file_enty_size + 1);
				memcpy(entry_buff, name_data.data(), file_enty_size);
				for (auto i = 0; i < file_enty_size; ++i)
				{
					if (!isDigitOrAlpha(entry_buff[i]))
						return "";
				}

				std::string new_name(entry_buff);
				return new_name;

			}
		}


		return "";
	}


/*
{
  "qt_fragment":
  { 
		"header":
          [
            {
                "textdata":"mdat",
                "offset": 4
            }            
            
          ],
          "extension": ".mov"
	}
}
*/


	/*
	�������������� ����������������� ������ QuickTime.
	1. ���� ��������� "mdat".
	2. ����������� �� ������ ������� �������� � ��������� "mdat" (����� ���� 64bit).
	3. ����������� �� ������� �������. � ���� ��������� "ftyp".
	4. ����� "ftyp" ������ ���� ��������� "moov", � "free" �� ������������.
	5. ��������� ������� ��������� "ftyp", "moov", "free", � ����� "mdat".
	*/
	class QTFragmentRaw 
		: public QuickTimeRaw
	{
	public:
		QTFragmentRaw(IODevicePtr device)
			: QuickTimeRaw(device)
		{
		}

		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			setBlockSize(32768);
			qt_block_t mdat_block = qt_block_t();
			// read mdat size
			auto mdat_atom = readQtAtom(start_offset);
			if (!mdat_atom.isValid())
				return 0;

			uint64_t find_pos = start_offset + mdat_atom.size();
			find_pos = alingToSector(find_pos);
			find_pos += this->getSectorSize();

			auto ftyp_atom = findQtKeyword(find_pos, s_ftyp);
			if (!ftyp_atom.isValid())
				return 0;

			uint64_t moov_offset = ftyp_atom.offset() + ftyp_atom.size();
			auto moov_atom = readQtAtom(moov_offset);
			if (moov_atom.isValid())
				if (moov_atom.compareKeyword(s_moov))
				{
					uint64_t writeSize = ftyp_atom.size() + moov_atom.size();

					uint64_t free_offset = moov_atom.offset() + moov_atom.size();
					auto free_atom = readQtAtom(free_offset);
					if (free_atom.isValid())
						if (free_atom.compareKeyword(s_free))
							writeSize += free_atom.size();

					uint64_t target_size = appendToFile(target_file, ftyp_atom.offset(), writeSize);
					target_size += appendToFile(target_file, start_offset, mdat_atom.size());
					return target_size;
				}
			return 0;
		}


		
		bool Specify(const uint64_t start_offset) override
		{
			return true;
		}
	};

	
	class QTFragmentRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new QTFragmentRaw(device);
		}
	};

	struct DataEntropy
	{
		double entropy;
		int numberNulls_;
		DataArray::Ptr data_array;
	};

	/*
		�������������� ����������������� ����� ������ (QuickTime). ���� ����� �������� ���������� ���� ����������.
		���� ������ �������� ��� ������� ��������. ���� �� �������� ��� �������� �� ������� �� �����������.
		������������� ���� � 3 �������� ���� �� ��������� ������ �������. 

		//////////////////////////////////////
		����� ������ ��������� 
		1. ����������� ��������� ����� � ��������. � ����� �������� ���������� ����� ������ �����.
		2. �� �������� �� 16 ���������.

		"ESER_YDXJ":{
		"header":
		[
		{
		"textdata":"ftyp",
		"offset": 4
		},
		{
		"textdata":"moov",
		"offset": 4
		},
		{
		"textdata":"mdat",
		"offset": 4
		}

		],
		"extension": ".mov"
		},

	*/
	// stco
	struct ChunkOffsetAtom
	{
		uint32_t atom_size;
		uint8_t	 atom_type[qt_keyword_size];
		uint8_t	version;
		uint8_t flags[3];
		uint32_t number_entries;
	};

	class ESER_YDXJ_QtRaw
		: public QuickTimeRaw
	{
		const uint32_t cluster_size_ = 32768;
	public:
		ESER_YDXJ_QtRaw(IODevicePtr device)
			: QuickTimeRaw(device)
		{

		}

		uint32_t cluster_size() const
		{
			return cluster_size_;
		}
		uint64_t getClusterNumber(uint64_t offset)
		{
			return offset / cluster_size_;
		}
		std::string ClusterNumberNullsToString(const uint64_t cluster_number, const uint32_t number_nulls)
		{
			std::string write_string;
		
			try
			{
				write_string = boost::lexical_cast<std::string>(cluster_number) + "\t";
				write_string += (boost::lexical_cast<std::string>(number_nulls));
			}
			catch (boost::bad_lexical_cast & ex)
			{
				std::cout << "cought error " << ex.what();
			}
			return write_string;

		}
		void logNumberNulls(File & log_file, const uint64_t cluster_number, const uint32_t number_nulls)
		{
			if (log_file.isOpen())
			{
				auto str = ClusterNumberNullsToString(cluster_number, number_nulls);
				if (!str.empty())
				{
					str += +"\n";
					log_file.WriteData((ByteArray)str.data(), (uint32_t)str.size());
				}
			}
		}
		void logEndLine(File & log_file)
		{
			if (log_file.isOpen())
			{
				std::string end_line = "\n";
				log_file.WriteData((ByteArray)end_line.data(), (uint32_t)end_line.size());
			}
		}

		uint64_t saveEndFile(File & target_file , uint32_t moov_offset , uint64_t offset , uint64_t file_size)
		{
			uint32_t moov_pos = moov_offset - sizeof(s_moov) + 1;
			appendToFile(target_file, offset - cluster_size(), moov_pos);
			file_size += moov_pos;

			uint64_t moov_position = offset - cluster_size() + moov_pos;
			qt_block_t qt_block = { 0 };

			setPosition(moov_position);
			ReadData((ByteArray)&qt_block, qt_block_struct_size);
			if (isQuickTime(qt_block))
			{
				toBE32((uint32_t &)qt_block.block_size);

				appendToFile(target_file, moov_position, qt_block.block_size);
				file_size += qt_block.block_size;
			}
			return file_size;

		}

		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{

			const double entropy_border = 7.99292;
			const uint32_t nulls_border = 187;

			uint64_t offset = start_offset;
			uint64_t file_size = 0;
			uint64_t cluster_number = 0;

			path_string log_file_name = target_file.getFileName() + L".txt";
			File log_file(log_file_name);
			if (!log_file.Open(OpenMode::Create))
				return 0;

			for (auto i = 0; i < 11; ++i)
			{
				auto cluster_data = IO::makeDataArray((uint32_t)cluster_size());
				setPosition(offset);
				cluster_number = getClusterNumber(offset - start_offset);
				if (!ReadData(cluster_data->data(), cluster_data->size()))
				{
					printf("Error read cluster %I64d\n", cluster_number);
					break;
				}
				appendToFile(target_file, offset, cluster_size());
				uint32_t null_count = calc_nulls(cluster_data->data(), cluster_data->size());

				logNumberNulls(log_file, cluster_number, null_count);
				offset += cluster_size();
			}


			cluster_number = getClusterNumber(offset - start_offset);
			std::unique_ptr<DataEntropy> prev = std::make_unique<DataEntropy>();
			prev->data_array = IO::makeDataArray(cluster_size());
			setPosition(offset);
			ReadData(prev->data_array->data(), prev->data_array->size());
			prev->numberNulls_ = calc_nulls(prev->data_array->data(), prev->data_array->size());
			prev->entropy = calcEntropy(prev->data_array->data(), prev->data_array->size());
			appendToFile(target_file, offset, cluster_size());
			logNumberNulls(log_file, cluster_number, prev->numberNulls_);
			offset += cluster_size();


			//uint64_t curr_offset = offset;
			cluster_number = getClusterNumber(offset - start_offset);
			std::unique_ptr<DataEntropy> curr = std::make_unique<DataEntropy>();
			curr->data_array = IO::makeDataArray(cluster_size());
			setPosition(offset);
			ReadData(curr->data_array->data(), curr->data_array->size());
			offset += cluster_size();
			curr->entropy = calcEntropy(curr->data_array->data(), curr->data_array->size());
			curr->numberNulls_ = calc_nulls(curr->data_array->data(), curr->data_array->size());
			logNumberNulls(log_file, cluster_number, curr->numberNulls_);
			//appendToFile(target_file, offset, cluster_size);

			uint32_t nCount = 0;
			uint32_t moov_offset = 0;

			uint32_t number_nulls = 0;
			uint32_t skip_count = 0;

			while (true)
			{
				cluster_number = getClusterNumber(offset - start_offset);
//				cluster_number = 2;
				std::unique_ptr<DataEntropy> next = std::make_unique<DataEntropy>();
				next->data_array = IO::makeDataArray(cluster_size());
				setPosition(offset);
				if (!ReadData(next->data_array->data(), next->data_array->size()))
				{
					printf("Error read cluster %I64d\n", cluster_number);
					break;
				}
				next->entropy = calcEntropy(next->data_array->data(), next->data_array->size());
				next->numberNulls_ = calc_nulls(next->data_array->data(), next->data_array->size());

				//logNumberNulls(log_file, cluster_number, curr->numberNulls_);

				//nCount = 0;
				//if (prev->entropy > entropy_border)
				//	++nCount;
				//if (curr->entropy > entropy_border)
				//	++nCount;
				//if (next->entropy > entropy_border)
				//	++nCount;

				nCount = 0;
				if (prev->numberNulls_ < nulls_border)
					++nCount;
				if (curr->numberNulls_ < nulls_border)
					++nCount;
				if (next->numberNulls_ < nulls_border)
					++nCount;


				if (nCount < 2)
				{
					++skip_count;
					if (skip_count > 1)
					{
						skip_count = 0;
						// skiping other 14 - cluster
						offset = offset + 14 * cluster_size();

						cluster_number = getClusterNumber(offset - start_offset);
						prev = std::make_unique<DataEntropy>();
						prev->data_array = IO::makeDataArray(cluster_size());
						setPosition(offset);
						ReadData(prev->data_array->data(), prev->data_array->size());
						prev->numberNulls_ = calc_nulls(prev->data_array->data(), prev->data_array->size());
						prev->entropy = calcEntropy(prev->data_array->data(), prev->data_array->size());
						if (findMOOV(prev->data_array->data(), prev->data_array->size(), moov_offset))
						{
							return saveEndFile(target_file, moov_offset, offset, file_size);
						}
						appendToFile(target_file, offset, cluster_size());

						offset += cluster_size();

						cluster_number = getClusterNumber(offset - start_offset);
						curr = std::make_unique<DataEntropy>();
						curr->data_array = IO::makeDataArray(cluster_size());
						setPosition(offset);
						ReadData(curr->data_array->data(), curr->data_array->size());
						curr->entropy = calcEntropy(curr->data_array->data(), curr->data_array->size());
						curr->numberNulls_ = calc_nulls(curr->data_array->data(), curr->data_array->size());
						if (findMOOV(curr->data_array->data(), curr->data_array->size(), moov_offset))
						{
							return saveEndFile(target_file, moov_offset, offset, file_size);
						}
						offset += cluster_size();
						continue;
					}
				}


			
				bool foundResult = findMOOV(curr->data_array->data(), curr->data_array->size(), moov_offset);
				int k = 1;
				k = 0;
				if (!foundResult)
				{ 
					if (nCount >= 2)
					{
						appendToFile(target_file, offset - cluster_size(), cluster_size());
						//if (skip_count > 0)
						//{
						//	auto continues_clusters = boost::lexical_cast<std::string>(skip_count);
						//	std::string str_skip = "\n\n\n" + continues_clusters + "\t -skiped" + "\n\n\n";
						//	log_file.WriteData((ByteArray)str_skip.data(), str_skip.size());
						//	skip_count = 0;
						//}
					}
					else
					{
						//++skip_count;
						printf("skip cluster #%I64d\r\n", cluster_number);
						std::string str_skip = "\t -skiped";
						log_file.WriteData((ByteArray)str_skip.data(),(uint32_t) str_skip.size());
					}
				}
				else
				{
					return saveEndFile(target_file, moov_offset, offset, file_size);
					//uint32_t moov_pos = moov_offset - sizeof(s_moov) + 1 ;
					//appendToFile(target_file, offset - cluster_size(), moov_pos);
					//file_size += moov_pos;

					//uint64_t moov_offset = offset - cluster_size() + moov_pos;
					//qt_block_t qt_block = { 0 };

					//setPosition(moov_offset);
					//ReadData((ByteArray)&qt_block, qt_block_struct_size);
					//if (isQuickTime(qt_block) )
					//{
					//	toBE32((uint32_t &)qt_block.block_size);

					//	appendToFile(target_file, moov_offset, qt_block.block_size);
					//	file_size += qt_block.block_size;
					//}
					//return file_size;

				}
				logEndLine(log_file);
				auto write_str = ClusterNumberNullsToString(cluster_number, next->numberNulls_);
				log_file.WriteData((ByteArray)write_str.data(), (uint32_t)write_str.size());

				prev = std::move(curr);
				curr = std::move(next);



				++cluster_number;
				offset += cluster_size();
				file_size += cluster_size();
			}
			return file_size;

		}
		bool findMOOV(const ByteArray data, const uint32_t size , uint32_t & moov_pos)
		{
			for (moov_pos = 0; moov_pos < size - sizeof(s_moov); ++moov_pos)
				if (memcmp(data + moov_pos, s_moov, sizeof(s_moov) ) == 0)
					return true;

			return false;
		}

		bool Specify(const uint64_t start_offset) override
		{
			return true;
		}
		bool Verify(const IO::path_string & file_path) override
		{
			auto target_file = IO::makeFilePtr(file_path);
			if (!target_file->Open(IO::OpenMode::OpenRead))
			{
				wprintf(L"Target file wasn't opened.\n");
				return false;

			}
			QuickTimeRaw qtRaw(target_file);
			//ListQtBlock qtList ;
			QuickTimeList keywrodsList;
			auto file_size = qtRaw.readAllQtAtoms(0, keywrodsList);
			return qtRaw.isPresentMainKeywords(keywrodsList);

		}

	};

	class ESER_YDXJ_QtRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new ESER_YDXJ_QtRaw(device);
		}
	};


	const char str_gpssv[] = "$GPGSV";
	const uint32_t str_gpssv_size = SIZEOF_ARRAY(str_gpssv) - 1;
	const char eightNulls[] = { 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 };
	const uint32_t eightNulls_size = SIZEOF_ARRAY(eightNulls);



	/*
	�������������� ����������������� ����� ������ (QuickTime). ����������� �������� ���������� ���(.gps) �  (.3gf).
	- gps �������������� �������� ��������� ����� ( $GPGSV ).
	- 3gf �������������� �������� 8-� ������.

	*/

	class BlackVue_QtRaw
		:public ESER_YDXJ_QtRaw
	{
	private:
		uint32_t cluster_size_ = 32768;
	public:
		BlackVue_QtRaw(IODevicePtr device)
			: ESER_YDXJ_QtRaw(device)
		{

		}

		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			uint64_t offset = 0;
			const path_string gps_ext = L".gps";
			const path_string threeGF_ext = L".3gf";


			boost::filesystem::path target_path(target_file.getFileName());
			auto onlyFileName = target_path.stem();
			auto folderPath = target_path.parent_path();
			auto gpsFileName = addBackSlash(folderPath.generic_wstring()) + onlyFileName.generic_wstring() + gps_ext;
			auto gps_file = makeFilePtr(gpsFileName);
			if (!gps_file->Open(OpenMode::Create))
			{
				wprintf_s(L"Error create  file %s", gpsFileName.c_str());
				return 0;
			}

			auto threeGFFileName = addBackSlash(folderPath.generic_wstring()) + onlyFileName.generic_wstring() + threeGF_ext;
			auto thrgf_file = makeFilePtr(threeGFFileName);
			if (!thrgf_file->Open(OpenMode::Create))
			{
				wprintf_s(L"Error create  file %s", threeGFFileName.c_str());
				return 0;
			}


			// Save first cluster
			appendToFile(target_file, start_offset, cluster_size_);
			uint64_t file_size = cluster_size_;

			DataArray data_buffer(cluster_size_);
			offset = start_offset;
			offset += cluster_size_;

			uint32_t moov_offset = 0;
			uint32_t cluster_number = 1;

			while (true)
			{
				setPosition(offset);
				ReadData(data_buffer.data(), data_buffer.size());

				if (!findMOOV(data_buffer.data(), data_buffer.size(), moov_offset))
				{
					if (isGPS(data_buffer.data(), data_buffer.size()))
					{

						gps_file->WriteData(data_buffer.data(), data_buffer.size());
						int k = 1;
						k = 2;
					}
					else if (is3GF(data_buffer.data(), data_buffer.size()))
					{
						thrgf_file->WriteData(data_buffer.data(), data_buffer.size());
						int k = 1;
						k = 2;

					}
					else
					{
						appendToFile(target_file, offset, data_buffer.size());
						file_size += data_buffer.size();
					}

				}
				else
				{
					uint32_t moov_pos = moov_offset - sizeof(s_moov) + 1;
					appendToFile(target_file, offset , moov_pos);
					file_size += moov_pos;

					uint64_t moov_offset = offset + moov_pos;
					qt_block_t qt_block = { 0 };

					setPosition(moov_offset);
					ReadData((ByteArray)&qt_block, qt_block_struct_size);
					if (isQuickTime(qt_block))
					{
						toBE32((uint32_t &)qt_block.block_size);

						appendToFile(target_file, moov_offset, qt_block.block_size);
						file_size += qt_block.block_size;
					}
					return file_size;

				}
				offset += cluster_size_;
				++cluster_number;
			}





			return 0;
		}

		bool Specify(const uint64_t start_offset) override
		{
			return true;
		}
		
		bool Verify(const IO::path_string & file_path) override
		{
			return ESER_YDXJ_QtRaw::Verify(file_path);
		}
		bool isGPS(ByteArray data, const uint32_t size)
		{
			for (uint32_t i = 0; i < size - str_gpssv_size; ++i)
			{
				if (memcmp(data + i, str_gpssv, str_gpssv_size) == 0)
					return true;
			}

			return false;
		}

		bool is3GF(ByteArray data, const uint32_t size)
		{
			for (uint32_t i = 0; i < size - eightNulls_size; ++i)
			{
				if (memcmp(data + i, eightNulls, eightNulls_size) == 0)
					return true;
			}

			return false;
		}


	};

	class BlackVue_QtRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new BlackVue_QtRaw(device);
		}
	};


	/*
		�������������� ����������������� ����� ������ (QuickTime). � ������ ���� ��������� ����������("moov"), ����� ��� ����� ����� ("mdat").
		� ��������� ���� ������ ������ �����. �� ����� ������� ����� ��������� ������ "mdat".
		����� ����� "mdat" � ����� ��������.
	*/
	class Canon80D_FragmentRaw
		: public QuickTimeRaw
	{
	public:
		Canon80D_FragmentRaw(IODevicePtr device)
			: QuickTimeRaw(device)
		{

		}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			const uint32_t fullFileSizeOffset = 2214;

			// 1. ������ header.
			qt_block_t qtBlock = qt_block_t();
			setPosition(start_offset);
			ReadData((IO::ByteArray)&qtBlock, qt_block_struct_size);
			if (!isQuickTime(qtBlock))
			{
				wprintf(L"Error wrong qt_header.\n");
				return 0;
			}
			uint32_t header_size = qtBlock.block_size;
			toBE32(header_size);

			uint64_t offset = start_offset;
			offset += header_size;
			// 2. ������ ��������� ������ ���� moov
			ZeroMemory(&qtBlock, qt_block_struct_size);
			setPosition(offset);
			ReadData((IO::ByteArray)&qtBlock, qt_block_struct_size);
			if (!isQuickTime(qtBlock)||(memcmp(qtBlock.block_type , s_moov, qt_keyword_size) != 0 ))
			{
				wprintf(L"Error wrong moov.\n");
				return 0;
			}

			uint32_t moov_size = qtBlock.block_size;
			toBE32(moov_size);

			// 3. ������ ������ ������ �����.
			uint32_t full_size = 0;
			offset = start_offset;
			offset += fullFileSizeOffset;
			setPosition(offset);
			ReadData((IO::ByteArray) & full_size, 4);


			uint32_t mdat_size = full_size - moov_size - header_size;

			int l = 0;
			l = 1;

			qt_block_t mdat_cmp;
			mdat_cmp.block_size = mdat_size;
			toBE32(mdat_cmp.block_size);
			memcpy(mdat_cmp.block_type, s_mdat, qt_keyword_size);

			DataArray buffer(getBlockSize());

			//const uint32_t GB_4 = UINT32_MAX;
			uint64_t mdat_start = start_offset;
			//uint64_t search_end = mdat_start + GB_4;
			//if (search_end > getSize())
			//	search_end = getSize();
			uint32_t bytes_read = 0;
			while (mdat_start <= getSize())
			{
				setPosition(mdat_start);
				//setPosition(0x7000000);
				bytes_read = ReadData(buffer.data(), buffer.size());
				if (bytes_read == 0)
					return 0;

				for (uint32_t iSector = 0; iSector < buffer.size(); iSector += default_sector_size)
				{
					memcpy(&qtBlock, buffer.data() + iSector, qt_block_struct_size);
					if (memcmp(&qtBlock, &mdat_cmp, qt_block_struct_size) == 0)
					{
						uint64_t mdat_offset = mdat_start + iSector;
						appendToFile(target_file, start_offset, header_size + moov_size);
						uint32_t write_size = mdat_cmp.block_size;
						toBE32(write_size);
						return appendToFile(target_file, mdat_offset, write_size);

					}
				}
				mdat_start += buffer.size();
				
			}



			return 0;
		}
		bool Specify(const uint64_t start_offset) override
		{
			return true;
		}

		bool Verify(const IO::path_string & file_path) override
		{
			return true;
		}
	};

	class Canon80D_FragmentRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new Canon80D_FragmentRaw(device);
		}
	};

	struct EntropyCluster
	{
		EntropyCluster(uint64_t offset , double theEntropy)
			: cluster_offset(offset)
			, entropy(theEntropy)
		{}
		uint64_t cluster_offset = 0;
		double entropy = 0.0;
	};

	struct FTYP_start
	{
		bool bFound = false;
		uint64_t offset = 0;
	};

	//const char GP_START_DATA[] = { 0x00 , 0x00 , 0x00 , 0x02 , 0x09 , 0x10 , 0x00 , 0x00 , 0x00 , 0x20 , 0x06 , 0x00 , 0x0D , 0x80 , 0x99 , 0xD0 };

}
		// save fragment only 'mdat' data, when found nulls more then 5000, skip this cluster.
		//uint64_t SaveFragmentMdat(File * target_file, uint64_t offset, uint32_t & copy_size)
		//{
		//	Buffer buffer(block_size_);
		//	ZeroMemory(buffer.data, block_size_);

		//	uint32_t bytes_to_copy = block_size_;

		//	uint64_t cur_pos = 0;
		//	uint64_t read_pos = 0;
		//	uint32_t write_size = 0;
		//	const uint32_t nulls_count = 5000;
		//	uint32_t null_counter = 0;

		//	uint32_t bytesRead = 0;
		//	uint32_t bytesWritten = 0;
		//	uint64_t write_offset = target_file->Size();

		//	bool isNulls = false;
		//	uint32_t real_size = 0;

		//	while (cur_pos < copy_size)
		//	{
		//		read_pos = offset + cur_pos;
		//		write_size = block_size_;
		//		null_counter = 0;
		//		isNulls = false;
		//		if ((read_pos % block_size_) != 0)
		//		{
		//			write_size = block_size_ - read_pos % block_size_;
		//		}

		//		bytes_to_copy = calcBlockSize(cur_pos, copy_size, write_size);

		//		device_->setPosition(read_pos);
		//		bytesRead = device_->ReadData(buffer.data, bytes_to_copy);
		//		if (bytesRead == 0)
		//			break;

		//		for (uint32_t iByte = 0; iByte < bytes_to_copy; ++iByte)
		//		{
		//			if (buffer.data[iByte] == 0)
		//			{
		//				++null_counter;
		//				if (null_counter > nulls_count)
		//				{
		//					isNulls = true;
		//					break;
		//				}
		//			}

		//		}
		//		if (isNulls)
		//		{
		//			cur_pos += block_size_ ;
		//			copy_size += block_size_ ;
		//			continue;
		//		}



		//		target_file->setPosition(write_offset);
		//		bytesWritten = target_file->WriteData(buffer.data, bytes_to_copy);
		//		if (bytesWritten == 0)
		//			break;

		//		write_offset += bytesWritten;
		//		cur_pos += bytesWritten;
		//		real_size += bytesWritten;
		//	}
		//	return cur_pos;

		//}
		//uint64_t AppendDataToFile(File * target_file, uint64_t offset, uint64_t copy_size)
		//{
		//	if (!device_)
		//		return 0;
		//	if (!target_file)
		//		return 0;
		//	if (!target_file->isOpen())
		//		return 0;

		//	Buffer buffer(block_size_);
		//	ZeroMemory(buffer.data, block_size_);

		//	uint64_t cur_pos = 0;
		//	uint64_t read_pos = 0;
		//	uint32_t bytes_to_copy = block_size_;

		//	uint32_t bytesRead = 0;
		//	uint32_t bytesWritten = 0;
		//	uint64_t write_offset = target_file->Size();


		//	while (cur_pos < copy_size)
		//	{
		//		bytes_to_copy = calcBlockSize(cur_pos, copy_size, block_size_);

		//		read_pos = offset + cur_pos;

		//		device_->setPosition(read_pos);
		//		bytesRead = device_->ReadData(buffer.data, bytes_to_copy);
		//		if (bytesRead == 0)
		//			break;

		//		target_file->setPosition(write_offset);
		//		bytesWritten = target_file->WriteData(buffer.data, bytes_to_copy);
		//		if (bytesWritten == 0)
		//			break;

		//		write_offset += bytesWritten;
		//		cur_pos += bytesWritten;
		//	}


		//	return cur_pos;
		//}


/*

	/*
	Make fragment raw files.
	1. 'mdat' data block
	2. Header data ( 'mdat' keyword in the end of block)
	
	const uint32_t default_sectors_per_cluster = 64;
	const uint32_t defalut_cluster_size = default_sector_size * default_sectors_per_cluster;

	class QuickTimeFragmentRaw
		: public QuickTimeRaw
	{
	private:
		uint32_t cluster_size_;
	public:
		QuickTimeFragmentRaw(IODevice * device)
			: QuickTimeRaw(device)
			, cluster_size_(defalut_cluster_size)
		{

		}
		void setClusterSize(uint32_t cluster_size)
		{
			this->cluster_size_ = cluster_size;
		}
		void execute(const path_string & target_folder) override
		{
			auto source = this->getDevice();
			if (!source->Open(OpenMode::OpenRead))
			{
				wprintf(L"Error to open.\n");	// ????????
				return;
			}

			bool bResult = false;

			uint64_t offset = 0;
			uint64_t header_offset = 0;
			uint32_t counter = 0;
			const uint32_t four_clusters = 4;

			uint32_t bytes_read = 0;

			uint32_t cluster_per_mdat = 0;
			uint64_t mdat_offset = 0;
			uint64_t mdat_entry = 0;

			while (true)
			{
				if (!findHeaderOffset(offset, header_offset))
				{
					wprintf(L"Not Found Header\n");
					break;
				}
				auto target_name = toFullPath(target_folder, counter++, L".mp4");
				File write_file(target_name);
				if (!write_file.Open(OpenMode::Create))
				{
					wprintf(L"Error create file\n");
					break;
				}
				qt_block_t qt_block = {0};
				uint64_t keyword_offset = header_offset;

				while (true)
				{
					qt_block_t qt_block = { 0 };
					source->setPosition(keyword_offset);
					int bytes_read = source->ReadData((uint8_t*)&qt_block, sizeof(qt_block_t));
					if (bytes_read == 0)
						break;
					if (qt_block.block_size == 0)
						break;

					to_big_endian32((uint32_t &)qt_block.block_size);



					if (!isQuickTime(&qt_block))
						break;

					if (memcmp(qt_block.block_type, qt_array[2], qt_keyword_size) == 0)
					{
						AppendDataToFile(&write_file, keyword_offset, sizeof(qt_block_t));
						uint32_t mdat_size = qt_block.block_size;
						cluster_per_mdat = mdat_size / cluster_size_ + 1;
						uint32_t mdat_aling_clusters = cluster_per_mdat * cluster_size_;
						if (header_offset > mdat_aling_clusters)
						{
							mdat_offset = header_offset - cluster_per_mdat * cluster_size_;
							AppendDataToFile(&write_file, mdat_offset, mdat_size - sizeof(qt_block_t));
						}

						break;
					}

					uint64_t write_size = ReadQtAtomSize(qt_block, keyword_offset);
					if (write_size == 0)
						break;

					auto bytes_written = AppendDataToFile(&write_file, keyword_offset, write_size);
					if (bytes_written != write_size)
						break;

					keyword_offset += write_size;
				}
				

				offset = header_offset + cluster_size_;


			}

		}
	};


	/*

	1. 'mdat' data block and save offset
	2. In the end of 'mdat', aling to cluster size and plus one cluster (+1 cluster) must 'ftyp' and 'moov' atom
	3. Save header data block
	4. Save 'mdat'.


	class CanonFragmentRaw
		:public QuickTimeRaw
	{
	public:
		CanonFragmentRaw(IODevice * device)
			: QuickTimeRaw(device)
		{


		}
		bool isMdat(const qt_block_t * pQtBlock)
		{
			if (memcmp(pQtBlock->block_type, s_mdat, qt_keyword_size) == 0)
				return true;
			return false;
		}
		bool isQuickTimeHeader(const qt_block_t * pQtBlock) override
		{
			return isMdat(pQtBlock);
		}
		bool findMdatOffset(uint64_t offset, uint64_t & header_offset)
		{
			return findHeaderOffset(offset, header_offset);
		}
		uint64_t readKeywordsSizes(const uint64_t start_offset, const array_keywords & key_array)
		{
			uint64_t keyword_offset = start_offset;
			auto device = this->getDevice();
			uint64_t result_size = 0;
			while (true)
			{
				qt_block_t qt_block = { 0 };
				device->setPosition(keyword_offset);
				int bytes_read = device->ReadData((uint8_t*)&qt_block, sizeof(qt_block_t));
				if (bytes_read == 0)
					return keyword_offset;
				if (qt_block.block_size == 0)
					break;

				to_big_endian32((uint32_t &)qt_block.block_size);

				if (!isPresentInArrayKeywords(key_array , qt_block.block_type))
					break;

				uint64_t theSize = ReadQtAtomSize(qt_block, keyword_offset);
				if (theSize == 0)
					break;

				result_size += theSize;
				keyword_offset += theSize;
			}

			return result_size;
		}
		void execute(const path_string & target_folder)
		{
			if (!this->getDevice()->Open(OpenMode::OpenRead))
			{
				wprintf(L"Error to open.\n");	// ????????
				return;
			}

			bool bResult = false;

			uint64_t offset = 0;
			uint64_t header_offset = 0;
			uint32_t counter = 0;
			while (true)
			{
				if (!findMdatOffset(offset, header_offset))
				{
					wprintf(L"Not Found Mdat Keyword\n");
					break;
				}
				array_keywords array_mdatANDskip = { s_mdat , s_skip };
				auto size_MdatFree = readKeywordsSizes(header_offset, array_mdatANDskip);
				if (size_MdatFree > 0)
				{
					uint64_t aling_cluster = size_MdatFree / this->getBlockSize();
					++aling_cluster;
					aling_cluster *= this->getBlockSize();
					uint64_t ftyp_offset = header_offset + aling_cluster;
					array_keywords array_ftypANDmoov = { s_ftyp , s_moov };
					auto size_FtypAndMoov = readKeywordsSizes(ftyp_offset, array_ftypANDmoov);
					if (size_FtypAndMoov > 0)
					{
						auto target_file = toFullPath(target_folder, counter++, L".mov");
						File new_file(target_file);
						if (new_file.Open(OpenMode::Create))
						{
							AppendDataToFile(&new_file, ftyp_offset, size_FtypAndMoov);
							AppendDataToFile(&new_file, header_offset, size_MdatFree);

							offset = ftyp_offset;
							offset += this->getBlockSize();
							continue;
						}

					}

				}
				offset += default_sector_size;
			}


		}




	};
	

	class QuitTimeRawNoSize
		: public QuickTimeRaw
	{
	public:
		QuitTimeRawNoSize(IODevice * device)
			: QuickTimeRaw(device)
		{

		}
		void execute(const path_string & target_folder) override
		{
			auto source = this->getDevice();
			if (!source->Open(OpenMode::OpenRead))
			{
				wprintf(L"Error to open.\n");	// ????????
				return;
			}

			bool bResult = false;

			uint64_t offset = 0;
			uint64_t header_offset = 0;
			uint32_t counter = 0;

			while (true)
			{
				if (!findHeaderOffset(offset, header_offset))
				{
					wprintf(L"Not Found Header\n");
					break;
				}
				
				auto target_file = toFullPath(target_folder, counter++, L".mov");
				offset = SaveWithOutMDatSize(header_offset, target_file);
				offset += default_sector_size;


			}
		}
		uint64_t SaveWithOutMDatSize( const uint64_t start_offset , const path_string & target_name)
		{
			File write_file(target_name);
			if (!write_file.Open(OpenMode::Create))
				return start_offset;

			auto source = this->getDevice();

			qt_block_t qt_atom = { 0 };

			uint32_t bytes_read = 0;
			uint64_t mdat_offset = 0;
			uint32_t mdat_target_pos = 0;
			uint64_t moov_offset = 0;

			uint64_t current_pos = start_offset;

			uint64_t full_targe_size = 0;

			bool bFoundMoov = false;
			
			// read 'ftyp'
			source->setPosition(current_pos);
			bytes_read = source->ReadData((ByteArray)&qt_atom, qt_block_struct_size);
			if (memcmp(qt_atom.block_type, s_ftyp, qt_keyword_size) == 0)
			{
			//	printf("%s\t", qt_atom.block_type);
				to_big_endian32((uint32_t &)qt_atom.block_size);
				current_pos += qt_atom.block_size;
				mdat_target_pos = qt_atom.block_size;


				source->setPosition(current_pos);
				bytes_read = source->ReadData((ByteArray)&qt_atom, qt_block_struct_size);
				if (memcmp(qt_atom.block_type, s_mdat, qt_keyword_size) == 0)
				{
					//printf("%s\t", qt_atom.block_type);

					mdat_offset = current_pos;

					Buffer buffer(this->getBlockSize());
					while (true)
					{
						source->setPosition(current_pos);
						bytes_read = source->ReadData(buffer.data, buffer.data_size);
						if ( bytes_read == 0)
							break;

						for (uint32_t iByte = 0; iByte < bytes_read; ++iByte)
						{
							qt_block_t * pQt_block = (qt_block_t *)&buffer.data[iByte];
							if (memcmp(pQt_block->block_type, s_moov, qt_keyword_size) == 0)
							{
								//printf("%s\t", pQt_block->block_type);

								moov_offset = current_pos;
								moov_offset += iByte;

								full_targe_size = moov_offset - start_offset;
								to_big_endian32((uint32_t &)pQt_block->block_size);
								full_targe_size += pQt_block->block_size;

								bFoundMoov = true;
								break;
							}

						}
						if (bFoundMoov)
							break;
						current_pos += bytes_read;
					}

				}
			}

			if (bFoundMoov)
				if (full_targe_size > 0)
				{
					AppendDataToFile(&write_file, start_offset, full_targe_size);
					uint32_t new_mdat_size = (uint32_t)(moov_offset - mdat_offset);
					to_big_endian32(new_mdat_size);

					write_file.setPosition(mdat_target_pos);
					write_file.WriteData((ByteArray)&new_mdat_size, sizeof(uint32_t));


				}

			full_targe_size /= default_sector_size;
			++full_targe_size;
			full_targe_size *= default_sector_size;
			full_targe_size += start_offset;
			return full_targe_size;
		}
	};
	*/
