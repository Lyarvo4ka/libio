#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "IO/IODevice.h"
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::_;
//using namespace IO;
//using namespace IO::Error;

#include "MockIOEngine.h"

TEST(test_IOEngine, test_OpenRead)
{
	MockIOEngine io_engine;
	EXPECT_CALL(io_engine, OpenRead(_))
		.Times(1)
		.WillOnce(Return(IOErrorsType::OK));

	auto actual = io_engine.OpenRead(L"the file");

	ASSERT_EQ(actual, IOErrorsType::OK);
}

//class Base
//{
//public:
//	virtual void test() {};
//};
//
//class MockBase
//	: public Base
//{
//public:
//	MOCK_METHOD0(test, void());
//};

//class MockIOEngine :
//	public IO::IOEngine
//{
//public:
//	MOCK_METHOD1(OpenRead, IO::Error::IOErrorsType (const IO::path_string &));
//};


//
//TEST(ioengineTest, testConstexptrDeviceTypeToString)
//{
//	auto expected_file = IO::DeviceTypeToString(IO::DeviceType::kFile);
//}