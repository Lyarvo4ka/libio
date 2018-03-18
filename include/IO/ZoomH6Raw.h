#pragma once
#include "IO\AbstractRaw.h"
#include "IO\utility.h"
#include <filesystem>

namespace IO
{
/*
	1. Читаем кластер это должен быть "ZOOM H6 prjectfile (*.hprj)".
	2. Потом читаем по 1 кластеру и определяем сколько у нас файлов 2 или 4.
	3.
*/
	namespace fs = std::filesystem;
	struct RiffFile
	{
		FilePtr file;
		bool bReead;
		uint32_t size;
	};

	class ZoomFiles
	{
		uint32_t countFiles_ = 0;
		std::vector<FilePtr> files_;
	public:
		ZoomFiles(const uint32_t countFiles)
			:countFiles_(countFiles)
		{
		}
		bool CreateFiles(const path_string & target_folder, const path_string & file_name)
		{
			// Create folder "file_name"
			auto newFolderName = addBackSlash(target_folder) + file_name;
			if (!std::fs::create_directory(newFolderName))
			{
				return false;
			}
			

			for (uint32_t fileNumber = 0; fileNumber < countFiles_; ++fileNumber)
			{ 
				auto number_name = std::to_wstring(fileNumber);
				auto file_ptr = makeFilePtr(addBackSlash(newFolderName) + file_name + number_name + L".wav");
				if (!file_ptr->Open(OpenMode::Create))
					return false;
				files_.push_back(file_ptr);
			}
		}
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
			setBlockSize(32768);
			if (!device_->isOpen())
			{
				LOG_MESSAGE("Error device isn't opened.");
			}
			uint64_t position = start_offset;
			IO::DataArray data_array(this->block_size());

			auto bytesRead = ReadBlock(data_array, position);


		 }
	};

}