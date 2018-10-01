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
	MOCK_METHOD1(OpenWrite, IOErrorsType(const path_string &));
	MOCK_METHOD1(Create, IOErrorsType(const path_string &));
	MOCK_METHOD0(Close, void());
	MOCK_METHOD1(setPostion, void(uint64_t));
	MOCK_METHOD3(Read, IOErrorsType(ByteArray, const uint32_t, uint32_t &));
	MOCK_METHOD3(Write, IOErrorsType(ByteArray, const uint32_t, uint32_t &));
	MOCK_METHOD1(SetFileSize, IOErrorsType (uint64_t));
	MOCK_METHOD1(readFileSize, IOErrorsType(uint64_t));
};

