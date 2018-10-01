#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "IO/IODevice.h"
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::_;
using ::testing::Test;
//using namespace IO;
//using namespace IO::Error;

#include "MockIOEngine.h"

class IOEngineTestSuit
	: public Test
{
protected:
	MockIOEngine io_engine_;
	void SetUp() override
	{
		//io_engine
		EXPECT_CALL(io_engine_, OpenRead(_))
			//.Times(1)
			.WillOnce(Return(IOErrorsType::OK))
			.WillOnce(Return(IOErrorsType::kOpenRead));

	}
};

TEST_F(IOEngineTestSuit, testOpenRead)
{
	//EXPECT_CALL(this, OpenRead(_))
	//	.Times(1)
	//	.WillOnce(Return(IOErrorsType::OK));


	ASSERT_EQ( IOErrorsType::OK, io_engine_.OpenRead(L""));
	auto val = io_engine_.OpenRead(L"");
	//ASSERT_EQ( IOErrorsType::kOpenRead, io_engine_.OpenRead(L""));
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