#pragma once

#include "IO/constants.h"

namespace IO
{
	template <typename T>
	struct DateType
	{
		T year;
		T month;
		T day;
		T hour;
		T min;
		T sec;
	};

	class Analyzer
	{
	public:
		virtual void analyze(const path_string & filePath) = 0;

		bool isCorupted() const;
		bool hasName() const;
		bool hasTime() const;
		path_string getExtension() const;
	};



}