#pragma once

#include <stdint.h>

namespace IO
{
	bool is_between(const uint32_t from_val, const uint32_t to_val , const uint32_t theValue)
	{
		if (theValue >= from_val && theValue <= to_val)
			return true;
		return false;
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
	};
#pragma pack(pop)
}