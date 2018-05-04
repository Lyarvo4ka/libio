#include "gtest/gtest.h"

#include "pch.h"

#include "IO\zbk_medoc.h"

TEST(ZbkTest, testValidDay)
{
	IO::zbk_DateTime zbkDate = IO::zbk_DateTime();
	zbkDate.day = 32;
	EXPECT_FALSE(zbkDate.isValidDay());
	zbkDate.day = 0;
	EXPECT_FALSE(zbkDate.isValidDay());
	zbkDate.day = 24;
	EXPECT_TRUE(zbkDate.isValidDay());
}
TEST(ZbkTest, testValidMonth)
{
	IO::zbk_DateTime zbkDate = IO::zbk_DateTime();
	zbkDate.month = 13;
	EXPECT_FALSE(zbkDate.isValidMounth());
	zbkDate.month = 0;
	EXPECT_FALSE(zbkDate.isValidMounth());
	zbkDate.month = 4;
	EXPECT_TRUE(zbkDate.isValidMounth());

}
TEST(ZbkTest, testValidYear)
{
	IO::zbk_DateTime zbkDate = IO::zbk_DateTime();
	zbkDate.year = 111;
	EXPECT_FALSE(zbkDate.isValidYear());
	zbkDate.year = 11111;
	EXPECT_TRUE(zbkDate.isValidYear());
	zbkDate.year = 1986;
	EXPECT_TRUE(zbkDate.isValidYear());

}

TEST(ZbkTest, testValidHour)
{
	IO::zbk_DateTime zbkDate = IO::zbk_DateTime();
	zbkDate.hour = 0;
	EXPECT_TRUE(zbkDate.isValidHour());
	zbkDate.hour = 24;
	EXPECT_FALSE(zbkDate.isValidHour());
	zbkDate.hour = 15;
	EXPECT_TRUE(zbkDate.isValidHour());
}

TEST(ZbkTest, testValidSeconds)
{
	IO::zbk_DateTime zbkDate = IO::zbk_DateTime();
	zbkDate.sec = 0;
	EXPECT_TRUE(zbkDate.isValideSeconds());

	zbkDate.sec = 60;
	EXPECT_FALSE(zbkDate.isValideSeconds());
	zbkDate.sec = 59;
	EXPECT_TRUE(zbkDate.isValideSeconds());

}

TEST(ZbkTest, testIsValid)
{
	IO::zbk_DateTime badDateDay = { 32,4,1986,15,00 };
	EXPECT_FALSE(badDateDay.isVaid());

	IO::zbk_DateTime badDateMonth = { 24,13,1986,15,00 };
	EXPECT_FALSE(badDateDay.isVaid());

	IO::zbk_DateTime badDateYear = { 24,4,1111,15,00 };
	EXPECT_FALSE(badDateYear.isVaid());

	IO::zbk_DateTime badDateHour = { 24,4,1986,25,00 };
	EXPECT_FALSE(badDateHour.isVaid());

	IO::zbk_DateTime badDateSec = { 24,4,1986,18,61 };
	EXPECT_FALSE(badDateSec.isVaid());

	IO::zbk_DateTime goodDate = { 24,4,1986,15,00 };
	EXPECT_TRUE(goodDate.isVaid());


}
