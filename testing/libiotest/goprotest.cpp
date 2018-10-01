#include "pch.h"

#include "IO\GoPro.h"
/*
TEST(goprotest, calcNumberToSkipTest)
{
	std::vector<uint32_t> vecTest = { 0,0,10,10,10,10,10,0,10,10,10,0,0,0,0,0,0,0 };
	IO::GoProFile gpFile;
	gpFile.setCluserMap(vecTest);
	gpFile.setNullsBorder(9);
	uint32_t number_of = 0;
	auto bResult = gpFile.findBackwardNumberOf(10, number_of);
	EXPECT_TRUE(bResult);
	EXPECT_EQ(number_of, 9);

	std::vector<uint32_t> only_one = { 0,0,0,0,0,0,0,0,0,0,10,0,0,0,0,0,0,0 };
	gpFile.setCluserMap(only_one);
	bResult = gpFile.findBackwardNumberOf(10, number_of);
	EXPECT_TRUE(bResult);
	EXPECT_EQ(number_of, 1);

	std::vector<uint32_t> vec16Test = { 0,0,10,10,10,10,10,10,10,10,10,0,10,10,10,10,10,10,10,10,10,10,10,0,0,0,0 };
	gpFile.setCluserMap(vec16Test);
	bResult = gpFile.findBackwardNumberOf(22, number_of);
	EXPECT_TRUE(bResult);
	EXPECT_EQ(number_of, 16);

	std::vector<uint32_t> vec12Test = { 0,0,0,0,0,0,0,0,0,10,10,0,10,10,10,10,10,10,10,10,10,0,0,0,0,0,0 };
	gpFile.setCluserMap(vec12Test);
	bResult = gpFile.findBackwardNumberOf(20, number_of);
	EXPECT_TRUE(bResult);
	EXPECT_EQ(number_of, 12);

	std::vector<uint32_t> vecStart = { 10,10,10,10,10,10,10,0,10,10,10,0,0,0,0,0,0,0 };
	gpFile.setCluserMap(vecStart);
	bResult = gpFile.findBackwardNumberOf(10, number_of);
	EXPECT_TRUE(bResult);
	EXPECT_EQ(number_of, 10);

}

TEST(goprotest, findFromEndTest)
{
	std::vector<uint32_t> vecTest = { 0,10,0,0,0,0,0,0 };
	IO::GoProFile gpFile;
	gpFile.setCluserMap(vecTest);
	gpFile.setNullsBorder(9);
	uint32_t pos = 0;
	auto bFound = gpFile.findFromEnd(5, pos);
	EXPECT_TRUE(bFound);
	EXPECT_EQ(pos, 1);
	EXPECT_EQ(vecTest[pos], 10);

	std::vector<uint32_t> vecTest7 = { 0,0,0,0,0,0,0,10 };
	gpFile.setCluserMap(vecTest7);
	bFound = gpFile.findFromEnd(7, pos);
	EXPECT_TRUE(bFound);
	EXPECT_EQ(pos, 7);
	EXPECT_EQ(vecTest7[pos], 10);

	std::vector<uint32_t> vecNulls = { 0,0,0,0,0,0,0,0 };
	gpFile.setCluserMap(vecNulls);
	bFound = gpFile.findFromEnd(7, pos);
	EXPECT_FALSE(bFound);



}*/
//
//TEST(goprotest, toStringTest)
//{
//	uint32_t int_val = 0;
//	auto str = std::to_string(int_val);
//	auto result = IO::clusterNullsToSstring(0, 1);
//
//	int k = 1;
//	k = 2;
//
//}