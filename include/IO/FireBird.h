#ifndef FIREBIRD_H
#define FIREBIR_H

#include <windows.h>
#include <sqltypes.h>
#include "AbstractRaw.h"
#include "constants.h"

#pragma pack( 1 )
struct firebird_page
{
	SCHAR pag_type;
	UCHAR pag_flags;
	USHORT pag_checksum;
	ULONG pag_generation;
	ULONG pag_scn;
	ULONG reserved;
	bool isFireBirdPage()
	{
		if (pag_type >= 0x01 && pag_type <= 0xF)
			if (pag_checksum == 0x3039)
				if ( pag_scn == 0 )
					if ( reserved == 0)
						return true;
		return false;
	}
};
#pragma pack()

const int firebird_size = sizeof(firebird_page);
const int FB_Size = 8 * 1024;


void removeNotFireBirdPages(const std::string & src_file, const std::string & dst_file)
{
	HANDLE hSource = INVALID_HANDLE_VALUE;
	if (!IO::open_read(hSource, src_file))
	{
		printf("Error open file\r\n");
		return;
	}
	HANDLE hTarget = INVALID_HANDLE_VALUE;
	if (!IO::create_file(hTarget, dst_file))
	{
		printf("Error create file\r\n");
		return;
	}

	BYTE buffer[FB_Size];
	DWORD bytesRead = 0;
	DWORD bytesWritten = 0;
	bool bResult = false;

	while (true)
	{
		bResult = IO::read_block(hSource, buffer, FB_Size, bytesRead);
		if (!bResult || bytesRead == 0)
		{
			printf("Error read block\r\n");
			break;
		}
		firebird_page * pFireBird = (firebird_page *) buffer;
		if (pFireBird->isFireBirdPage())
		{
			bResult = IO::write_block(hSource, buffer, FB_Size, bytesWritten);
			if (!bResult || bytesWritten == 0)
			{
				printf("Error write block\r\n");
				break;
			}
		}
		
	}
	CloseHandle(hSource);
	CloseHandle(hTarget);
}


class FileWriter
{
public:
	FileWriter(const std::string & target_folder)
		:folder_(target_folder)
		, counter_(0)
		, bReady_(false)
		, bNewFile_(false)
		, hHandle_( INVALID_HANDLE_VALUE)
	{

	}

	void AddPage(BYTE * buffer, DWORD buffer_size, LONGLONG page_offset)
	{
		if (bNewFile_)
		{
			if (bReady_)
				CloseHandle(hHandle_);

			create_new_file(page_offset);

			bNewFile_ = false;
		}
		DWORD bytesWritten = 0;
		bool bResult = IO::write_block(hHandle_, buffer, buffer_size, bytesWritten);
		if (bytesWritten == 0 || !bResult )
		{
			printf("Error write to file");
		}

	}

	void create_new_file(LONGLONG page_offset_name)
	{
		
		std::string file_name = IO::file_offset_name(folder_, page_offset_name, ".fdb");
		if (!IO::create_file(hHandle_, file_name))
		{
			printf("Error create file: %s", file_name.c_str());
			exit(-3);
		}
		bReady_ = true;
	}

	void setNewFile()
	{
		bNewFile_ = true;
	}

private:
	bool bNewFile_;
	bool bReady_;
	std::string folder_;
	int counter_;
	HANDLE hHandle_;

};

class FireBird_Raw
	: public AbstractRaw
{
public:
	FireBird_Raw(const DWORD drive_number , const std::string & target_folder)
		: AbstractRaw(drive_number)
		, folder_(target_folder)
		, fileWriter_(nullptr)

	{
		fileWriter_ = new FileWriter(target_folder);
	}
	FireBird_Raw(const std::string & file_path, const std::string & target_folder)
		: AbstractRaw(file_path)
		, folder_(target_folder)
		, fileWriter_(nullptr)

	{
		fileWriter_ = new FileWriter(target_folder);
	}
	~FireBird_Raw()
	{
		delete fileWriter_;
	}
	LONGLONG find_firebird(LONGLONG start_offset)
	{
		DWORD bytes_read = 0;
		
		BYTE buffer[BLOCK_SIZE];
		firebird_page * pFireBird = nullptr;

		while (true)
		{
			IO::set_position(*this->getHandle(), start_offset);
			if (!IO::read_block(*this->getHandle(), buffer, BLOCK_SIZE, bytes_read))
				return ERROR_RESULT;
			if (bytes_read == 0)
				return ERROR_RESULT ;

			for (DWORD iPage = 0; iPage < BLOCK_SIZE; iPage += SECTOR_SIZE)
			{
				pFireBird = (firebird_page *)&(buffer[iPage]);
				if (pFireBird->isFireBirdPage())
				{
					LONGLONG file_offset = start_offset;
					file_offset += iPage;
					return file_offset;
				}
			}

			start_offset += bytes_read;
		}

	}
	LONGLONG save_firebird_data(LONGLONG start_offset)
	{
		this->fileWriter_->setNewFile();
		DWORD bytes_read = 0;

		BYTE buffer[BLOCK_SIZE];
		firebird_page * pFireBird = nullptr;

		LONGLONG cluster_offset = 0;
		LONGLONG offset = start_offset;

		while (true)
		{
			IO::set_position(*this->getHandle(), offset);
			if (!IO::read_block(*this->getHandle(), buffer, BLOCK_SIZE, bytes_read))
				return ERROR_RESULT;
			if (bytes_read == 0)
				return ERROR_RESULT;

			for (DWORD iPage = 0; iPage < BLOCK_SIZE; iPage += FB_Size)
			{
				pFireBird = (firebird_page *)&(buffer[iPage]);
				if (pFireBird->isFireBirdPage())
				{
					cluster_offset = offset;
					cluster_offset += iPage;
					fileWriter_->AddPage(buffer + iPage, FB_Size, cluster_offset);
				}
				else
				{
					cluster_offset = offset;
					cluster_offset += iPage;
					return cluster_offset;
				}
			}

			offset += bytes_read;
		}
		return ERROR_RESULT;
	}
	void execute() override
	{
		
		DWORD bytesRead = 0;
		HANDLE *hSource = this->getHandle();

		bool bResult = false;
		

		LONGLONG offset = 0x748141e00;
		LONGLONG header_offset = 0;
		if (*hSource == INVALID_HANDLE_VALUE)
		{
			printf("Error open drive\r\n");
			return ;
		}

		while (true)
		{
			header_offset = this->find_firebird(offset);
			if ( header_offset == ERROR_RESULT )
				break;

			offset = this->save_firebird_data(header_offset);
			if ( header_offset == ERROR_RESULT)
				break;
			offset += SECTOR_SIZE;
		}


	}
private:
	std::string folder_;
	FileWriter *fileWriter_;
};


//enum page_type { UNDEFINE , };

#endif