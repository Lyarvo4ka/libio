#ifndef FINDER_H
#define FINDER_H

#include "iofs.h"
#include <iostream>

#include <locale>
#include <codecvt>
#include <boost/algorithm/string.hpp>
#include <algorithm>

#include "IODevice.h"
#include "QuickTime.h"
#include "dbf.h"

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
//#include "tinyxml2.h"

namespace IO
{
	inline IO::path_string  getFileNameFromPath(const IO::path_string  & fullFileName)
	{
		fs::path filePath(fullFileName);
		return filePath.filename().generic_wstring();
	}

	inline int NotNullPosFromEnd(const ByteArray data, const uint32_t size)
	{
		int pos = size - 1;
		while (pos >= 0)
		{
			if (data[pos] != 0)
				return (size - pos);
			--pos;
		}
		return 0;
	}

	inline void removeNullsFromEndFile(const path_string & file_path, uint32_t sizeToTest = default_block_size)
	{
		File file(file_path);
		if (!file.Open(OpenMode::OpenWrite))
		{
			wprintf_s(L"Error open file.\n");
			return;
		}
		//sizeToTest = default_block_size;
		auto file_size = file.Size();
		DataArray buffer(sizeToTest);
	//	if (file_size >= sizeToTest)
		{
			uint32_t lastBlock = sizeToTest;
			if (file_size < sizeToTest)
				lastBlock = static_cast<uint32_t>(file_size);

			uint64_t offset = file_size - lastBlock;
			file.setPosition(offset);
			auto bytesRead = file.ReadData(buffer.data(), lastBlock);
			if (bytesRead == lastBlock)
			{
				int not_null = NotNullPosFromEnd(buffer.data() , lastBlock);
				if (not_null > 0)
				{
					uint64_t new_size = file_size - not_null + 1;
					file.setSize(new_size);
					wprintf_s(L"FIle size has been changed %s.\n", file_path.c_str());
				}
			}
		}
	
	}

	inline void removeNullsAlignedToSector(const path_string & file_path, uint32_t sizeToTest = default_block_size)
	{
		File file(file_path);
		if (!file.Open(OpenMode::OpenWrite))
		{
			wprintf_s(L"Error open file.\n");
			return;
		}
		//sizeToTest = default_block_size;
		auto file_size = file.Size();

		DataArray buffer(sizeToTest);

		uint32_t lastBlock = sizeToTest;
		if (file_size < sizeToTest)
			return;

		file.setPosition(file_size - lastBlock);
		auto bytesRead = file.ReadData(buffer.data(), lastBlock);
		if (bytesRead == lastBlock)
		{
			int not_null = NotNullPosFromEnd(buffer.data(), lastBlock);
			uint64_t new_size = file_size - not_null;
			new_size /= default_sector_size;
			new_size++;
			new_size *= default_sector_size;

			file.setSize(new_size);
			wprintf_s(L"FIle size has been changed %s.\n", file_path.c_str());
		}


	}

	inline void removeNullsFromEndZipFile(const path_string & file_path, uint32_t sizeToTest = default_block_size)
	{
		File file(file_path);
		if (!file.Open(OpenMode::OpenWrite))
		{
			wprintf_s(L"Error open file.\n");
			return;
		}
		//sizeToTest = default_block_size;
		auto file_size = file.Size();

		DataArray buffer(sizeToTest);

		uint32_t lastBlock = sizeToTest;
		if (file_size < sizeToTest)
			return;



		const uint8_t end_zip_signature[] = { 0x50, 0x4B , 0x05 , 0x06 };
		const uint32_t sizeEndSign = 4;

		file.setPosition(file_size - lastBlock);
		auto bytesRead = file.ReadData(buffer.data(), lastBlock);
		if (bytesRead == lastBlock)
		{
			for (int i = bytesRead - sizeEndSign; i >= 0 ; --i)
			{
				if (memcmp(buffer.data() + i, end_zip_signature, sizeEndSign) == 0)
				{
					uint32_t save_size = sizeToTest - i ;
					uint64_t new_size = file_size - save_size + 22;
					file.setSize(new_size);
					wprintf_s(L"FIle size has been changed %s.\n", file_path.c_str());
				}
			}
		}

	}

	class Finder
	{
	private:
		DirectoryNode::Ptr rootFolder_;
		path_list list_ext_;
		path_list files_;

	public:
		void add_extension(path_string ext)
		{
			list_ext_.push_back(ext);
		}
		Finder()
		{

		}
		void FindFiles(const path_string & folder)
		{
			this->rootFolder_ = DirectoryNode::CreateDirectoryNode(folder);
			Find(rootFolder_);
		}
		void FindFiles(const path_string & folder, const path_list & list_extensions)
		{
			this->list_ext_ = list_extensions;
			FindFiles(folder);
		}
		DirectoryNode::Ptr getRoot()
		{
			return rootFolder_;
		}
		path_list getFiles() const
		{
			return files_;
		}
		void printFiles(DirectoryNode::Ptr current_folder)
		{

			if (auto file = current_folder->getFirstFile())
			{
				auto folder_path = current_folder->getFullPath();
				do
				{
					wprintf_s(L"%s\n", file->getFullPath().c_str());
					auto full_path = file->getFullPath();

					files_.push_back(full_path);
					file = current_folder->getNextFile();
				} while (file != nullptr);
			}
			if (auto folder = current_folder->getFirstFolder())
			{
				do
				{
					printFiles(folder);
					folder = current_folder->getNextFolder();
				} while (folder != nullptr);
			}

		}
		void printAll()
		{
			if (!rootFolder_)
				return;

			wprintf_s(L"Root: %s\n", rootFolder_->getFullPath().c_str());
			printFiles(rootFolder_);

		}
	private:

		void Rename_wave(const IO::path_string & filePath)
		{
			auto test_file = IO::makeFilePtr(filePath);
			const uint32_t date_offset = 0x154;
			const uint32_t date_size = 18;
			const uint32_t check_size = 0x16A;
			char buff[date_size + 1];
			ZeroMemory(buff, date_size + 1);

			if (test_file->Open(IO::OpenMode::OpenRead))
			{
				if (check_size > test_file->Size())
					return;

				test_file->setPosition(date_offset);
				test_file->ReadData((ByteArray)buff, date_size);


				if (buff[0] == '2' && buff[1] == '0')
				{

					std::string date_string(buff);
					std::replace(date_string.begin(), date_string.end(), ' ', '-');
					std::replace(date_string.begin(), date_string.end(), '.', '-');
					std::replace(date_string.begin(), date_string.end(), ':', '-');
					date_string.insert(10, 1, '-');

					IO::path_string new_date_str(date_string.begin(), date_string.end());

					fs::path src_path(filePath);
					auto folder_path = src_path.parent_path().generic_wstring();
					auto only_name_path = src_path.stem().generic_wstring();
					auto ext = src_path.extension().generic_wstring();
					auto new_name = folder_path + L"//" + new_date_str + L"-" + only_name_path + ext;
					test_file->Close();
					try
					{
						fs::rename(filePath, new_name);
					}
					catch (const fs::filesystem_error& e)
					{
						std::cout << "Error: " << e.what() << std::endl;
					}

				}


			}
		}

		void Find(DirectoryNode::Ptr folder_node)
		{
			path_string current_folder = folder_node->getFullPath();
			path_string mask_folder(addBackSlash(current_folder) + mask_all);
			WIN32_FIND_DATA findData = { 0 };

			HANDLE hFindFile = FindFirstFile(mask_folder.c_str(), &findData);
			if (hFindFile != INVALID_HANDLE_VALUE)
			{
				do
				{
					path_string current_file(findData.cFileName);

					if (isOneDotOrTwoDots(current_file))
						continue;

					if (isDirectoryAttribute(findData))
					{
						path_string new_folder = findData.cFileName;

						auto new_folder_node = DirectoryNode::CreateDirectoryNode(new_folder);

						folder_node->AddDirectory(new_folder_node);
						Find(new_folder_node);
					}

					// Than it's must be file
					if (!isDirectoryAttribute(findData))
					{
						path_string file_name = findData.cFileName;
						fs::path tmp_path(file_name);
						path_string file_ext = tmp_path.extension().wstring();

						auto full_name = addBackSlash(current_folder) + file_name;
						

						if (list_ext_.empty())
						{
							folder_node->AddFile(file_name);
							files_.push_back(full_name);
						}
						else
						if (isPresentInList(file_ext, this->list_ext_))
						{

							//TestEndJpg(full_name);
							//zbk_rename(full_name);
							//removeNullsFromEndFile(full_name, 2048);
							//addDateToFile(full_name);
							//testSignatureMP4(full_name);
							//Rename_wave(full_name);
							files_.push_back(full_name);

							folder_node->AddFile(file_name);
						}
					}

					//SearchFiles(
				} while (FindNextFile(hFindFile, &findData));

				FindClose(hFindFile);

			}


		}



	};

	//bad sector
	const char bad_sector_sign[] = { 0x62 , 0x61 , 0x64 , 0x20 , 0x73 , 0x65 , 0x63 , 0x74 , 0x6F , 0x72 };
	const char nulls_sign[] = { 0x00 , 0x00 , 0x00 , 0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00  , 0x00 };
	const uint32_t nulls_sing_size = SIZEOF_ARRAY(nulls_sign);
	const uint32_t bad_sector_sign_size = SIZEOF_ARRAY(bad_sector_sign);

	inline void testHeaderToBadSectorKeyword(const path_string & file_name)
	{
		const uint32_t offset = 0;
		uint8_t buff[bad_sector_sign_size];
		ZeroMemory(buff, bad_sector_sign_size);

		auto test_file = makeFilePtr(file_name);

		if (test_file->Open(IO::OpenMode::OpenRead))
		{
			if (test_file->Size() >= bad_sector_sign_size)
			{
				test_file->setPosition(offset);
				test_file->ReadData(buff, bad_sector_sign_size);
				test_file->Close();

				if ( memcmp(buff , bad_sector_sign , bad_sector_sign_size) == 0)
				{
					fs::rename(file_name, file_name + L".bad_file");
				}
			}

		}

	}

	inline void testHeaderToNullsKeywords(const path_string & file_name)
	{
		const uint32_t offset = 0;
		uint8_t buff[nulls_sing_size];
		ZeroMemory(buff, nulls_sing_size);

		auto test_file = makeFilePtr(file_name);

		if (test_file->Open(IO::OpenMode::OpenRead))
		{
			if (test_file->Size() >= nulls_sing_size)
			{
				test_file->setPosition(offset);
				test_file->ReadData(buff, nulls_sing_size);
				test_file->Close();

				if (memcmp(buff, nulls_sign, nulls_sing_size) == 0)
				{
					try
					{
						fs::rename(file_name, file_name + L".bad_file");
					}
					catch (fs::filesystem_error ex)
					{
						std::cout << "Rename exception " << ex.what() << std::endl;
					}
				}
			}


		}

	}
	inline void TestEndJpg(const IO::path_string & filePath)
	{
		auto test_file = IO::makeFilePtr(filePath);
		const uint32_t constSize = 2;
		uint8_t buff[constSize];
		ZeroMemory(buff, constSize);

		//const uint8_t const_r3d_header[] = { 0x52 , 0x45 , 0x44 , 0x32 };
		const uint8_t const_end_jpg[] = { 0xFF, 0xD9 };
		//const uint8_t const_end_pdf[] = { 0x25 , 0x25 , 0x45 , 0x4F , 0x46 , 0x0A };

		if (test_file->Open(IO::OpenMode::OpenRead))
		{
			if (test_file->Size() >= constSize)
			{
				test_file->setPosition(test_file->Size() - constSize);
				test_file->ReadData(buff, constSize);
				test_file->Close();

				if (memcmp(buff, const_end_jpg, constSize) != 0)
				{

					fs::rename(filePath, filePath + L".bad_file");
				}
			}

		}
	}

	inline void Join1Cv8(const IO::path_string & src_name, const IO::path_string & old_name, const IO::path_string & result_name)
	{
		const uint32_t Data_Size = 4096;
		const uint32_t Enc_Size = 1024;

		auto src_file = makeFilePtr(src_name);
		if (!src_file->Open(OpenMode::OpenRead))
		{
			wprintf_s(L"Error %s open file to read.\n", src_file->getFileName().c_str());
			return;
		}

		auto old_file = makeFilePtr(old_name);
		if (!old_file->Open(OpenMode::OpenRead))
		{
			wprintf_s(L"Error %s open file to read.\n", old_file->getFileName().c_str());
			return;
		}

		auto result_file = makeFilePtr(result_name);
		if (!result_file->Open(OpenMode::Create))
		{
			wprintf_s(L"Error %s create file.\n", result_file->getFileName().c_str());
			return;
		}


		auto src_buff = IO::makeDataArray(Data_Size);
		auto old_buff = IO::makeDataArray(Data_Size);

		uint32_t bytes_read = 0;
		uint64_t offset = 0;
		while (true)
		{
			src_file->setPosition(offset);
			bytes_read = src_file->ReadData(src_buff->data(), src_buff->size());
			if (bytes_read == 0)
				break;

			if (offset < old_file->Size())
			{
				if (offset < 24914 * Data_Size)
				{
					old_file->setPosition(offset);
					bytes_read = old_file->ReadData(old_buff->data(), old_buff->size());
					if (bytes_read == 0)
						break;

					memcpy(src_buff->data(), old_buff->data(), Enc_Size);
				}
			}

			result_file->WriteData(src_buff->data(), src_buff->size());

			offset += Data_Size;
		}

	}

	inline bool isQtSignature(const uint8_t * qt_header, const uint32_t size)
	{
		if (memcmp(qt_header, IO::s_ftyp, size) == 0)
			return true;
		else if (memcmp(qt_header, IO::s_moov, size) == 0)
			return true;
		else if (memcmp(qt_header, IO::s_mdat, size) == 0)
			return true;
		else if (memcmp(qt_header, IO::s_wide, size) == 0)
			return true;


		return false;
	}

	inline void testSignatureMP4(const IO::path_string & filePath)
	{
		auto test_file = IO::makeFilePtr(filePath);
		//const uint8_t const_header[] = { 0x66 , 0x74 , 0x79 , 0x70/*0x47 , 0x40 , 0x00 , 0x10 , 0x00 , 0x00 , 0xB , 0x011*/ };
		const uint32_t offset = 4;
		const uint32_t header_size = 4;
		uint8_t buff[header_size];
		ZeroMemory(buff, header_size);
		if (test_file->Open(IO::OpenMode::OpenRead))
		{
			if (test_file->Size() >= header_size)
			{
				test_file->setPosition(offset);
				test_file->ReadData(buff, header_size);
				test_file->Close();

				if (isQtSignature(buff, header_size) == false)
				{
					fs::rename(filePath, filePath + L".bad_file");
				}
			}

		}


	}


};

#endif