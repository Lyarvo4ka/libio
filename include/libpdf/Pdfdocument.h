#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include <string>

#include "libpdf.h"
#include "acrobat.h"
#include "iac.h"

#include <afxdisp.h>

#include "IO/Analyzer.h"
#include <memory>


BOOL LIBPDF_API isAcrobatInstalled( COleException &e );

const CString sAutor = L"Autor";
const CString sCreationDate = L"CreationDate";
const CString sCreator = L"Creator";
const CString sProducer = L"Producer";
const CString sTitle = L"Title";
const CString sSubject = L"Subject";
const CString sKeywords = L"Keywords";
const CString sModDate = L"ModDate";


struct LIBPDF_API DocInfo
{
	CString Author;
	CString CreationDate;
	CString Creator;
	CString Producer;
	CString Title;
	CString Subject;
	CString Keywords;
	CString ModDate;
};

using DateString = IO::DateType<CString>;

void LIBPDF_API PdfStringToTime( const CString & pdf_string , DateString & date_string );
bool LIBPDF_API ParseDateString( const CString & date_string , DateString & parsed_date );

class LIBPDF_API PdfDocument
{
public:
	PdfDocument( );
	~PdfDocument( ); 
	BOOL CreateDocument( COleException & e );
	void DestroyDocument();
	BOOL Open( const std::wstring & pdf_file );
	void Close();
	BOOL isOpened() const;
	BOOL isCreated() const;
	BOOL Save(const std::wstring & filePath);
	DocInfo getInfo( );

private:
	BOOL bOpened_ = FALSE;
	BOOL bCreated_ = FALSE;
	std::unique_ptr<CAcroPDDoc>  pAcroPdDoc_;
	//CAcroTime * m_pAcroTime;
};


class LIBPDF_API PDFAnalyzer
	: public IO::Analyzer
{
	PdfDocument pdfDoc_;

public:
	void analyze(const IO::path_string & filePath) override;
	bool test(const IO::path_string & filePath);
	bool open(const IO::path_string & filePath);
	void close();
	bool save(const IO::path_string & filePath);

};



#endif 