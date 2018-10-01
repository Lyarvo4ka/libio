#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "IO/IODevice.h"
using namespace IO;
using namespace IO::Error;

class MockIOEngine :
	public IOEngine
{
public:
	MOCK_METHOD1(OpenRead, IOErrorsType (const path_string &));
};