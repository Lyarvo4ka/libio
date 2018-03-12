#pragma once
#include <system_error>
#include "windows.h"
#include "constants.h"
//#include <string_view>

namespace IO
{

	namespace Error
	{
		const std::string OpenRead_str = "Error opening the file for reading.";
		const std::string OpenWrite_str = "Error opening file for writing.";
		const std::string Create_str = "Error creating file.";
		const std::string ReadData_str = "Error reading from file.";
		const std::string WriteData_str = "Error writing to file.";
		const std::string Unknown_str = "Unknown error";

		enum class DeviceErrors
		{
			kOpenRead = 1,
			kOpenWrite,
			kCreate,
			kReadData,
			kWriteData,
			kUnknown

		};

		inline DeviceErrors OpenModeToError(IO::OpenMode open_mode)
		{
			switch (open_mode)
			{
			case IO::OpenMode::OpenRead:
				return DeviceErrors::kOpenRead;
			case IO::OpenMode::OpenWrite:
				return DeviceErrors::kOpenWrite;
			case IO::OpenMode::Create:
				return DeviceErrors::kCreate;
			default:
				return DeviceErrors::kUnknown;
			}
		}

		static inline std::string getDiskOrFileError(const DeviceErrors error, const std::string & sourceName)
		{
			std::string resString;
			switch (error)
			{
			case DeviceErrors::kOpenRead:
				return "Error opening the " + sourceName + " for reading.";
			case DeviceErrors::kOpenWrite:
				return "Error opening " + sourceName + " for writing.";
			case DeviceErrors::kCreate:
				return "Error creating " + sourceName;
			case DeviceErrors::kReadData:
				return "Error reading from " + sourceName;
			case DeviceErrors::kWriteData:
				return "Error writing to " + sourceName;
			default:
				return Unknown_str;
			}
		}
	}
	template <class S>
	class SingletonHolder
		/// This is a helper template class for managing
		/// singleton objects allocated on the heap.
		/// The class ensures proper deletion (including
		/// calling of the destructor) of singleton objects
		/// when the application that created them terminates.
	{
	public:
		SingletonHolder() :
			_pS(0)
			/// Creates the SingletonHolder.
		{
		}

		~SingletonHolder()
			/// Destroys the SingletonHolder and the singleton
			/// object that it holds.
		{
			delete _pS;
		}

		S* get()
			/// Returns a pointer to the singleton object
			/// hold by the SingletonHolder. The first call
			/// to get will create the singleton.
		{
			//FastMutex::ScopedLock lock(_m);
			if (!_pS) _pS = new S;
			return _pS;
		}

	private:
		S* _pS;
		//FastMutex _m;
	};

	enum class ErrorLevel
	{
		none, // (turns off logging)
 	    fatal,
	    critical,
	    error,
	    warning,
	    notice,
	    information,
	    debug,
	    trace
	};

	constexpr ErrorLevel defaulErrorLevel()
	{
		return ErrorLevel::error;
	}



	class ErrorHandler
	{
	private: 
		static ErrorHandler * error_handler_;
	protected:
		static ErrorHandler* defaultHandler();
	public:
		static ErrorHandler * get()
		{
			return error_handler_;
		}
		void showMessage(const std::wstring & messageText)
		{
			wprintf(messageText.c_str());
			wprintf(L"\n");
		}
		void showMessage(const std::string & messageText)
		{
			printf(messageText.c_str());
			printf("\n");
		}


	};

	static void LOG_MESSAGE(const std::wstring & messageText)
	{
		ErrorHandler::get()->showMessage(messageText);
	}
	static void LOG_MESSAGE(const std::string & messageText)
	{
		ErrorHandler::get()->showMessage(messageText);
	}


	//static constexpr auto ErrorLevelStr = std::string_view::make_literal_array(
	//	"none",
	//	"fatal",
	//	"critical",
	//	"error",
	//	"warning",
	//	"notice",
	//	"information",
	//	"debug",
	//	"trace"

	//);

	/*
	class Error
	{
	private:
	uint32_t error_code_ = 0;
	path_string error_message_;
	ErrorLevel error_level_ = defaulErrorLevel();

	public:
	Error(uint32_t last_error)
	: error_code_(last_error)
	{
	error_message_ = getMessage(error_code_);
	}
	Error(const path_string & error_message)
	{
	error_message_ = error_message;
	}
	Error(const path_string & error_message , uint32_t last_error)
	: error_code_(last_error)
	{
	error_message_ = error_message;
	}
	void setErrorCode(uint32_t error_code)
	{
	error_code_ = error_code;
	}
	uint32_t getErrorCode() const
	{
	return error_code_;
	}
	path_string error_message() const
	{
	return error_message_;
	}

	uint32_t last() const
	{
	return ::GetLastError();
	}
	std::wstring Error::getMessage(uint32_t errorCode)
	{
	std::wstring errMsg;
	DWORD dwFlg = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

	LPWSTR lpMsgBuf = 0;
	if (FormatMessageW(dwFlg, 0, errorCode, 0, (LPWSTR)& lpMsgBuf, 0, NULL))
	errMsg = lpMsgBuf;

	//				UnicodeConverter::toUTF8(lpMsgBuf, errMsg);
	//#else
	//			LPTSTR lpMsgBuf = 0;
	//			if (FormatMessageA(dwFlg, 0, errorCode, 0, (LPTSTR)& lpMsgBuf, 0, NULL))
	//				errMsg = lpMsgBuf;
	//#endif
	LocalFree(lpMsgBuf);
	return errMsg;
	}
	};
	*/



}