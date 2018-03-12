// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "IO\XorAnalyzer.h"
#include <boost/lexical_cast.hpp>


int main(int argc, CHAR **argv)
{
	if (argc == 4)
	{
		std::string dump_file = argv[1];
		std::string xor_file = argv[2];
		auto xor_size = boost::lexical_cast<uint32_t>(argv[3]);

		IO::XorAnalyzer xorAnalyzer(dump_file);
		xorAnalyzer.Analize(xor_file, xor_size);
	}
	else
	{
		printf("Wrong parametrs.\r\n");
		printf("XorAnalyzer.exe dump.bin xor.bin xor_size\r\n");
	}
    return 0;
}

