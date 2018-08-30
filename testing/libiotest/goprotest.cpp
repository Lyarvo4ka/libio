#include "pch.h"

#include "IO\GoPro.h"
//
TEST(goprotest, calcNumberToSkipTest)
{
	std::vector<uint32_t> vecTest = { 0,0,10,10,10,10,10,0,10,10,10,0,0,0,0,0,0,0 };
	IO::GoProFile gpFile;
	gpFile.setCluserMap(vecTest);
	gpFile.setNullsBorder(9);
	auto lastIter = gpFile.findFromEnd();
	//if (lastIter != vecTest.rend())
	{
		auto count = gpFile.calcNumberToSkip(lastIter);
		int k = 1;
		k = 2;
	}

	//IO::GoProFile gpFile;
}
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