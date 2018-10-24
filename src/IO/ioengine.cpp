#include "IO/ioengine.h"

namespace IO
{

	IOEngine::~IOEngine()
	{
		Close();
	}
	IOErrorsType IOEngine::OpenRead(const path_string & path)
	{
		hDevice_ = ::CreateFile(path.c_str(),
			GENERIC_READ /*| GENERIC_WRITE*/,
			FILE_SHARE_READ /*| FILE_SHARE_WRITE*/,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (hDevice_ == INVALID_HANDLE_VALUE)
		{
			auto last_error = ::GetLastError();
			return IOErrorsType::kOpenRead;
		}


		bOpen_ = true;
		return IOErrorsType::OK;
	}
	IOErrorsType IOEngine::OpenWrite(const path_string & path)
	{
		hDevice_ = ::CreateFile(path.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (hDevice_ == INVALID_HANDLE_VALUE)
			return IOErrorsType::kOpenWrite;

		bOpen_ = true;
		return IOErrorsType::OK;
	}
	IOErrorsType IOEngine::Create(const path_string & path)
	{
		hDevice_ = ::CreateFile(path.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL);
		if (hDevice_ == INVALID_HANDLE_VALUE)
			return IOErrorsType::kCreate;

		bOpen_ = true;
		return IOErrorsType::OK;

	}
	void IOEngine::Close()
	{
		if (hDevice_ != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hDevice_);
			hDevice_ = INVALID_HANDLE_VALUE;
		}
		bOpen_ = false;
	}
	bool IOEngine::isOpen() const
	{
		return bOpen_;
	}
	void IOEngine::setPostion(uint64_t position)
	{
		if (position_ != position)
		{
			position_ = position;
			LARGE_INTEGER liPos = { 0 };
			liPos.QuadPart = position_;
			::SetFilePointerEx(hDevice_, liPos, NULL, FILE_BEGIN);
		}
	}
	uint64_t IOEngine::getPostion() const
	{
		return position_;
	}
	IOErrorsType IOEngine::Read(ByteArray data, const uint32_t read_size, uint32_t & bytes_read)
	{
		uint32_t data_pos = 0;
		uint32_t bytes_to_read = 0;
		while (data_pos < read_size)
		{
			bytes_to_read = calcBlockSize(data_pos, read_size, getTranferSize());
			setPostion(position_);
			ByteArray pData = data + data_pos;

			auto result = read_data(pData, bytes_to_read, bytes_read);
			if (result != IOErrorsType::OK)
				return result;
			data_pos += bytes_read;
			position_ += bytes_read;
		}
		bytes_read = data_pos;
		return IOErrorsType::OK;

		//auto read_func = std::bind(&IOEngine::read_data, std::ref(*this), data, read_size, bytes_read);
		//return ReadOrWriteData(data, read_size, bytes_read, read_func);

	}
	IOErrorsType IOEngine::Write(ByteArray data, const uint32_t write_size, uint32_t & bytes_written)
	{
		uint32_t data_pos = 0;
		uint32_t bytes_to_read = 0;
		while (data_pos < write_size)
		{
			bytes_to_read = calcBlockSize(data_pos, write_size, getTranferSize());
			setPostion(position_);
			ByteArray pData = data + data_pos;

			auto result = write_data(pData, bytes_to_read, bytes_written);
			if (result != IOErrorsType::OK)
				return result;
			data_pos += bytes_written;
			position_ += bytes_written;
		}
		bytes_written = data_pos;
		return IOErrorsType::OK;

		//auto write_func = std::bind(&IOEngine::write_data, std::ref(*this), data, write_size, bytes_written);
		//return ReadOrWriteData(data, write_size, bytes_written, write_func);

	}
	IOErrorsType IOEngine::SetFileSize(uint64_t new_size)
	{
		LARGE_INTEGER li = LARGE_INTEGER();
		li.QuadPart = new_size;
		::SetFilePointerEx(hDevice_, li, NULL, FILE_BEGIN);
		if (auto bResult = ::SetEndOfFile(hDevice_); !bResult)
			return IOErrorsType::kSetFileSize;

		return IOErrorsType::OK;
	}
	IOErrorsType IOEngine::readFileSize(uint64_t & file_size)
	{
		LARGE_INTEGER liSize = { 0 };
		auto bResult = ::GetFileSizeEx(hDevice_, &liSize);
		if (!bResult)
			return IOErrorsType::kGetFileSize;

		file_size = liSize.QuadPart;
		return IOErrorsType::OK;
	}
	void IOEngine::setTranserSize(const uint32_t transfer_size)
	{
		transfer_size_ = transfer_size;
	}
	uint32_t IOEngine::getTranferSize() const
	{
		return transfer_size_;
	}
	IOErrorsType IOEngine::ReadOrWriteData(ByteArray data, const uint32_t read_size, uint32_t & bytes_read, read_or_write_func read_write)
	{
		uint32_t data_pos = 0;
		uint32_t bytes_to_read = 0;
		while (data_pos < read_size)
		{
			bytes_to_read = calcBlockSize(data_pos, read_size, getTranferSize());
			setPostion(position_);
			ByteArray pData = data + data_pos;

			auto result = read_write(pData, bytes_to_read, bytes_read); 
			if( result != IOErrorsType::OK)
				return result;
			data_pos += bytes_read;
			position_ += bytes_read;
		}
		bytes_read = data_pos;
		return IOErrorsType::OK;

	}


	IOErrorsType IOEngine::read_data(ByteArray data, uint32_t read_size, uint32_t & bytes_read)
	{
		auto bResult = read_device(hDevice_, data, read_size, bytes_read);
		if (!bResult || (bytes_read == 0))
			return IOErrorsType::kReadData;

		return IOErrorsType::OK;
	}
	Error::IOErrorsType IOEngine::write_data(ByteArray data, uint32_t write_size, uint32_t & bytes_written)
	{
		auto bResult = write_device(hDevice_, data, write_size, bytes_written);
		if (!bResult || (bytes_written == 0))
			return IOErrorsType::kWriteData;

		return IOErrorsType::OK;
	}
	BOOL IOEngine::read_device(HANDLE & hDevice, ByteArray data, const uint32_t bytes_to_read, uint32_t & bytes_read)
	{
		return::ReadFile(hDevice, data, bytes_to_read, reinterpret_cast<LPDWORD>(&bytes_read), NULL);
	}
	BOOL IOEngine::write_device(HANDLE & hDevice, ByteArray data, const uint32_t bytes_to_write, uint32_t & bytes_written)
	{
		return::WriteFile(hDevice, data, bytes_to_write, reinterpret_cast<LPDWORD>(&bytes_written), NULL);
	}
}