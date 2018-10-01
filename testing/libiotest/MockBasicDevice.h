#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "IO/IODevice.h"
using namespace IO;
using namespace IO::Error;
#include "MockIOEngine.h"

class MockBasicDevice :
	public BasicDevice
{
public:
	//MOCK_METHOD1(setPosition , )

};