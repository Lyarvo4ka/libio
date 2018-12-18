// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBPDF_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LIBPDF_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LIBPDF_EXPORTS
#define LIBPDF_API __declspec(dllexport)
#else
#define LIBPDF_API __declspec(dllimport)
#endif

// This class is exported from the libpdf.dll
class LIBPDF_API Clibpdf {
public:
	Clibpdf(void);
	// TODO: add your methods here.
};

extern LIBPDF_API int nlibpdf;

LIBPDF_API int fnlibpdf(void);
