
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Defines what MINERVA_COMMON_EXPORT means.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MINERVA_COMMON_EXPORT_H_
#define _MINERVA_COMMON_EXPORT_H_

#ifdef _WIN32
# pragma warning ( disable : 4275 ) // Irrelevant VC6 warning.
# pragma warning ( disable : 4251 ) // Irrelevant VC6 warning.
# ifdef _COMPILING_MINERVA_COMMON
#   define MINERVA_COMMON_EXPORT __declspec ( dllexport ) // We are compiling this library so the classes are exported.
# else
#   define MINERVA_COMMON_EXPORT __declspec ( dllimport ) // The classes will be imported into the client's code.
# endif
#else // _WIN32
# define MINERVA_COMMON_EXPORT
#endif

#endif // _MINERVA_COMMON_EXPORT_H_
