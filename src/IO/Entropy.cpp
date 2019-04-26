#include "IO\Entropy.h"
#include "IO\Finder.h"
#include "IO\IODevice.h"



namespace IO
{
	double calcEntropy(BYTE * data, DWORD size)
	{
		if (!data || (size == 0))
			return 0.0;

		DWORD symbols[MAX_BYTE] = { 0 };
		memset(symbols, 0, MAX_BYTE * sizeof(DWORD));
		for (DWORD Index = 0; Index < size; ++Index)
		{
			symbols[data[Index]]++;
		}
		double temp = 0.0;

		double entropy = 0.0;
		for (size_t iByte = 0; iByte < MAX_BYTE; ++iByte)
		{
			if (symbols[iByte] > 0)
			{
				temp = ((double)symbols[iByte]) / ((double)size);
				entropy -= temp * log(temp) / Log2;
			}
		}
		return entropy;
	}

	bool calcEntropyForFile(const std::string & file_name, DWORD block_size)
	{
		HANDLE hRead = INVALID_HANDLE_VALUE;
		if (!IO::open_read(hRead, file_name))
			return false;

		std::string entropy_file = file_name + ".txt";
		HANDLE hWrite = INVALID_HANDLE_VALUE;
		if (!IO::create_file(hWrite, entropy_file))
			return false;

		BYTE * pBuffer = new BYTE[block_size];

		DWORD bytesRead = 0;
		DWORD byteWritten = 0;
		double entropy = 0.0;
		bool bResult = false;
		DWORD cluster_number = 0;

		while (true)
		{
			bResult = IO::read_block(hRead, pBuffer, block_size, bytesRead);
			if (!bResult || (bytesRead == 0))
				break;

			entropy = calcEntropy(pBuffer, bytesRead);
			std::string write_string(boost::lexical_cast<std::string>(entropy));
			write_string.append("\r\n");

			IO::write_block(hWrite, (BYTE*)write_string.data(),(DWORD) write_string.size(), byteWritten);
			++cluster_number;
		}

		delete[] pBuffer;

		CloseHandle(hWrite);
		CloseHandle(hRead);
		return true;
	}
	bool calcEntropyForFile(const path_string & file_name, DWORD block_size)
	{
		IO::File src_file(file_name);
		if (!src_file.Open(IO::OpenMode::OpenRead))
			return false;

		IO::path_string entropy_name = file_name + L".txt";


		IO::File entrory_file(entropy_name);
		if (!entrory_file.Open(IO::OpenMode::Create))
			return false;

		IO::DataArray data_array(block_size);

		uint32_t bytesRead = 0;
		DWORD byteWritten = 0;
		double entropy = 0.0;
		bool bResult = false;
		DWORD cluster_number = 0;

		while (true)
		{
			try {
				bResult = true;
				bytesRead = src_file.ReadData(data_array);
			}
			catch (Error::IOErrorException ex)
			{
				std::cout << "Cougth exception" << ex.what() << std::endl;
				bResult = false;
			}
			if (!bResult)
				break;
			entropy = calcEntropy(data_array.data(), bytesRead);
			std::string write_string(boost::lexical_cast<std::string>(entropy));
			write_string.append("\r\n");

			entrory_file.WriteData((IO::ByteArray)write_string.data(), (DWORD)write_string.size());
			++cluster_number;
		}
		return true;
	}
	bool calcNullsForFile(const path_string & file_name, DWORD block_size)
	{
		IO::File src_file(file_name);
		if (!src_file.Open(IO::OpenMode::OpenRead))
			return false;

		IO::path_string entropy_name = file_name + L".nulls";


		IO::File entrory_file(entropy_name);
		if (!entrory_file.Open(IO::OpenMode::Create))
			return false;

		IO::DataArray data_array(block_size);

		uint32_t bytesRead = 0;
		DWORD byteWritten = 0;
		uint32_t number_nulls = 0;
		bool bResult = false;
		DWORD cluster_number = 0;



		while (true)
		{
			try {
				bResult = true;
				bytesRead = src_file.ReadData(data_array);
			}
			catch (Error::IOErrorException ex)
			{
				std::cout << "Cougth exception" << ex.what() << std::endl;
				bResult = false;
			}
			if (!bResult)
				break;

				number_nulls = 0;
				for (uint32_t i = 0; i < bytesRead; ++i)
				{
					if (data_array.data()[i] == 0)
						++number_nulls;
				}
				std::string write_string(boost::lexical_cast<std::string>(number_nulls));
				write_string.append("\r\n");

				entrory_file.WriteData((IO::ByteArray)write_string.data(), (DWORD)write_string.size());
				++cluster_number;
			
		}
		return true;
	}

	void calcEntropyForFolder(const std::string & folder, DWORD block_size)
	{

	}

	void calcEntropyForFolder(const path_string & folder, DWORD block_size)
	{
		Finder finder;
		finder.FindFiles(folder);
		auto files = finder.getFiles();
		for (auto theFile : files)
		{
			calcEntropyForFile(theFile, block_size);
		}
	}
	void calcNullsForFolder(const path_string & folder, DWORD block_size)
	{
		Finder finder;
		finder.FindFiles(folder);
		auto files = finder.getFiles();
		for (auto theFile : files)
		{
			calcNullsForFile(theFile, block_size);
		}
	}
	void removeLRV(const std::string & input_file, const std::string & output_file, DWORD cluster_size)
	{
		HANDLE hRead = INVALID_HANDLE_VALUE;
		if (!IO::open_read(hRead, input_file))
			return;

		HANDLE hWrite = INVALID_HANDLE_VALUE;
		if (!IO::create_file(hWrite, output_file))
			return;

		DWORD bytesRead = 0;
		DWORD bytesWritten = 0;
		bool bResult = false;
		double entropy = 0.0;

		//IO::set_position(hRead, 0x4380000);

		BYTE * pBuffer = new BYTE[cluster_size];

		while (true)
		{
			bResult = IO::read_block(hRead, pBuffer, cluster_size, bytesRead);
			if (!bResult || bytesRead == 0)
				break;

			if (memcmp(pBuffer, Signatures::qt_header4, SIZEOF_ARRAY(Signatures::qt_header4)) == 0)
			{
				bResult = IO::write_block(hWrite, pBuffer, bytesRead, bytesWritten);
				if (!bResult || bytesWritten == 0)
					break;
			}
			else
			{
				entropy = IO::calcEntropy(pBuffer, bytesRead);
				if (entropy > 7.9990 && entropy < 7.9999)
				{
					bResult = IO::write_block(hWrite, pBuffer, bytesRead, bytesWritten);
					if (!bResult || bytesWritten == 0)
						break;
				}
			}
		}

		delete[] pBuffer;

		CloseHandle(hRead);
		CloseHandle(hWrite);
	}

}