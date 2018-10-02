#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "IO/IODevice.h"
using namespace IO;
using namespace IO::Error;

class MockFile :
	public IOEngine
{
public:
	MOCK_METHOD1(Open, bool(OpenMode));
	MOCK_METHOD0(Close, void());
	MOCK_METHOD0(isOpen, bool());
	MOCK_METHOD1(setPosition, void(uint64_t));
	MOCK_CONST_METHOD0(getPosition, uint64_t());
	MOCK_METHOD2(ReadData, uint32_t(ByteArray, uint32_t));
	MOCK_METHOD2(WriteData, uint32_t(ByteArray, uint32_t));
	MOCK_CONST_METHOD0(Size, uint64_t());
	//MOCK_METHOD1(setSize, void(uint64_t));




};