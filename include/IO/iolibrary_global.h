#ifndef IOLIBRARY_GLOBAL_H
#define IOLIBRARY_GLOBAL_H

#define ImportDll   __declspec( dllimport )
#define ExportDll   __declspec( dllexport )

#ifdef IOLIBRARY_EXPORTS
# define IOLIBRARY_EXPORT ExportDll
#else
# define IOLIBRARY_EXPORT ImportDll
#endif

#endif	// IOLIBRARY_GLOBAL_H