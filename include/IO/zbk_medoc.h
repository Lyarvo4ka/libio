#pragma once

#include <stdint.h>
#include <string>
#include <sstream>
#include <iomanip>

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

	

}