#include "gtest/gtest.h"

#include "pch.h"
#include "QuickTime.h"


IO::DataArray::Ptr createTeastDataArrayPtr(const uint32_t size, const uint8_t test_val)
{
	auto data_array = IO::makeDataArray(size);
	memset(data_array->data(), test_val, size);
	return data_array;
}

const uint8_t const_header_1[] = { 0xAA, 0xBB, 0xCC, 0xDD };
const uint32_t const_header_1_size = SIZEOF_ARRAY(const_header_1);

const uint8_t const_header_2[] = { 0x00, 0x11 , 0x22 , 0x33, 0x44, 0x55 };
const uint32_t const_header_2_size = SIZEOF_ARRAY(const_header_2);

const uint8_t const_header_3[] = { 0x00, 0x11 , 0x22 , 0x33, 0x44};
const uint32_t const_header_3_size = SIZEOF_ARRAY(const_header_3);

const uint32_t offset_1 = 0;
const uint32_t offset_2 = 10;

IO::ByteArray createFromConstData(const uint8_t const_data[], uint32_t size)
{
	IO::ByteArray data = new uint8_t[size];
	memcpy(data, const_data, size);
	return data;
}

IO::DataArray::Ptr createTestDataBlock()
{
	auto data_array = IO::makeDataArray(512);
	memcpy(data_array->data() + offset_1, const_header_1, const_header_1_size);
	memcpy(data_array->data() + offset_2, const_header_2, const_header_2_size);
	return data_array;
}

void deleteDataArray(IO::DataArray * data_array)
{
	delete data_array;
}

TEST(DataArrayTest, DataArrayConstructors) 
{
	IO::DataArray const_ctor(const_header_1, const_header_1_size);
	EXPECT_EQ(const_ctor.size(), const_header_1_size);
	auto actual = const_ctor.compareData((IO::ByteArray)const_header_1, const_header_1_size);
	EXPECT_TRUE(actual);

	IO::ByteArray data_array = new uint8_t[const_header_1_size];
	memcpy(data_array, const_header_1, const_header_1_size);

	IO::DataArray standard_ctor(data_array, const_header_1_size);
	EXPECT_EQ(standard_ctor.size(), const_header_1_size);
	actual = standard_ctor.compareData((IO::ByteArray)const_header_1, const_header_1_size);
	EXPECT_TRUE(actual);
}

TEST(DataArrayTest, CompareDataArray)
{
	auto data_array = createTeastDataArrayPtr(5, 0xFF);
	auto data2 = createTeastDataArrayPtr(512, 0xFF);
	EXPECT_TRUE(data_array->compareData(data2->data(), 512));
}

TEST(DataArrayTest , DataArrayEqual)
{
	auto data1 = IO::makeDataArray(const_header_1, const_header_1_size);
	auto data2 = IO::makeDataArray(const_header_1, const_header_1_size);
	auto data3 = IO::makeDataArray(const_header_2, const_header_2_size);

	EXPECT_TRUE(data1 == data2);
	EXPECT_FALSE(data1 == data3);

	IO::DataArray data_array1(const_header_1, const_header_1_size);
	IO::DataArray same_data_array1(const_header_1, const_header_1_size);
	IO::DataArray data_array2(const_header_2, const_header_2_size);
	IO::DataArray data_array3(const_header_3, const_header_3_size);

	EXPECT_TRUE(data_array1 == same_data_array1);
	EXPECT_FALSE(data_array1 == data_array2);
	EXPECT_FALSE(data_array2 == data_array3);

}

TEST(DataArrayTest, indexTest)
{
	IO::DataArray data_array(const_header_1, const_header_1_size);
	auto val_AA = data_array[0];
	auto val_BB = data_array[1];
	EXPECT_EQ(val_AA, 0xAA);
	EXPECT_EQ(val_BB, 0xBB);


}
