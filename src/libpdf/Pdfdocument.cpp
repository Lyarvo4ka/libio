
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
	date_string.YEAR = pdf_string.Mid(0,4);
	date_string.MONTH = pdf_string.Mid(4,2);
	date_string.DAY = pdf_string.Mid(6,2);
	date_string.HOUR = pdf_string.Mid(8,2);
	date_string.MINUTES = pdf_string.Mid(10,2);
	date_string.SECONDS = pdf_string.Mid(12,2);
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

