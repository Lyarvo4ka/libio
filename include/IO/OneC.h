#pragma once

#include "IO\IODevice.h"

#include <boost/algorithm/string.hpp>



namespace IO
{
	struct HeaderObject{

	char sig[8]; // сигнатура У1CDBOBV8Ф

	int length; // длина содержимого объекта

	int version1;

	int version2;

	unsigned int version;

	unsigned int blocks[1018];

	};

	struct RootObject
	{
		char lang[32];

		int numblocks;

		int * tableblocks;
	};

	class File1C
	{
	private:
		path_string fileName_;

	};

	enum FILE_TABLE_NAME { DESCR  =0 ,INDEX , BLOB };
	struct Table1COffsets
	{
		Table1COffsets()
		{
			threeTables.resize(3, 0);
		}
		std::vector<uint64_t> threeTables;
		bool isValid() const
		{
			uint32_t counter = 0;
			for (const auto & table : threeTables)
			{
				if (table == 0)
					++counter;
			}

			return (counter != 3);
		
		}
	};

	class Table1CHandle
	{
	private:
		path_string filePath_;
		path_string tableName_;
		std::list<uint64_t> tablesOffset_;
		uint64_t page_size_ = 4096;
	public:
		Table1CHandle(const path_string & pathToFile)
			: filePath_(pathToFile)
		{

		}
		std::vector<path_string> splitStrings(path_string str, const path_string delimiters)
		{
			const path_string endLineDelimeter = L"\n";
			std::vector<path_string> strs;
			boost::split(strs, str, boost::is_any_of(endLineDelimeter)/*,boost::algorithm::token_compress_on*/);
			std::vector<path_string> no_str_bracked;
			std::vector<IO::path_string> new_vec_string;
			for (const auto & tmp_str : strs)
			{
				if (!tmp_str.empty())
					if (tmp_str.compare(L"\"") != 0)
					{
						new_vec_string.push_back(tmp_str);
					}
			}
			return new_vec_string;
		}
		Table1COffsets getFilesOffset(std::vector<path_string> & strings_1C)
		{
			Table1COffsets offsetsTables;
			// First Entry must 1C table name
			if (strings_1C.empty())
				return Table1COffsets();

			tableName_ = strings_1C.at(0);

			const path_string FILES_NAME = L"\"Files\"";
			auto iter = std::find(begin(strings_1C), end(strings_1C), FILES_NAME);
			if (iter == strings_1C.end())
			{
				LOG_MESSAGE(L"Not found keyword (\"Files\")");
				return Table1COffsets();
			}

			for (int i = 0; i < 3; ++i)
			{
				++iter;
				if (iter == strings_1C.end())
					break;

				auto tmp_str = *iter;
				try
				{
					auto tmp_val = boost::lexical_cast<uint64_t>(tmp_str);
					offsetsTables.threeTables.at(i) = tmp_val;
				}
				catch (const boost::bad_lexical_cast& ex)
				{
					LOG_MESSAGE("Could not convert string to uint64_t");
					break;
				}
				
			}
			return offsetsTables;
		}
		bool readTable(Table1COffsets offsets)
		{

		}
	};



}
