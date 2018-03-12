#ifndef BLOCK_NUMBER_H
#define BLOCK_NUMBER_H

#include "constants.h"
#include "iofunctions.h"

const DWORD BankSizeTable[] = { 0x3d100, 0x3d400, 0x3d500, 0x3d500, 0x3d600, 0x3d600, 0x3d300, 0x3d300 };

#include <vector>
#include <map>


typedef std::vector<LONGLONG> ArrayPos;
typedef std::map<DWORD, ArrayPos> BankTable;
typedef std::pair<DWORD, ArrayPos> BankTablePair;
typedef std::vector<BankTable> BankTableArray;

DWORD  MaskMarker(const DWORD maker, const DWORD mask);


const DWORD PAGE_SIZE = 4224;
const DWORD PAGE_IMAGE_SIZE = 512*8;
//const DWORD SECTOR_SIZE = 512;
const DWORD SERVICE_SIZE = 16;
const DWORD SECTOR_SERVICE = SECTOR_SIZE + SERVICE_SIZE;
const DWORD SECTORS_PER_PAGE = PAGE_SIZE / SECTOR_SERVICE;

class  BlockNumber
{

public:
	BlockNumber( );

	bool OpenDump(const std::string & dump_file);
	bool CreateImage(const std::string & image_file);

	void addMarker(DWORD marker, LONGLONG pos);
	bool ReadBank(LONGLONG start_offset, LONGLONG bank_size);
	void WriteBank(DWORD bank_number);


	//bool writeImage(const std::string & image_file);

private:
	HANDLE hDump_;
	HANDLE hImage_;

	BankTable bankTable_;
};


#endif