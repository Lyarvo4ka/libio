#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(JSONREADER_LIB)
#  define JSONREADER_EXPORT Q_DECL_EXPORT
# else
#  define JSONREADER_EXPORT Q_DECL_IMPORT
# endif
#else
# define JSONREADER_EXPORT
#endif
