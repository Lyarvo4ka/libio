// PdfChecker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PdfChecker.h"
#include "libpdf\Pdfdocument.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "IO/Finder.h"
#include "IO/Analyzer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




CWinApp theApp;

using namespace std;

//bool pdfDateToNormal( const CString & pdfDateStr  , FILETIME * file_time )
//{
//	if ( pdfDateStr.IsEmpty() ) 
//		return false;
//
//
//}

//
////#include "..\ZipLib\ZipFile.h"



int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	//ZipFile zip_file;
	//auto zipPtr = ZipFile::Open("d:\\test_folder\\Анкета посетителя фабрики.zip");
	//zipPtr->GetEntriesCount() > 




	



	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{

		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
		

		CoInitialize( NULL );

		if (!AfxOleInit())	{
			AfxMessageBox(L"OLE initialization failed in BasicIacVc sample.");
			return FALSE;
		}

		COleException e;
		if ( ! isAcrobatInstalled( e ) )
		{
			printf( "Error to start acrobat app." );
			return -1;
		}
		if (argc == 3)
		{
			std::wstring source_folder(argv[1]);
			//verify_pdf_files(source_folder);

			std::wstring target_folder(argv[2]);

			identify_files(source_folder, target_folder);
		}
		else
		{
			printf("Error. You entered invalid params.\r\n");
			//IO::path_string str1, str2;
			//identify_pdf(str1, str2,0);
		}

		CoUninitialize();


		}
	}
	else
	{
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}
	
	return nRetCode;

}

std::wstring get_wstring_from_sz(const char* psz)
{
	int res;
	wchar_t buf[0x400];
	wchar_t *pbuf = buf;
	std::shared_ptr<wchar_t[]> shared_pbuf;

	res = MultiByteToWideChar(CP_ACP, 0, psz, -1, buf, sizeof(buf) / sizeof(wchar_t));

	if (0 == res && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		res = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);

		shared_pbuf = std::shared_ptr<wchar_t[]>(new wchar_t[res]);

		pbuf = shared_pbuf.get();

		res = MultiByteToWideChar(CP_ACP, 0, psz, -1, pbuf, res);
	}
	else if (0 == res)
	{
		pbuf = L"error";
	}

	return std::wstring(pbuf);
}

inline std::string get_string_from_wcs(const wchar_t* pcs)
{
	int res;
	char buf[0x400];
	char* pbuf = buf;
	std::shared_ptr<char[]> shared_pbuf;

	res = WideCharToMultiByte(CP_ACP, 0, pcs, -1, buf, sizeof(buf), NULL, NULL);

	if (0 == res && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		res = WideCharToMultiByte(CP_ACP, 0, pcs, -1, NULL, 0, NULL, NULL);

		shared_pbuf = std::shared_ptr<char[]>(new char[res]);

		pbuf = shared_pbuf.get();

		res = WideCharToMultiByte(CP_ACP, 0, pcs, -1, pbuf, res, NULL, NULL);
	}
	else if (0 == res)
	{
		pbuf = "error";
	}

	return std::string(pbuf);
}

bool open_pdf_file(const IO::path_string & file_name)
{
	COleException e;
	PdfDocument pdfDoc;
	if (pdfDoc.CreateDocument(e))
	{
		if (pdfDoc.Open(file_name))
			return true;
	}

	return false;
}

void verify_pdf_files(const IO::path_string & folder)
{
	using namespace IO;
	path_string add_bad_name = L".bad_file";
	path_string add_good_name = L".good";

	Finder finder;
	finder.add_extension(L".pdf");
	finder.FindFiles(folder);
	finder.printAll();

	auto listFiles = finder.getFiles();

	for (auto the_file : listFiles)
	{
		auto add_name = (open_pdf_file(the_file)) ? add_good_name : add_bad_name;

		try
		{
			fs::rename(the_file, the_file + add_name);
		}
		catch (const fs::filesystem_error& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
		}
	}

}

IO::path_string addFolderName(const IO::path_string & current_folder, const IO::path_string & new_folder)
{
	auto new_path(IO::addBackSlash(current_folder));
	new_path.append(new_folder);
	return new_path;

}

IO::path_string add_folder(const IO::path_string & current_folder, const IO::path_string & new_folder)
{
	auto target_folder(addFolderName(current_folder, new_folder));
	if (!fs::exists(target_folder))
		fs::create_directory(target_folder);

	return target_folder;
}
void identify_files(const IO::path_string & source_folder, IO::path_string & target_folder)
{
	IO::path_string bad_dir = L"bad";

	IO::Finder finder;
	finder.add_extension(L".pdf");
	finder.FindFiles(source_folder);
	auto file_list = finder.getFiles();

	DWORD counter = 0;

	for (auto cur_file: file_list)
	{	
		fs::path file_path(cur_file);
		auto ext = file_path.extension().generic_wstring();

		bool bResult = false;
		IO::path_string target_name;

		if (ext.compare(L".pdf") == 0)
			bResult = identify_pdf(cur_file, target_name, counter);
		//else
		//	if (isOffice2007(ext))
		//		bResult = identify_office2007(source_name, target_name, counter);
		//	else
		//		bResult = identify_office2003(source_name, target_name, counter);
		++counter;

		IO::path_string target_file_path;
		IO::path_string ext_folder = ext.substr(1);
		IO::path_string new_folder = (bResult) ? ext_folder : bad_dir;
		IO::path_string new_target_folder = add_folder(target_folder, new_folder);
		target_file_path = IO::addBackSlash(new_target_folder) + target_name;

		try
		{
			fs::rename(cur_file, target_file_path);
		}
		catch (const fs::filesystem_error& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
		}

	}
}



bool identify_pdf(const IO::path_string & file_name, IO::path_string & new_filename, int counter)
{
	COleException e;
	PdfDocument pdfDoc;

	fs::path file_path(file_name);
	auto ext = file_path.extension().generic_wstring();
	new_filename = IO::toNumberString(counter) + ext;

	if (pdfDoc.CreateDocument(e))
	{
		if (pdfDoc.Open(file_name))
		{ 
			auto docInfo = pdfDoc.getInfo();
			DateString data_string;
			CString targe_name = IO::toNumberString(counter).c_str();

			CString dataToParse = (!docInfo.ModDate.IsEmpty()) ? docInfo.ModDate : docInfo.CreationDate;
			if (!dataToParse.IsEmpty())
				if (ParseDateString(dataToParse, data_string))
				{
					targe_name = data_string.year + L"-" +
						data_string.month + L"-" +
						data_string.day + L"-" +
						data_string.hour + L"-" +
						data_string.min + L"-" +
						data_string.sec + L"-" +
						IO::toNumberString(counter).c_str();
				}
			new_filename = targe_name.GetString() + ext;

			pdfDoc.DestroyDocument();
			return true;
		}

	}
	else
		printf("Error to create pdf document application\r\n");

	return false;
}
