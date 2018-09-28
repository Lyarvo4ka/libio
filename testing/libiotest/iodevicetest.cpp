#include "gtest/gtest.h"

#include "pch.h"
#include "IO/IODevice.h"


TEST(ioengineTest, testConstexptrDeviceTypeToString)
{
	auto expected_file = IO::DeviceTypeToString(IO::DeviceType::kFile);
}