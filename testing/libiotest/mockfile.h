#pragma once

#include "AbstractRaw.h"

class MockFile
	: public IO::File
{
private:
	IO::DataArray::Ptr data_;
	uint64_t offset_ = 0;
public:
	using Ptr = std::shared_ptr<MockFile>;
	MockFile(uint32_t file_size)
		: IO::File(L"")
		, data_(IO::makeDataArray(file_size))
	{
	}
	IO::DataArray * getData()
	{
		return data_.get();
	}
	bool Open(IO::OpenMode open_mode) override
	{
		return true;
	}
	void Close() override
	{

	}
	bool isOpen() override
	{
		return true;
	}
	void setPosition(uint64_t offset) override
	{
		offset_ = offset;
	}
	uint32_t ReadData(IO::ByteArray data, uint32_t read_size) override
	{
		if (offset_ + read_size <= data_->size())
		{
			memcpy(data, data_->data() + offset_, read_size);
			return read_size;
		}
		return 0;
	}
	virtual uint32_t WriteData(IO::ByteArray data, uint32_t write_size) override
	{
		if (offset_ + write_size >= data_->size())
		{
			auto new_size = offset_ + write_size;
			IO::DataArray * new_data = new IO::DataArray((uint32_t)new_size);
			memcpy(new_data->data(), data_->data(), data_->size());
			data_.reset(new_data);
		}

		memcpy(data_->data() + offset_, data, write_size);
		return write_size;
	}
	virtual uint64_t Size() const
	{
		return data_->size();
	}

};

static MockFile::Ptr makeMockFile(uint32_t size)
{
	return std::make_shared<MockFile>(size);
}




