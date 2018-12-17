#pragma once

#include "IO/constants.h"

namespace IO
{
	class Analyzer
	{
	public:
		virtual void analyze() = 0;

		bool isCorupted() const;
		bool hasName() const;
		bool hasTime() const;
		path_string getExtension() const;
	};


}