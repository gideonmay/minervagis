
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/GDAL/Common.h"

GlobalGDALMutex* GlobalGDALMutex::_instance ( 0x0 );

GlobalGDALMutex* GlobalGDALMutex::instance()
{
  if ( 0x0 == _instance )
    _instance = new GlobalGDALMutex;
  return _instance;
}

GlobalGDALMutex::GlobalGDALMutex() : _mutex()
{
}

GlobalGDALMutex::~GlobalGDALMutex()
{
}

Usul::Threads::Mutex& GlobalGDALMutex::mutex()
{
  return _mutex;
}
