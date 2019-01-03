
#include "stdafx.h"
#include "Pdfdocument.h"
//#include <boost/lexical_cast.hpp>


#define PDF_OK -1

BOOL isAcrobatInstalled(COleException & e)
{
	PdfDocument pdfDoc;
	return pdfDoc.CreateDocument( e );
}

void PdfStringToTime( const CString & pdf_string , DateString & date_string )
{
	date_string.year = pdf_string.Mid(0,4);
	date_string.month = pdf_string.Mid(4,2);
	date_string.day = pdf_string.Mid(6,2);
	date_string.hour = pdf_string.Mid(8,2);
	date_string.min = pdf_string.Mid(10,2);
	date_string.sec = pdf_string.Mid(12,2);
}

bool ParseDateString( const CString & date_string , DateString & parsed_date)
{
	if ( date_string.IsEmpty() )
	{
		return false;
	}

	const int date_length = 14;
	const int D_size = 2;
	const int full_length = date_length + D_size;
	const wchar_t * D_str = L"D:";

	CString str_date;
	int str_len = date_string.GetLength();
	if ( date_string.GetLength() >= full_length )
	{
		CString d_str = date_string.Left(D_size);
		if ( d_str.Compare( D_str ) == 0 )
			str_date = date_string.Mid( D_size , date_length );
		else
			return false;
	}
	else
		str_date = date_string;

	PdfStringToTime( str_date , parsed_date );

	return true;

}


PdfDocument::PdfDocument( )
{

}

PdfDocument::~PdfDocument()
{
	this->Close();
	DestroyDocument();
}

BOOL PdfDocument::CreateDocument( COleException & e )
{
	if (isCreated())
		DestroyDocument();

	pAcroPdDoc_ = std::make_unique< CAcroPDDoc >( );
	bCreated_ = pAcroPdDoc_->CreateDispatch( L"AcroExch.PDDoc", &e );
	return bCreated_;
}

void PdfDocument::DestroyDocument()
{
	if (pAcroPdDoc_)
		pAcroPdDoc_->ReleaseDispatch();
}

BOOL PdfDocument::Open( const std::wstring & pdf_file )
{
	COleException ex;
	if (!isCreated())
	{
		if (!CreateDocument(ex) )
			return FALSE;
	}

	if( pAcroPdDoc_->Open( pdf_file.c_str() ) == PDF_OK ) {
		return TRUE;
	}
	return FALSE;
}

void PdfDocument::Close()
{
	if (isOpened)
		pAcroPdDoc_->Close();
}

BOOL PdfDocument::isOpened() const
{
	return bOpened_;
}

BOOL PdfDocument::isCreated() const
{
	return bCreated_;
}

BOOL PdfDocument::Save(const std::wstring & filePath)
{
	if (isOpened)
	if (pAcroPdDoc_->Save(PDSaveFull, filePath.c_str()) == PDF_OK)
		return TRUE;
	return FALSE;
}

DocInfo PdfDocument::getInfo( )
{
	DocInfo docInfo;
	if ( pAcroPdDoc_ )
	{
		docInfo.Author = pAcroPdDoc_->GetInfo( sAutor );
		docInfo.CreationDate = pAcroPdDoc_->GetInfo( sCreationDate );
		docInfo.Creator = pAcroPdDoc_->GetInfo( sCreator );
		docInfo.Producer = pAcroPdDoc_->GetInfo( sProducer );
		docInfo.Title = pAcroPdDoc_->GetInfo( sTitle );
		docInfo.Subject = pAcroPdDoc_->GetInfo( sSubject );
		docInfo.Keywords = pAcroPdDoc_->GetInfo( sKeywords );
		docInfo.ModDate = pAcroPdDoc_->GetInfo( sModDate );
	}
	return docInfo;
}


void PDFAnalyzer::analyze(const IO::path_string & filePath)
{
	COleException ex;
	if (pdfDoc_.CreateDocument(ex))
		if (pdfDoc_.Open(filePath))
		{
			// 1 Save to temp file to detect file coruption
			// 2. try to get datetime
		}

	

	/*
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


	*/
}
bool PDFAnalyzer::test(const IO::path_string & filePath)
{

}

bool PDFAnalyzer::open(const IO::path_string & filePath)
{
	COleException e;

	if (pdfDoc_.CreateDocument(e))
		if (pdfDoc_.Open(filePath))
			return true;


	return false;
}

void PDFAnalyzer::close()
{
	pdfDoc_.DestroyDocument();
}

bool PDFAnalyzer::save(const IO::path_string & filePath)
{


	return false;
}
