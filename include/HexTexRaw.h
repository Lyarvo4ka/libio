#ifndef HEXTEXTRAW_H
#define HEXTEXTRAW_H

#include "AbstractRaw.h"
#include "constants.h"

bool isBetweenValues(BYTE start, BYTE val, BYTE end)
{
	return (val >= start && val <= end);
}

bool isSameValues(BYTE * buffer, DWORD size)
{
	BYTE value = buffer[0];
	for (DWORD iByte = 1; iByte < size; ++iByte)
	{
		if (value != buffer[iByte])
			return false;
	}
	return true;
}

class HexTextRaw
	: public AbstractRaw
{
	static const int def_file_size = 168;
public:
	HexTextRaw(const std::string & source_file, const std::string & output_folder)
		: AbstractRaw(source_file)
		, folder_(output_folder)
	{

	}
	bool isHexValues(BYTE * buffer, DWORD size)
	{
		for (DWORD iByte = 0; iByte < size; ++iByte)
		{
			auto val = buffer[iByte];
			if (!isBetweenValues(0x30, val, 0x39))
				if ( !isBetweenValues(0x41, val, 0x46))
					return false;

		}
		if (isSameValues(buffer, size))
			return false;
		return true;
	}
	void execute() override
	{
		if (!this->isReady())
		{
			printf("Device isn't ready.");
			return;
		}
		auto hSource = this->getHandle();
		BYTE buffer[BLOCK_SIZE];
		DWORD bytesRead = 0;
		bool bResult = false;

		HANDLE hTarget = INVALID_HANDLE_VALUE;
		DWORD bytesWritten = 0;
		LONGLONG offset = 0;
		while (true)
		{
			IO::set_position(*hSource, offset);
			bResult = IO::read_block(*hSource, buffer, BLOCK_SIZE, bytesRead);
			if (bytesRead == 0 || !bResult)
				break;


			for (DWORD iSector = 0; iSector < bytesRead; iSector += SECTOR_SIZE)
			{
				BYTE * pSector = buffer + iSector;
				if (this->isHexValues(pSector, def_file_size))
				{
					LONGLONG temp_offset = offset;
					temp_offset += iSector;
					std::string file_path = IO::file_offset_name(folder_, temp_offset, ".dat");
					if (IO::create_file(hTarget, file_path))
					{
						bResult = IO::write_block(hTarget, pSector, def_file_size, bytesWritten);
						if (bytesWritten == 0 || !bResult)
							break;
						printf("Write to file %s OK.\r\n", file_path.c_str());
						CloseHandle(hTarget);

					}
					else
						printf("Error to create file. %s.\r\n", file_path.c_str());
				}
			}
			offset += bytesRead;
		}



	}
private:
	std::string folder_;
};


#endif