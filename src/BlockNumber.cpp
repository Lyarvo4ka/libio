#include "BlockNumber.h"

DWORD MaskMarker(const DWORD maker, const DWORD mask)
{
	DWORD new_marker = maker;
	to_big_endian(new_marker);
	new_marker >>= 8;
	new_marker &= mask;
	return new_marker;
}


BlockNumber::BlockNumber()
	: hDump_(INVALID_HANDLE_VALUE)
	, hImage_(INVALID_HANDLE_VALUE)

{

}

bool BlockNumber::OpenDump(const std::string & dump_file)
{
	return IO::open_read(hDump_, dump_file);
}
bool BlockNumber::CreateImage(const std::string & image_file)
{
	return IO::create_file(hImage_, image_file);
}


void BlockNumber::addMarker(DWORD marker, LONGLONG pos)
{
	auto findIter = bankTable_.find(marker);
	if (findIter == bankTable_.end())
	{
		ArrayPos array_pos;
		array_pos.push_back(pos);
		bankTable_.insert(BankTablePair(marker, array_pos));
	}
	else
		findIter->second.push_back(pos);
}

bool BlockNumber::ReadBank(LONGLONG start_offset, LONGLONG bank_size)
{
	if (hDump_ == INVALID_HANDLE_VALUE)
		return false;

	bankTable_.clear();

	LONGLONG pos = start_offset;

	BYTE block[PAGE_SIZE];
	DWORD bytesRead = 0;
	bool bResult = false;

	DWORD *pMarker = nullptr;
	DWORD marker = 0;

	bool bBlockData = false;

	while (pos <  bank_size)
	{
		IO::set_position(hDump_, pos);
		bResult = IO::read_block(hDump_, block, PAGE_SIZE, bytesRead);

		if (!bResult || (bytesRead == 0))
			break;

		bBlockData = false;
		for (DWORD iByte = 0; iByte < PAGE_SIZE; ++iByte)
		{
			if (block[iByte] != 0)
			{
				bBlockData = true;
				break;
			}
		}

		if (bBlockData)
		{
			pMarker = (DWORD *)&block[512];

			marker = MaskMarker(*pMarker, 0x0003FFFF);
			addMarker(marker, pos);
		}
		pos += PAGE_SIZE;
	}
	return false;
}

void BlockNumber::WriteBank(DWORD bank_number)
{
	if (hImage_ == INVALID_HANDLE_VALUE)
		return ;

	auto mapIter = bankTable_.begin();

	BYTE dump_block[PAGE_SIZE];
	BYTE image_block[PAGE_IMAGE_SIZE];

	LONGLONG source_offset = 0;
	LONGLONG target_offset = 0;

	DWORD bytesRead = 0;
	DWORD byteWritten = 0;
	bool bResult = false;

	LONGLONG bank_offset = 0;
	for (int iBank = 0; iBank < bank_number; ++iBank)
	{
		bank_offset += BankSizeTable[iBank];
	}

	if ( bank_number > 0 )
		bank_offset = bank_offset*PAGE_IMAGE_SIZE;

	while (mapIter != bankTable_.end())
	{
		if (mapIter->first < BankSizeTable[bank_number])
		{
			source_offset = *mapIter->second.rbegin();
			IO::set_position(hDump_, source_offset);
			bResult = IO::read_block(hDump_, dump_block, PAGE_SIZE, bytesRead);
			if (!bResult || (bytesRead == 0))
				break;

			for (DWORD iSector = 0; iSector < SECTORS_PER_PAGE; ++iSector)
				memcpy(image_block + SECTOR_SIZE * iSector, dump_block + SECTOR_SERVICE*iSector , SECTOR_SIZE);

			target_offset = mapIter->first*PAGE_IMAGE_SIZE ;
			target_offset += bank_offset;
			IO::set_position(hImage_, target_offset);
			bResult = IO::write_block(hImage_, image_block, PAGE_IMAGE_SIZE, byteWritten);
			if (!bResult || (byteWritten == 0))
				break;
		}
		++mapIter;
	}


}