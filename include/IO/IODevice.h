#pragma once
#include "physicaldrive.h"
#include <string>
#include <string_view>
#include <functional>
#include "dataarray.h"
#include "error.h"

namespace IO
{


	using read_or_write_func = std::function<Error::IOErrorsType(ByteArray, const uint32_t, uint32_t &)>;

	inline uint32_t calcBlockSize(uint64_t current, uint64_t size, uint32_t block_size)
	{
		uint32_t bytes = 0;
		if (current + (uint64_t)block_size <= size)
			bytes = block_size;
		else
			bytes = (uint32_t)(size - current);
		return bytes;
	}

	class IOEngine
	{
		HANDLE hDevice_ = INVALID_HANDLE_VALUE;		
		uint32_t transfer_size_ = default_block_size;
		uint64_t position_ = 0;
		bool bOpen_ = false;

	public:
		virtual ~IOEngine() 
		{
			Close();
		}

		virtual Error::IOErrorsType OpenRead(const path_string & path)
		{
			hDevice_ = ::CreateFile(path.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);
			if (hDevice_ == INVALID_HANDLE_VALUE)
				return Error::IOErrorsType::kOpenRead;

			bOpen_ = true;
			return Error::IOErrorsType::OK;
		}
		virtual Error::IOErrorsType OpenWrite(const path_string & path)
		{
			hDevice_ = ::CreateFile(path.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);
			if (hDevice_ == INVALID_HANDLE_VALUE)
				return Error::IOErrorsType::kOpenWrite;

			bOpen_ = true;
			return Error::IOErrorsType::OK;
		}
		virtual Error::IOErrorsType Create(const path_string & path)
		{
			hDevice_ = ::CreateFile(path.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				CREATE_ALWAYS,
				0,
				NULL);
			if (hDevice_ == INVALID_HANDLE_VALUE)
				return Error::IOErrorsType::kCreate;

			bOpen_ = true;
			return Error::IOErrorsType::OK;

		}
		virtual void Close()
		{
			if (hDevice_ != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hDevice_);
				hDevice_ = INVALID_HANDLE_VALUE;
			}
			bOpen_ = false;
		}
		virtual bool isOpen() const
		{
			return bOpen_;
		}
		virtual void setPostion(uint64_t position)
		{
			position_ = position;
			LARGE_INTEGER liPos = { 0 };
			liPos.QuadPart = position_;
			::SetFilePointerEx(hDevice_, liPos, NULL, FILE_BEGIN);
		}
		uint64_t getPostion() const
		{
			return position_;
		}
		virtual Error::IOErrorsType Read( ByteArray data, const uint32_t read_size , uint32_t & bytes_read)
		{
			auto read_func = std::bind(&IOEngine::read_data, std::ref(*this), data, read_size, bytes_read);
			return ReadOrWriteData(data, read_size, bytes_read, read_func);

		}
		virtual Error::IOErrorsType Write( ByteArray data, const uint32_t write_size, uint32_t & bytes_written)
		{
			auto write_func = std::bind(&IOEngine::write_data, std::ref(*this), data, write_size, bytes_written);
			return ReadOrWriteData(data, write_size, bytes_written, write_func);

		}
		virtual Error::IOErrorsType SetFileSize(uint64_t new_size)
		{
			LARGE_INTEGER li = LARGE_INTEGER();
			li.QuadPart = new_size;
			::SetFilePointerEx(hDevice_, li, NULL, FILE_BEGIN);
			if (auto bResult = ::SetEndOfFile(hDevice_); !bResult)
				return Error::IOErrorsType::kSetFileSize;

			return Error::IOErrorsType::OK;
		}

		virtual Error::IOErrorsType readFileSize(uint64_t & file_size)
		{
			LARGE_INTEGER liSize = { 0 };
			if (!::GetFileSizeEx(hDevice_, &liSize))
			{
				return Error::IOErrorsType::kGetFileSize;
			}
			return Error::IOErrorsType::OK;
		}

		void setTranserSize(const uint32_t transfer_size)
		{
			transfer_size_ = transfer_size;
		}
		uint32_t getTranferSize() const
		{
			return transfer_size_;
		}

	private:

		Error::IOErrorsType ReadOrWriteData(ByteArray data, const uint32_t read_size, uint32_t & bytes_read, read_or_write_func read_write)
		{

			uint32_t data_pos = 0;
			uint32_t bytes_to_read = 0;
			while (data_pos < read_size)
			{
				bytes_to_read = calcBlockSize(data_pos, read_size, getTranferSize());
				setPostion(position_);
				ByteArray pData = data + data_pos;
				if (auto result = read_write(pData, bytes_to_read, bytes_read); result != Error::IOErrorsType::OK)
					return result;
				data_pos += bytes_read;
				position_ += bytes_read;
			}
			bytes_read = data_pos;
			return Error::IOErrorsType::OK;

		}

		Error::IOErrorsType read_data(ByteArray data, uint32_t read_size, uint32_t & bytes_read)
		{
			auto bResult = read_device(hDevice_, data, read_size, bytes_read);
			if (!bResult || (bytes_read == 0))
				return Error::IOErrorsType::kReadData;

			return Error::IOErrorsType::OK;

		}

		Error::IOErrorsType write_data(ByteArray data, uint32_t write_size, uint32_t & bytes_written)
		{
			auto bResult = write_device(hDevice_, data, write_size, bytes_written);
			if (!bResult || (bytes_written == 0))
				return Error::IOErrorsType::kWriteData;

			return Error::IOErrorsType::OK;
		}


		BOOL read_device(HANDLE & hDevice, ByteArray data, const uint32_t bytes_to_read, uint32_t & bytes_read)
		{
			return::ReadFile(hDevice, data, bytes_to_read, reinterpret_cast<LPDWORD>(&bytes_read), NULL);
		}
		BOOL write_device(HANDLE & hDevice, ByteArray data, const uint32_t bytes_to_write, uint32_t & bytes_written)
		{
			return::WriteFile(hDevice, data, bytes_to_write, reinterpret_cast<LPDWORD>(&bytes_written), NULL);
		}

	};




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
				throw Error::IOErrorException(makeErrorStatus(this,result));


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
			io_engine_->getPostion();
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
			assert(!isOpen());
			auto sector_size = physical_drive_->getBytesPerSector();

			if (isMultiple(this->getPosition(), sector_size) && isMultiple(read_size, sector_size))
				return ReadBlock(data, read_size);
			else
				return ReadDataNotAligned(data, read_size);
		}
		uint32_t WriteData(ByteArray data, uint32_t write_size) override
		{
			assert(!isOpen());
			return WriteBlock(data, write_size);
		}

		uint64_t Size() const override
		{
			return physical_drive_->getSize();	// return byte, not sectors
		}

	private:
		uint32_t ReadBlock(ByteArray data, uint32_t read_size) 
		{
			assert(data != nullptr);
			assert(read_size >= 0);

			uint32_t bytes_read = 0;
			if ( auto result = io_engine_->Read(data, read_size, bytes_read); result != Error::IOErrorsType::OK)
				throw Error::IOErrorException(makeErrorStatus(this , result));

			return bytes_read;
		}
		uint32_t WriteBlock(ByteArray data, uint32_t write_size)
		{
			assert(data != nullptr);
			assert(write_size >= 0);

			uint32_t bytes_written = 0;
			if ( auto result = io_engine_->Write(data,write_size, bytes_written); result != Error::IOErrorsType::OK)
				throw Error::IOErrorException(makeErrorStatus(this , result));

			return bytes_written;
		}
	};

}
