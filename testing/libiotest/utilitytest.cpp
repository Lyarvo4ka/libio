
#include "pch.h"
#include "IO\utility.h"

TEST( UtilityTest, fileNameToOffsetTest)
{
	IO::path_string file1 = L"d:\\Temp\\1\\img016937500.bin";
	auto offset_val = IO::fileNameToOffset(file1);
	EXPECT_EQ(offset_val, 16937500);

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