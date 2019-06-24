#pragma once

#include "IO\constants.h"
#include <stdint.h>
#include <string>
#include <sstream>
#include <iomanip>

#include "IO\IODevice.h"
#include "IO\Finder.h"

namespace IO
{

	inline bool is_between(const uint32_t from_val, const uint32_t to_val , const uint32_t theValue)
	{
		if (theValue >= from_val && theValue <= to_val)
			return true;
		return false;
	}
	inline std::string toString(const uint32_t int_val, const uint32_t fill_count)
	{
		std::stringstream ss;
		ss << std::setw(fill_count) << std::setfill('0') << int_val;
		return ss.str();
	}
#pragma pack( push, 1)
	struct zbk_DateTime
	{
		uint8_t day;
		uint8_t month;
		uint16_t year;
		uint8_t hour;
		uint8_t sec;
		bool isValidDay() const
		{
			return is_between(1, 31, day);
		}
		bool isValidMounth() const
		{
			return is_between(1, 12, month);
		}
		bool isValidYear() const
		{
			if (year >= 1970)
				return true;
			return false;
		}
		bool isValidHour() const
		{
			return is_between(0, 23, hour);
		}
		bool isValideSeconds() const
		{
			return is_between(0, 59, sec);
		}
		bool isVaid() const
		{
			if (isValidDay())
			if (isValidMounth())
			if (isValidYear())
			if (isValidHour())
			if (isValideSeconds())
				return true;

			return false;
		}

		std::string to_string()
		{
			std::string strResult;
			if (isVaid())
			{
				auto s_year = toString(year, 4);
				auto s_month = toString(month, 2);
				auto s_day = toString(day, 2);
				auto s_hour = toString(hour, 2);
				auto s_sec = toString(sec,2);

				strResult = s_year + '-' + s_month + '-' + s_day + '-' + s_hour + '-' + s_sec;
			}
			return strResult;
		}
	};
#pragma pack(pop)

	class ZBKAnalyzer
	{
		std::string str_date_;
	public:
		void analize(const path_string & fileName)
		{
			File file(fileName);
			if (!file.Open(OpenMode::OpenRead))
				return;

			DataArray data_array(1024);
			auto bytesRead = file.ReadData(data_array);
			if (bytesRead == 0)
				return;

			const uint32_t struct_offset = 16;

			zbk_DateTime * zbkDateTime = (zbk_DateTime *)(data_array.data() + struct_offset);
			str_date_ = zbkDateTime->to_string();
			if (str_date_.empty())
				return;
		}
		std::string getDateString() const
		{
			return str_date_;
		}
	};

	void renameToDateTime(const path_string & src_folder, const path_string & dst_folder)
	{
		const path_string zbk_ext = L".zbk";
		Finder finder;
		finder.add_extension(zbk_ext);
		finder.FindFiles(src_folder);

		uint32_t counter = 0 ;

		for (auto src_file : finder.getFiles())
		{
			ZBKAnalyzer zbk_analizer;
			zbk_analizer.analize(src_file);
			auto str_date = zbk_analizer.getDateString();
			if (str_date.empty())
				continue;
			path_string wStrDate(str_date.begin(), str_date.end());
			auto numberStr = toString(counter++, 5);
			path_string wCounter(numberStr.begin() , numberStr.end());
			auto new_file_name = dst_folder + wStrDate + L"_"+ wCounter + L".zbk";
			try
			{
				fs::rename(src_file, new_file_name);
			}
			catch (const fs::filesystem_error& e)
			{
				std::cout << "Error: " << e.what() << std::endl;
			}
		}
	}

	

}