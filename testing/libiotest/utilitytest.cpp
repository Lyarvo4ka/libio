
#include "pch.h"
#include "IO\utility.h"
#include <string>
#include "IO\GoPro.h"

TEST( UtilityTest, fileNameToOffsetTest)
{
	IO::path_string file1 = L"d:\\Temp\\1\\img016937500.bin";
	auto offset_val = IO::fileNameToOffset(file1);
	EXPECT_EQ(offset_val, 16937500);

}

TEST(UtilityTest, toStringTest)
{
	uint32_t int_val = 0;
	auto str = std::to_string(int_val);
	auto result = IO::clusterNullsToSstring(0, 1);

	int k = 1;
	k = 2;

}

//TEST(UtilityTest, testAcronis_decompress)
//{
//	IO::path_string tib_file = L"g:\\src_file.bin";
//	IO::path_string dst_folder = L"g:\\decompress\\";
//
//	IO::AcronisDecompress acronis_decompressor(tib_file);
//	uint64_t offset = 0;
//
//	auto file_name = IO::offsetToPath(dst_folder, offset, L"bin");
//	offset = acronis_decompressor.saveToFile(file_name);
//}