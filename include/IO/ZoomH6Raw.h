#pragma once
#include "IO\AbstractRaw.h"
#include "IO\utility.h"
#include "IO\RawRIFF.h"

//#include <boost\filesystem.hpp>
#include <filesystem>

namespace IO
{
/*
	1. Читаем кластер это должен быть "ZOOM H6 prjectfile (*.hprj)".
	2. Потом читаем по 1 кластеру и определяем сколько у нас файлов 2 или 4.
	3.
*/
	namespace fs = std::experimental::filesystem;//boost::filesystem;
	class RiffFile
	{
		FilePtr file_;
		bool bReady_ = false;
		uint32_t size_;
		uint32_t currentSize_ = 0;
	public:
		using Ptr = std::shared_ptr<RiffFile>;
		RiffFile(FilePtr file, const uint32_t file_size)
			:file_(file)
			, size_(file_size + riff_header_struct_size)
		{

		}
		bool isReady() const
		{
			return bReady_;
		}

		uint32_t WriteBlock(const DataArray & data_array)
		{
			uint32_t bytesToWrite = data_array.size();
			if (currentSize_ + data_array.size() > size_)
			{
				bytesToWrite = size_ - currentSize_;
			}
			auto bytesWritten = file_->WriteData(data_array.data(), bytesToWrite);
			if (bytesWritten == 0)
				return 0;
			currentSize_ += bytesWritten;
			if (currentSize_ == size_)
				bReady_ = true;

		}
	};
	RiffFile::Ptr makeRiffFile(FilePtr filePtr, const uint32_t file_size)
	{
		return std::make_shared<RiffFile>(filePtr, file_size);
	}


	class ZoomFiles
	{
		std::vector<RiffFile::Ptr> files_;
	public:
		//ZoomFiles()
		//{
		//}
		void add_file(FilePtr file, const uint32_t file_size)
		{
			files_.emplace_back(makeRiffFile(file, file_size));
		}

		bool CreateFiles(const path_string & target_folder, const path_string & file_name)
		{
			// Create folder "file_name"
			auto newFolderName = addBackSlash(target_folder) + file_name;
			if (!fs::create_directory(newFolderName))
			{
				LOG_MESSAGE("Error to create directory.");
				return false;
			}
			
		}
		uint32_t appendFile(const DataArray & data_array, const uint32_t fileNumber)
		{
			if (fileNumber < files_.size())
				return files_.at(fileNumber)->WriteBlock(data_array);
			return 0;
		}
		bool isAllReady()  
		{
			uint32_t nCount = 0;
			for (auto & theFile : files_)
				if (theFile->isReady())
					++nCount;

			if (nCount == files_.size())
				return true;
			else return false;
		}
		std::size_t count() const
		{
			return files_.size();
		}
	};

	const char zoom_h6[] = { 0x5A , 0x4F , 0x4F , 0x4D , 0x20 , 0x48 , 0x36 , 0x20 };
	const uint32_t zoom_h6_size = SIZEOF_ARRAY(zoom_h6);

	const char riff_header[] = { 0x52 , 0x49 , 0x46 , 0x46 };
	const uint32_t riff_header_size = SIZEOF_ARRAY(riff_header);

	struct ZoomH6Header
	{
		char header_text[22];
		char folder_name[11];
	};

	class ZoomH6Raw
		: public SpecialAlgorithm
	{
	private:
		IODevicePtr device_;
		uint32_t block_size_ = default_block_size;
	public:
		ZoomH6Raw(IODevicePtr device)
			: device_(device)
		{

		}
		void setBlockSize(const uint32_t block_size)
		{
			block_size_ = block_size;
		}
		uint32_t block_size() const
		{
			return block_size_;
		}
		uint32_t ReadBlock(DataArray & data_array , const uint64_t offset)
		{
			device_->setPosition(offset);
			return device_->ReadData(data_array.data(), data_array.size());
		}
		uint64_t Execute(const uint64_t start_offset, const path_string target_folder) override
		{
			ZoomFiles zoomFiles;
			const uint32_t MAX_FILES = 4;
			setBlockSize(32768);
			if (!device_->isOpen())
			{
				LOG_MESSAGE("Error device isn't opened.");
				return 0;
			}
			uint64_t position = start_offset;
			IO::DataArray data_array(this->block_size());

			// must *.hprj
			auto bytesRead = ReadBlock(data_array, position);
			if (memcmp(data_array.data(), zoom_h6, zoom_h6_size) != 0)
			{
				LOG_MESSAGE("It's not zoom h6 header.");
				return 0;
			}
			ZoomH6Header *pZoomHeader = (ZoomH6Header*)data_array.data();
			std::string folder_name(pZoomHeader->folder_name);
			path_string wstr(folder_name.begin(), folder_name.end());
			auto newFolderName = addBackSlash(target_folder) + wstr;
			if (!fs::create_directory(newFolderName))
				return 0;
			path_string folderName = newFolderName;
			path_string fileName = wstr;

			position += this->block_size();
			uint32_t fileCount = 0;
			
			uint64_t bytesWritten = 0;

			for (uint32_t fileNumber = 0; fileNumber < MAX_FILES; ++fileNumber)
			{
				bytesRead = ReadBlock(data_array, position);
				if (bytesRead == 0)
					break;
				riff_header_struct * pRiffHeader = (riff_header_struct *)data_array.data();
				if (memcmp(pRiffHeader->riff_name, riff_header, riff_header_size) != 0)
					break;
				auto number_name = std::to_wstring(fileNumber);
				auto file_ptr = makeFilePtr(addBackSlash(folderName) + fileName + L"_" + number_name + L".wav");
				if (!file_ptr->Open(OpenMode::Create))
					return false;
				

				zoomFiles.add_file(file_ptr, pRiffHeader->size);
				bytesWritten += zoomFiles.appendFile(data_array, fileNumber);

				position += this->block_size();
			}


			uint32_t fileCounter = 0;
			while (true)
			{
				if (zoomFiles.isAllReady())
					break;

				if (fileCounter == 0)
				{
					for (auto i = 0; i < 2; ++i)
					{
						bytesRead = ReadBlock(data_array, position);
						if (bytesRead == 0)
							break;
						bytesWritten += zoomFiles.appendFile(data_array, fileCounter);
						position += this->block_size();
					}
				}
				else
				{
					bytesRead = ReadBlock(data_array, position);
					if (bytesRead == 0)
						break;
					bytesWritten += zoomFiles.appendFile(data_array, fileCounter);
					position += this->block_size();
				}
				++fileCounter;
				if (fileCounter >= zoomFiles.count())
					fileCounter = 0;

			}
			return bytesWritten;

		 }
	};

}