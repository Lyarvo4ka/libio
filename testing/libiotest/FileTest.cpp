
#include "MockFile.h"

using ::testing::Test;
using ::testing::_;
using ::testing::Return;

class FileSuit
	: public Test
{

protected:
	void SetUp()
	{
		//EXPECT_CALL(file_, Open(IO::OpenMode::OpenRead))
		//	.WillOnce(Return(true));

		//EXPECT_CALL(file_, isOpen())
		//	.WillOnce(Return(true));

		//EXPECT_CALL(file_, setPosition(_))
		//	.WillOnce();

		//EXPECT_CALL(file_, getPosition())
		//	.WillOnce(Return(10));

	}

	MockFile file_;
};

TEST_F(FileSuit, TestOpenFile)
{
	auto actual = file_.Open(IO::OpenMode::OpenRead);

	ASSERT_TRUE(actual);
}