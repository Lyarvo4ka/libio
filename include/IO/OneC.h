#pragma once

#include "IO\IODevice.h"

#include <boost/algorithm/string.hpp>




namespace IO
{
	std::vector<path_string> splitStrings(path_string str , const path_string delimiters)
	{
		std::vector<path_string> strs;
		boost::split(strs, str, boost::is_any_of(delimiters)/*,boost::algorithm::token_compress_on*/);
		std::vector<path_string> no_str_bracked;
		//const wchar_t symbol[] = L"\"";
		//for (auto & src_str : strs)
		//{
		//	if (src_str.size() == 1 && src_str.at(0) == *symbol)
		//	{

		//	}
		//	else
		//		no_str_bracked.emplace_back(src_str);
		//}
		//return no_str_bracked;
		return strs;
	}
}
