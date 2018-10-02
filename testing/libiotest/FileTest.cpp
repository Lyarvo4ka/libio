
#include "IO/IODevice.h"
#include "MockFile.h"

using ::testing::Test;

class FileSuit
	: public Test
{

protected:
	void SetUp()
	{

	}

	MockFile file_;
};