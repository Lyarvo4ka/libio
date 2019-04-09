#pragma once
#include "physicaldrive.h"
#include <string>
#include <string_view>
#include <functional>
#include "dataarray.h"
#include "error.h"

#include "ioengine.h"

namespace IO
{





	inline bool isMultiple(uint64_t number, uint32_t multiple_by)
	{
		if (multiple_by == 0)
			return false;
		return (number % multiple_by == 0);
	}

	const std::string_view file_txt = "file";
	const std::string_view disk_txt = "disk";
	const std::string_view unknown_txt = "unknown";
	enum class DeviceType
	{
		kFile ,
		kDisk 
	};

	static const std::string_view DeviceTypeToString(DeviceType device_type)
	{
		switch (device_type)
		{
		case DeviceType::kFile:	return file_txt;
		case DeviceType::kDisk: return disk_txt;
				
		default:
			return unknown_txt;
		}
	}




	class IODevice
	{
	public:
		virtual ~IODevice() = 0 {};
		virtual bool Open(OpenMode) = 0;
		virtual void Close() = 0;
		virtual bool isOpen() = 0;
		virtual void setPosition(uint64_t offset) = 0;
		virtual uint64_t getPosition()  const = 0;
		virtual uint32_t ReadData(ByteArray data, uint32_t read_size) = 0;
		virtual uint32_t WriteData(ByteArray data, uint32_t read_size) = 0;
		virtual uint64_t Size() const = 0;
		virtual std::string getDeviceTypeName() const = 0;
	};


		inline Error::IOStatus makeErrorStatus(IODevice & io_device , Error::IOErrorsType error_type)
		{
			auto error_message = Error::getDiskOrFileError(error_type, io_device.getDeviceTypeName());
			auto lastError = ::GetLastError();
			Error::IOStatus error_status(error_type, error_message, lastError);
			return error_status;

		}
		inline Error::IOStatus makeErrorStatus(IODevice * io_device, Error::IOErrorsType error_type)
		{
			auto error_message = Error::getDiskOrFileError(error_type, io_device->getDeviceTypeName());
			auto lastError = ::GetLastError();
			Error::IOStatus error_status(error_type, error_message, lastError);
			return error_status;

		}


	using IODevicePtr = std::shared_ptr<IODevice>;

	class File
		: public IODevice
	{
	private:
		std::unique_ptr<IOEngine> io_engine_ = std::make_unique<IOEngine>();
		uint64_t size_;
		path_string file_name_;

	public:
		File(const path_string & file_name)
			: size_(0)
			, file_name_(file_name)
		{

		}
		~File()
		{
			Close();
		}

		bool OpenRead()
		{
			return Open(OpenMode::OpenRead);
		}
		bool OpenWrite()
		{
			return Open(OpenMode::OpenWrite);
		}
		bool OpenCreate()
		{
			return Open(OpenMode::Create);
		}

		bool Open( OpenMode openMode) override
		{
			Error::IOErrorsType result = Error::IOErrorsType::OK;
			switch (openMode)
			{
			case OpenMode::OpenRead:
				result = io_engine_->OpenRead(file_name_);
				break;
			case OpenMode::OpenWrite:
				result = io_engine_->OpenWrite(file_name_);
				break;
			case OpenMode::Create:
				result = io_engine_->Create(file_name_);
				break;
			}

			if (result != Error::IOErrorsType::OK)
			{
				Error::IOErrorException error_exception(makeErrorStatus(this, result));
				throw error_exception;
			}


			auto error_status = readFileSize(size_);
			if (!error_status.isOK())
				throw Error::IOErrorException(error_status);

			return io_engine_->isOpen();
		}

		
		void Close() override
		{
			io_engine_->Close();
		}
		bool isOpen() override
		{
			return io_engine_->isOpen();
		}

		void setPosition(uint64_t offset) override
		{
			io_engine_->setPostion(offset);
		}

		uint64_t getPosition() const override
		{
			return io_engine_->getPostion();
		}

		uint32_t ReadData(ByteArray data, uint32_t read_size) override
		{
			assert(data != nullptr);
			assert(read_size >= 0);

			uint32_t bytes_read = 0;

			auto result = io_engine_->Read(data, read_size, bytes_read);
			if (result == Error::IOErrorsType::OK)
				return bytes_read;
			
			throw Error::IOErrorException(makeErrorStatus(this, result));
		}

		uint32_t ReadData(DataArray & data_array)
		{
			return ReadData(data_array.data(), data_array.size());
		}

		uint32_t WriteText(const std::string_view text_data)
		{
			return WriteData((ByteArray)text_data.data(), static_cast<uint32_t>(text_data.length()));
		}
		uint32_t WriteData(ByteArray data, uint32_t write_size) override
		{
			assert(data != nullptr);
			assert(write_size >= 0);

			uint32_t bytes_written = 0;
			auto result = io_engine_->Write(data, write_size, bytes_written);
			if (result == Error::IOErrorsType::OK)
				return bytes_written;

			//	//ERROR_DISK_FULL
			throw Error::IOErrorException(makeErrorStatus(this , result));
		};

		uint64_t Size() const override
		{
			return size_;
		}

		std::string getDeviceTypeName() const override
		{
			return file_txt.data();
		}
		void setSize(uint64_t new_size)
		{
			auto status = makeErrorStatus(this,io_engine_->SetFileSize(new_size));
			if (!status.isOK())
				throw Error::IOErrorException(status);
			size_ = new_size;
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
		Error::IOStatus readFileSize(uint64_t & file_size)
		{
			return makeErrorStatus(this,io_engine_->readFileSize(file_size));
		}

	};

	using FilePtr = std::shared_ptr<File>;
	inline FilePtr makeFilePtr(const path_string & file_name)
	{
		return std::make_shared<File>(file_name);
	}

	//class BlockDevice
	//	: public BasicDevice
	//{
	//public:
	//	virtual uint32_t ReadBlock(ByteArray data, uint32_t read_size) = 0;
	//	virtual uint32_t WriteBlock(ByteArray data, uint32_t read_size) = 0;

	//};


	class DiskDevice
		: public IODevice
	{
	private:
		std::unique_ptr<IOEngine> io_engine_ = std::make_unique<IOEngine>();
		PhysicalDrivePtr physical_drive_;
	public:
		DiskDevice(PhysicalDrivePtr physical_drive)
			: physical_drive_(physical_drive)
		{
			io_engine_->setTranserSize(physical_drive_->getTransferLength());
		}
		bool Open(OpenMode open_mode) override
		{
			if (physical_drive_)
			{
				auto status = makeErrorStatus(this,io_engine_->OpenRead(physical_drive_->getPath()));
				if (!status.isOK())
					throw Error::IOErrorException(status);
			}
			return io_engine_->isOpen();
		}
		void Close() override
		{
			io_engine_->Close();

		}
		bool isOpen() override
		{
			return io_engine_->isOpen();
		}
		void setPosition(uint64_t offset) override
		{
			io_engine_->setPostion(offset);

		}
		uint64_t getPosition() const 
		{
			return io_engine_->getPostion();
		}

		uint32_t ReadDataNotAligned(ByteArray data, uint32_t read_size)
		{
			const auto sector_size = physical_drive_->getBytesPerSector();
			const uint32_t data_start = this->getPosition() % sector_size;
			const int sector_to_read = (data_start + read_size) / sector_size + 1;
			const int bytes_to_read = sector_to_read * sector_size;

			
			DataArray temp_data(bytes_to_read);
			auto bytes_read = ReadBlock(temp_data.data() , temp_data.size());

			memcpy(data, temp_data.data() + data_start, read_size);
			return read_size;
		}
		uint32_t ReadData(ByteArray data, uint32_t read_size) override
		{
			assert(isOpen() == true);
			auto sector_size = physical_drive_->getBytesPerSector();

			if (isMultiple(this->getPosition(), sector_size) && isMultiple(read_size, sector_size))
				return ReadBlock(data, read_size);
			else
				return ReadDataNotAligned(data, read_size);
		}
		uint32_t WriteData(ByteArray data, uint32_t write_size) override
		{
			assert(isOpen() == true);
			return WriteBlock(data, write_size);
		}

		uint64_t Size() const override
		{
			return physical_drive_->getSize();	// return byte, not sectors
		}
		std::string getDeviceTypeName() const override
		{
			return disk_txt.data();
		}
	private:
		uint32_t ReadBlock(ByteArray data, uint32_t read_size) 
		{
			assert(data != nullptr);
			assert(read_size >= 0);

			uint32_t bytes_read = 0;
		    auto result = io_engine_->Read(data, read_size, bytes_read); 
			if (result != Error::IOErrorsType::OK)
				throw Error::IOErrorException(makeErrorStatus(this , result));

			return bytes_read;
		}
		uint32_t WriteBlock(ByteArray data, uint32_t write_size)
		{
			assert(data != nullptr);
			assert(write_size >= 0);

			uint32_t bytes_written = 0;
			auto result = io_engine_->Write(data,write_size, bytes_written); 
			if (result != Error::IOErrorsType::OK)
				throw Error::IOErrorException(makeErrorStatus(this , result));

			return bytes_written;
		}
	};

}
