#pragma once
#include "physicaldrive.h"
#include <string>
#include "dataarray.h"
#include "error.h"

namespace IO
{
	


	inline uint32_t calcBlockSize(uint64_t current, uint64_t size, uint32_t block_size)
	{
		uint32_t bytes = 0;
		if (current + (uint64_t)block_size <= size)
			bytes = block_size;
		else
			bytes = (uint32_t)(size - current);
		return bytes;
	}
	inline bool isMultiple(uint64_t number, uint32_t multiple_by)
	{
		if (multiple_by == 0)
			return false;
		return (number % multiple_by == 0);
	}


	class IODevice
	{
	public:
		virtual ~IODevice() = 0 {};
		virtual bool Open(OpenMode) = 0;
		virtual void Close() = 0;
		virtual bool isOpen() = 0;
		virtual void setPosition(uint64_t offset) = 0;
		virtual uint32_t ReadData(ByteArray data, uint32_t read_size) = 0;
		virtual uint32_t WriteData(ByteArray data, uint32_t read_size) = 0;
		virtual uint64_t Size() const = 0;
	};

	using IODevicePtr = std::shared_ptr<IODevice>;

	class File
		: public IODevice
	{
	private:
		HANDLE hFile_;
		uint64_t position_;
		uint64_t size_;
		path_string file_name_;
		bool bOpen_;
	public:
		File(const path_string & file_name)
			: hFile_(INVALID_HANDLE_VALUE)
			, position_( 0 )
			, size_(0)
			, file_name_(file_name)
			, bOpen_(false)
		{

		}
		~File()
		{
			Close();
		}

		bool Open( OpenMode openMode) override
		{
			uint32_t win_open_mode = 0;

			switch (openMode)
			{
			case OpenMode::OpenRead:
			case OpenMode::OpenWrite:
				win_open_mode = OPEN_EXISTING;
				break;
			case OpenMode::Create:
				win_open_mode = CREATE_ALWAYS;
				break;
			}

			hFile_ = ::CreateFile(file_name_.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				win_open_mode,
				0,
				NULL);

			if (hFile_ != INVALID_HANDLE_VALUE)
			{
				bOpen_ = true;
				LARGE_INTEGER liSize = { 0 }; 
				::GetFileSizeEx(hFile_, &liSize);
				size_ = liSize.QuadPart;
			}
			else
			{
				bOpen_ = false;
				auto err = ErrorHandler::get();
				DWORD dwError = ::GetLastError();
				//Error error();
				auto openmode_error = Error::OpenModeToError(openMode);
				auto error_string = Error::getDiskOrFileError(openmode_error, "file");
				err->showMessage(error_string);

				//printf(error_string.c_str());
				//***********************
				//1. (OpenMode::OpenRead)			Error opening the file for reading
				//2. (OpenMode::OpenWrite)			Error opening file for writing
				//3. (OpenMode::Create)				Error creating file
				//4. (ReadData)						Error reading from file
				//4. (WriteData)					Error writing to file
				//***********************
				int k = 1;
				k = 2;
				//auto device_err = make_error_condition(openModeToIOError(openMode));
				
				//device_err.message()
				//auto device_error = openModeToIOError(openMode);
				//err->showMessage(dwError, ioDeviceErrorToString(device_error), file_name_);
			}

			return bOpen_;
		}
		
		void Close() override
		{
			if (hFile_ != INVALID_HANDLE_VALUE)
			{
				::CloseHandle(hFile_);
				hFile_ = INVALID_HANDLE_VALUE;
			}
			bOpen_ = false;

		}
		bool isOpen()
		{
			return bOpen_;
		}

		void setPosition(uint64_t offset) override
		{
			position_ = offset;
			LARGE_INTEGER liPos = { 0};
			liPos.QuadPart = position_;
			::SetFilePointerEx(hFile_, liPos, NULL, FILE_BEGIN);
		};

		uint32_t ReadData(ByteArray data, uint32_t read_size) override
		{
			if (data == nullptr)
				return 0;
			if (read_size == 0)
				return 0;

			DWORD bytes_read = 0;


			if (!::ReadFile(hFile_, data, read_size, &bytes_read, NULL))
			{
				auto err = ErrorHandler::get();
				err->showMessage(Error::getDiskOrFileError(Error::DeviceErrors::kReadData, "file"));
				auto dwError = ::GetLastError();
				return 0;
			}
			return bytes_read;
		};

		uint32_t ReadData(DataArray & data_array)
		{
			auto transfer_size = default_block_size;

			DWORD bytes_read = 0;
			uint32_t data_pos = 0;
			uint32_t bytes_to_read = 0;
			while (data_pos < data_array.size())
			{
				bytes_to_read = calcBlockSize(data_pos, data_array.size(), transfer_size);
				setPosition(position_);
				if (!::ReadFile(hFile_, data_array.data() + data_pos, bytes_to_read, &bytes_read, NULL))
				{
					auto err = ErrorHandler::get();
					err->showMessage(Error::getDiskOrFileError(Error::DeviceErrors::kReadData, "file"));
					return 0;
				}
				if (bytes_read == 0)
				{
					auto err = ErrorHandler::get();
					err->showMessage(Error::getDiskOrFileError(Error::DeviceErrors::kReadData, "file"));
					return 0;
				}
				data_pos += bytes_read;
				position_ += bytes_read;
			}
			bytes_read = data_pos;
			return bytes_read;
		}

		uint32_t WriteData(ByteArray data, uint32_t write_size) override
		{
			if (data == nullptr)
				return 0;
			if (write_size == 0)
				return 0;

			DWORD bytes_written = 0;


			if (!::WriteFile(hFile_, data, write_size, &bytes_written, NULL))
			{
				auto dwLastError = ::GetLastError();
				auto err = ErrorHandler::get();
				err->showMessage(Error::getDiskOrFileError(Error::DeviceErrors::kWriteData, "file"));
				//ERROR_DISK_FULL
				err->showMessage(err->getMessage(dwLastError));

				return 0;
			}
			size_ += bytes_written;
			return bytes_written;
		};

		uint64_t Size() const override
		{
			return size_;
		}
		void setSize(uint64_t new_size)
		{
			if (size_ != new_size)
			{
				size_ = new_size;
				LARGE_INTEGER li = { 0 };
				li.QuadPart = size_;
				::SetFilePointerEx(hFile_, li, NULL, FILE_BEGIN);
				::SetEndOfFile(hFile_);
			}
		}
		void setFileName(const path_string new_filename)
		{
			Close();
			file_name_ = new_filename;
		}
		path_string getFileName() const
		{
			return file_name_;
		}
	};

	using FilePtr = std::shared_ptr<File>;
	inline FilePtr makeFilePtr(const path_string & file_name)
	{
		return std::make_shared<File>(file_name);
	}

	class BlockDevice
		: public IODevice
	{
	public:
		virtual uint32_t ReadBlock(ByteArray data, uint32_t read_size) = 0;
		virtual uint32_t WriteBlock(ByteArray data, uint32_t read_size) = 0;

	};


	class DiskDevice
		: public BlockDevice
	{
	private:
		HANDLE hDrive_;
		bool bOpen_;
		uint64_t position_;
		PhysicalDrivePtr physical_drive_;
	public:
		DiskDevice(PhysicalDrivePtr physical_drive)
			:hDrive_(INVALID_HANDLE_VALUE)
			, bOpen_(false)
			, position_(0)
			, physical_drive_(physical_drive)
		{
			system_oi_ = std::make_unique<SystemIO>();
		}
		DiskDevice(PhysicalDrivePtr physical_drive , std::unique_ptr<SystemIO> system_io)
			:hDrive_(INVALID_HANDLE_VALUE)
			, bOpen_(false)
			, position_(0)
			, physical_drive_(physical_drive)
			, system_oi_(std::move(system_io))
		{
		}
		bool Open(OpenMode open_mode) override
		{
			if (physical_drive_)
			{
				hDrive_ = system_oi_->OpenPhysicalDrive(physical_drive_->getPath());
				if (hDrive_ != INVALID_HANDLE_VALUE)
				{
					bOpen_ = true;
					return true;
				}

			}
			bOpen_ = false;
			//***********************
			//1. (OpenMode::OpenRead)			Error opening the disk for reading
			//2. (OpenMode::OpenWrite)			Error opening disk for writing
			//3. (OpenMode::Create)				Error cannot open physical drive for creating
			//4. (ReadData)						Error reading from disk
			//4. (WriteData)					Error writing to disk
			//***********************

			return bOpen_;
		}
		void Close() override
		{
			if (hDrive_ != INVALID_HANDLE_VALUE)
				CloseHandle(hDrive_);
			bOpen_ = false;

		}
		bool isOpen() override
		{
			return bOpen_;
		}
		void setPosition(uint64_t offset) override
		{
			//if (offset != position_)
			{
				position_ = offset;
				LARGE_INTEGER li{ 0};
				li.QuadPart = position_;
				::SetFilePointerEx(hDrive_, li, nullptr, FILE_BEGIN);
			}
		}
		uint64_t getPosition() const 
		{
			return position_;
		}

		uint32_t ReadDataNotAligned(ByteArray data, uint32_t read_size)
		{
			DWORD numByteRead = 0;
			auto sector_size = physical_drive_->getBytesPerSector();
			uint32_t data_start = position_ % sector_size;
			int sector_to_read = (data_start + read_size) / sector_size + 1;
			int bytes_to_read = sector_to_read * sector_size;

			if (bytes_to_read > 0)
			{
				ByteArray temp_buffer = new uint8_t[bytes_to_read];

				uint64_t aling_offset = position_ / sector_size;
				aling_offset *= sector_size;
				setPosition(aling_offset);
				if (system_oi_->ReadFile(hDrive_, temp_buffer, bytes_to_read, &numByteRead, NULL))
					if (numByteRead > 0)
					{
						memcpy(data, temp_buffer + data_start, read_size);
						numByteRead = read_size;
					}
				delete[] temp_buffer;
			}

			position_ += (numByteRead + data_start);
			return numByteRead;

		}
		uint32_t ReadData(ByteArray data, uint32_t read_size) override
		{
			if (!isOpen())
				return 0;
			if (data == nullptr)
				return 0;
			if (read_size == 0)
				return 0;

			auto sector_size = physical_drive_->getBytesPerSector();


			if (isMultiple(position_, sector_size) && isMultiple(read_size, sector_size))
			{
				return ReadBlock(data, read_size);
			}
			else
			{
				return ReadDataNotAligned(data, read_size);
			}
		}
		uint32_t WriteData(ByteArray data, uint32_t write_size) override
		{
			DWORD bytes_written = 0;
			uint64_t data_pos = 0;
			uint32_t bytes_to_write = default_block_size;

			// copy paste
			auto transfer_size = this->physical_drive_->getTransferLength();
			while (data_pos < write_size)
			{
				bytes_to_write = calcBlockSize(data_pos, write_size, transfer_size);
				setPosition(position_);// ??? not work
				if (!system_oi_->WriteFile(hDrive_, data + data_pos, bytes_to_write, &bytes_written, NULL))
					return 0;
				if (bytes_to_write == 0)
					return 0;

				data_pos += bytes_written;
				position_ += bytes_written;
			}
			return static_cast<uint32_t>(data_pos);

		}

		uint64_t Size() const override
		{
			return physical_drive_->getSize();	// return byte, not sectors
		}

		uint32_t ReadBlock(ByteArray data, uint32_t read_size) override
		{
			if (!isOpen())
				return 0;
			if (data == nullptr)
				return 0;
			if (read_size == 0)
				return 0;
			if (!isMultiple(position_, physical_drive_->getBytesPerSector()))
				return 0;
			if (!isMultiple(read_size , physical_drive_->getBytesPerSector()))
				return 0;
				
			DWORD bytes_read = 0;
			uint32_t data_pos = 0;
			uint32_t bytes_to_read = 0;
			auto transfer_size = this->physical_drive_->getTransferLength();

			while (data_pos < read_size)
			{
				bytes_to_read = calcBlockSize(data_pos, read_size, transfer_size);
				setPosition(position_);// ??? not work
				if (!system_oi_->ReadFile(hDrive_, data + data_pos, bytes_to_read, &bytes_read, NULL))
					return 0;
				if (bytes_read == 0)
					return 0;

				data_pos += bytes_read;
				position_ += bytes_read;
			}
			bytes_read = data_pos;
			return bytes_read;


		}
		uint32_t WriteBlock(ByteArray data, uint32_t write_size) override
		{
			//if ( !isOpen())
			//	return 0;
			DWORD bytes_written = 0;

			BOOL bResult = WriteFile(hDrive_, data , write_size, &bytes_written, NULL);
			if (!bResult)
				return 0;
			return bytes_written;
		}


		private:
			std::unique_ptr<SystemIO> system_oi_;



	};
	//using DiskDevicePtr = std::shared_ptr<DiskDevice>;
	//inline DiskDevicePtr makeDiskDevicePtr(DiskDevice * pFile)
	//{
	//	return std::make_shared<DiskDevice>(pFile);
	//}

	//class FileDevice
	//	: public BlockDevice
	//{
	//public:
	//	FileDevice(const path_string & path)
	//		: BlockDevice(path)
	//	{

	//	}

	//};

}
