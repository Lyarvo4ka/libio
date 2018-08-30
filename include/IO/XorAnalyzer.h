#pragma once

#include "iolibrary_global.h"
#include <windows.h>

#include <vector>
//using std::vector;

#pragma warning(disable:4251)
namespace IO
{
	ULONGLONG NumBytesForBlock(DWORD block_size);
	int chunksPrerBlock(ULONGLONG block_size);

	class ByteCount
	{
	public:
		ByteCount(void);
		~ByteCount(void);
		void add(BYTE byte_value);
		BYTE getMax();
	private:
		WORD * bytes_;
	};


	class XorAnalyzer
	{
	private:
		std::string dump_file_;
		HANDLE hDump_ = INVALID_HANDLE_VALUE;
	public:
		XorAnalyzer()
		{}
		XorAnalyzer(const std::string & dump_file);
		~XorAnalyzer(void);
		bool open();
		void close();

		void Analize(const std::string & result_xor, DWORD xor_size);

		DWORD getChunckBufferSize(DWORD chunck_size, int nChunck, DWORD xor_size);

	};
}
inline void xor_secror(BYTE *source_data, BYTE * target_data, BYTE *sector, DWORD sector_size)
{
	for (DWORD iSector = 0; iSector < sector_size; ++iSector)
	{
		target_data[iSector] = source_data[iSector] ^ sector[iSector];
		//target_data[iSector] = ~target_data[iSector];
	}
}

inline void XorBlockWithSector(BYTE * source_data, BYTE * target_data, BYTE *sector_data, DWORD block_size, DWORD sector_size)
{
	for (DWORD sector_offset = 0; sector_offset < block_size; sector_offset += sector_size)
		xor_secror(source_data + sector_offset, target_data + sector_offset, sector_data, sector_size);
}


#include "IO\constants.h"
#include "IO\AbstractRaw.h"
inline void XorHardDrive(DWORD drive_number, const std::string & target_file, const std::string xor_file_sector)
{
	std::string drive_path = drivePathFromNumber(drive_number);
	HANDLE hDrive = INVALID_HANDLE_VALUE;

	if (!IO::open_read(hDrive, drive_path))
	{
		printf("Error open drive");
		return;
	}

	HANDLE hTarget = INVALID_HANDLE_VALUE;
	if (!IO::open_write(hTarget, target_file))
	{
		printf("Error to create file.");
		return;
	}

	HANDLE hXorSector = INVALID_HANDLE_VALUE;

	if (!IO::open_read(hXorSector, xor_file_sector))
	{
		printf("Error open xor file.");
		return;
	}


	const DWORD block_size = 256 * SECTOR_SIZE;
	BYTE read_buffer[block_size];
	BYTE write_buffer[block_size];
	BYTE sector_buffer[SECTOR_SIZE];


	DWORD bytesRead = 0;
	DWORD bytesWritten = 0;

	if (!IO::read_block(hXorSector, sector_buffer, SECTOR_SIZE, bytesRead))
	{
		printf("Error read xor file");
		return;
	}
	if (bytesRead != SECTOR_SIZE)
		return;

	bytesRead = 0;


	//IO::set_position(hDrive, 0x75B56BE00);

	while (true)
	{
		if (!IO::read_block(hDrive, read_buffer, block_size, bytesRead))
		{
			printf("Error read from disk");
			break;
		}
		if ( bytesRead == 0)
			break;

		XorBlockWithSector(read_buffer, write_buffer, sector_buffer, block_size, SECTOR_SIZE);
		if (!IO::write_block(hTarget , write_buffer , block_size , bytesWritten ))
		{
			printf("Error write to file.");
			break;
		}
		if (bytesWritten == 0)
			break;
	}

	CloseHandle(hDrive);
	CloseHandle(hTarget);


}