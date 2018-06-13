#ifndef ENTROPY_H
#define ENTROPY_H

#include "iolibrary_global.h"


#include <stdio.h>      /* printf */
#include <math.h>   
#include <Windows.h>
#include "iofunctions.h"
#include "constants.h"

#define MAX_BYTE 256
#define MAX_BYTE2 UCHAR_MAX + 1

#include <boost/lexical_cast.hpp>

namespace IO
{
	static double Log2 = log(2.0);

	double calcEntropy( BYTE * data, DWORD size );
	bool calcEntropyForFile(const std::string & file_name, DWORD block_size);
	bool calcNullsForFile(const std::string & file_name, DWORD block_size);
	bool calcEntropyForFile(const path_string & file_name, DWORD block_size);
	void calcEntropyForFolder(const std::string & folder, DWORD block_size);// not implemented
	void calcEntropyForFolder(const path_string & folder, DWORD block_size);
	void calcNullsForFolder(const path_string & folder, DWORD block_size);
	void removeLRV(const std::string & input_file, const std::string & output_file, DWORD cluster_size);
}



#endif