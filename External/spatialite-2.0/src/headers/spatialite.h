/* 
 spatialite.h -- Gaia spatial support for SQLite 
  
 version 1.0, 2008 May 6

 Author: Sandro Furieri a-furieri@lqt.it

 Copyright (C) 2008  Alessandro Furieri

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 
*/

#ifdef DLL_EXPORT
#define SPATIALITE_DECLARE __declspec(dllexport)
#else
#define SPATIALITE_DECLARE extern
#endif

#ifndef _SPATIALITE_H
#define _SPATIALITE_H

#ifdef __cplusplus
extern "C"
{
#endif

#define SPATIALITE_VERSION	"2.0"

    SPATIALITE_DECLARE const char *spatialite_version (void);
    SPATIALITE_DECLARE void spatialite_init (void);

#ifdef __cplusplus
}
#endif

#endif				/* _SPATIALITE_H */
