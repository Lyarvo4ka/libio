#ifndef CALCRAID5PARITY_H
#define CALCRAID5PARITY_H

#include "iofunctions.h"
#include "constants.h"

void calcRaid5Parity(ULONGLONG offset_lba, int src_1, int src_2, int target, DWORD block_size)
{
	if (block_size <= 0)
	{
		printf("Error block_size must more than 0");
		return;
	}
	HANDLE hSource1 = INVALID_HANDLE_VALUE;
	HANDLE hSource2 = INVALID_HANDLE_VALUE;
	HANDLE hTarget = INVALID_HANDLE_VALUE;

	if (!IO::open_read(hSource1, drivePathFromNumber(src_1)))
	{
		printf("Error open source_1");
		return;
	}

	if (!IO::open_read(hSource2, drivePathFromNumber(src_2)))
	{
		printf("Error open source_2");
		return;
	}

	if (!IO::open_write(hTarget, drivePathFromNumber(target)))
	{
		printf("Error open target");
		return;

	}


	BYTE * buffer1 = new BYTE[block_size];
	BYTE * buffer2 = new BYTE[block_size];
	BYTE * target_buff = new BYTE[block_size];

	bool bResult = false;
	DWORD bytesRead = 0;
	DWORD byteWritten = 0;

	LONGLONG offset = offset_lba;
	offset *= SECTOR_SIZE;
	while (true)
	{
		printf("Offset: %llu (sectors)\r\n", offset / (LONGLONG)(SECTOR_SIZE) );

		IO::set_position(hSource1, offset);
		bResult = IO::read_block(hSource1, buffer1, block_size, bytesRead);
		if (!bResult )
			break;
		if (bytesRead == 0 )
			break;

		IO::set_position(hSource2, offset);
		bResult = IO::read_block(hSource2, buffer2, block_size, bytesRead);
		if (!bResult)
			break;
		if (bytesRead == 0)
			break;

		for (auto iByte = 0; iByte < block_size; ++iByte)
		{
			target_buff[iByte] = buffer1[iByte] ^ buffer2[iByte];
		}

		IO::set_position(hTarget, offset);
		bResult = IO::write_block(hTarget, target_buff, block_size, byteWritten);
		if (!bResult )
			break;
		if (byteWritten == 0)
		{
			printf("Error to write ");
			break;
		}


		offset += block_size;

	}




	delete[] buffer1;
	delete[] buffer2;
	delete[] target_buff;

	CloseHandle(hSource1);
	CloseHandle(hSource2);
	CloseHandle(hTarget);

}



#endif