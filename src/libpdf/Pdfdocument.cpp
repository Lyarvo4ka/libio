
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
	: m_pAcroPdDoc( nullptr )
{

}

PdfDocument::~PdfDocument()
{
	DestroyDocument();
}

BOOL PdfDocument::CreateDocument( COleException & e )
{
	m_pAcroPdDoc = new CAcroPDDoc( );
	return m_pAcroPdDoc->CreateDispatch( L"AcroExch.PDDoc", &e );
}

void PdfDocument::DestroyDocument()
{
	this->Close();
	if ( m_pAcroPdDoc )
	{
		m_pAcroPdDoc->ReleaseDispatch();
		delete m_pAcroPdDoc;
		m_pAcroPdDoc = nullptr;
	}
}

BOOL PdfDocument::Open( const std::wstring & pdf_file )
{
	COleException ex;
	if (!m_pAcroPdDoc)
		if (BOOL bOpened = CreateDocument(ex); bOpened == FALSE)
			return FALSE;

	if( m_pAcroPdDoc->Open( pdf_file.c_str() ) == PDF_OK ) {
		return TRUE;
	}
	return FALSE;
}

void PdfDocument::Close()
{
	if ( m_pAcroPdDoc )
		m_pAcroPdDoc->Close();
}

BOOL PdfDocument::Save(const std::wstring & filePath)
{
	if (m_pAcroPdDoc->Save(PDSaveFull, filePath.c_str()) == PDF_OK)
		return TRUE;
	return FALSE;
}

DocInfo PdfDocument::getInfo( )
{
	DocInfo docInfo;
	if ( m_pAcroPdDoc )
	{
		docInfo.Author = m_pAcroPdDoc->GetInfo( sAutor );
		docInfo.CreationDate = m_pAcroPdDoc->GetInfo( sCreationDate );
		docInfo.Creator = m_pAcroPdDoc->GetInfo( sCreator );
		docInfo.Producer = m_pAcroPdDoc->GetInfo( sProducer );
		docInfo.Title = m_pAcroPdDoc->GetInfo( sTitle );
		docInfo.Subject = m_pAcroPdDoc->GetInfo( sSubject );
		docInfo.Keywords = m_pAcroPdDoc->GetInfo( sKeywords );
		docInfo.ModDate = m_pAcroPdDoc->GetInfo( sModDate );
	}
	return docInfo;
}


void PDFAnalyzer::analyze(const IO::path_string & filePath)
{
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

bool PDFAnalyzer::Open(const IO::path_string & filePath)
{
	COleException e;

	if (pdfDoc_.CreateDocument(e))
		if (pdfDoc_.Open(filePath))
			return true;


	return false;
}

void PDFAnalyzer::Close()
{
	pdfDoc_.DestroyDocument();
}

bool PDFAnalyzer::Save(const IO::path_string & filePath)
{


	return false;
}
