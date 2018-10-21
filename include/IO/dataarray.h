#pragma once

#include <memory>
#include "constants.h"


namespace IO
{
	class DataArray
	{
		ByteArray data_;
		uint32_t size_;
	public:
		using Ptr = std::unique_ptr<DataArray>;

		DataArray(const uint32_t size)
			:data_(nullptr)
			, size_(size)
		{
			if (size > 0)
			{
				data_ = new uint8_t[size];
			}
		}
		DataArray(ByteArray data, uint32_t size)
			: data_(data)
			, size_(size)
		{

		}
		DataArray(const uint8_t const_data[], uint32_t size)
			: data_(nullptr)
			, size_(size)
		{
			if (size_ > 0)
			{
				data_ = new uint8_t[size_];
				memcpy(data_, const_data, size_);

			}
		}
		DataArray(DataArray && tmp_data_array)
			: data_(tmp_data_array.data_)
			, size_(tmp_data_array.size_)
		{
			//printf("Move constructor called.");
			tmp_data_array.data_ = nullptr;
			tmp_data_array.size_ = 0;
		}

		~DataArray()
		{
			clear();
		}
		uint32_t size() const
		{
			return size_;
		}
		ByteArray data()
		{
			return data_;
		}
		ByteArray data() const
		{
			return data_;
		}
		void resize(const uint32_t new_size)
		{
			if (size_ != new_size)
			{
				clear();
				data_ = new uint8_t[new_size];
			}
		}
		void clear()
		{
			if (data_)
			{
				delete[] data_;
				data_ = nullptr;
			}

		}
		static bool compareData(const DataArray & left, const DataArray & right)
		{
			if (left.size() == right.size())
				return (memcmp(left.data(), right.data(), left.size()) == 0);
			return false;
		}
		friend bool operator == (const DataArray & left, const DataArray & right)
		{
			return compareData(left, right);
		}	

		friend bool operator == (const DataArray::Ptr & left, const DataArray::Ptr & right)
		{
			return compareData(*left , *right);
		}
	
		uint8_t & operator[](size_t index)
		{
			return data_[index];
		}

		uint8_t operator[](size_t index) const
		{
			return data_[index];
		}

		bool compareData(const ByteArray data, uint32_t size, uint32_t offset = 0)
		{
			if (size >= this->size())
			{

				if (std::memcmp(data_, data + offset, this->size()) == 0)
					return true;
			}
			return false;
		}
		bool compareData(const DataArray & dataArray, uint32_t offset = 0)
		{
			return compareData(dataArray.data(), dataArray.size(), offset);
		}

	};

	inline DataArray::Ptr makeDataArray(ByteArray data, uint32_t size)
	{
		return std::make_unique<DataArray>(data, size);
	}
	inline DataArray::Ptr makeDataArray(const uint8_t const_data[], uint32_t size)
	{
		return std::make_unique<DataArray>(const_data, size);
	}

	inline DataArray::Ptr makeDataArray(uint32_t size)
	{
		return std::make_unique<DataArray>(size);
	}
}