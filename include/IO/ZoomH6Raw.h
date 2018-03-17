#pragma once
#include "IO\AbstractRaw.h"

namespace IO
{
/*
	1. Читаем кластер это должен быть "ZOOM H6 prjectfile (*.hprj)".
	2. 
*/
	class ZoomH6Raw
		: public SpecialAlgorithm
	{
		uint64_t Execute(const uint64_t start_offset, const path_string target_folder) override
		{
		 }
	};

}