#pragma once
#include <string>

class Logger
{
public:
	virtual ~Logger() {};

	virtual void showMessage(const std::wstring & messageText) = 0;
};

class ConsoleLogger
	: public Logger
{
public:
	void showMessage(const std::wstring & messageText) override
	{
		wprintf(messageText.c_str());
		wprintf(L"\n");
	}
};
