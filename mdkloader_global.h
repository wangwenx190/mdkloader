#pragma once

#include <QtCore/qglobal.h>

#ifndef MDKLOADER_EXPORT
#ifdef MDKLOADER_STATIC
#define MDKLOADER_EXPORT
#else
#ifdef MDKLOADER_BUILD_LIBRARY
#define MDKLOADER_EXPORT Q_DECL_EXPORT
#else
#define MDKLOADER_EXPORT Q_DECL_IMPORT
#endif
#endif
#endif
