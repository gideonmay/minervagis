
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Gary Huber for AlphaPixel and Iowa State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Gary Huber
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Defines what AOI_INTERSECTION_EXPORT means.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _AOI_INTERSECTION_EXPORT_H_
#define _AOI_INTERSECTION_EXPORT_H_

#ifdef _WIN32
# pragma warning ( disable : 4275 ) // Irrelevant VC6 warning.
# pragma warning ( disable : 4251 ) // Irrelevant VC6 warning.
# ifdef _COMPILING_AOI_INTERSECTION
#   define AOI_INTERSECTION_EXPORT __declspec ( dllexport ) // We are compiling this library so the classes are exported.
# else
#   define AOI_INTERSECTION_EXPORT __declspec ( dllimport ) // The classes will be imported into the client's code.
# endif
#else // _WIN32
# define AOI_INTERSECTION_EXPORT
#endif

#endif // _AOI_INTERSECTION_EXPORT_H_
