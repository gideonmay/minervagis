
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __GDAL_COMMON_H__
#define __GDAL_COMMON_H__

#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_api.h"
#include "ogrsf_frmts.h"
#include "cpl_config.h"
#include "cpl_error.h"

#include "Usul/Threads/Guard.h"
#include "Usul/Threads/Mutex.h"

#include <iostream>
#include <stdexcept>

namespace Minerva {

///////////////////////////////////////////////////////////////////////////////
//
//  Error handler.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  static void errorHandler ( CPLErr errorType, int, const char* text )
  {
    if ( CE_Fatal == errorType )
      throw std::runtime_error ( text );
    else
      std::cout << text << std::endl;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Push/Pop error handler.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  struct PushPopErrorHandler
  {
    PushPopErrorHandler()
    {
      CPLPushErrorHandler( (CPLErrorHandler) Detail::errorHandler );
    }
    ~PushPopErrorHandler()
    {
      CPLPopErrorHandler();
    }
  };
}

}

class GlobalGDALMutex
{
public:

  static GlobalGDALMutex* instance();

  Usul::Threads::Mutex& mutex();

private:

  GlobalGDALMutex();
  ~GlobalGDALMutex();

  static GlobalGDALMutex *_instance;

  Usul::Threads::Mutex _mutex;
};

#if !defined WIN32 && !defined CPL_MULTIPROC_PTHREAD
  #define SCOPED_GDAL_LOCK Usul::Threads::Guard<Usul::Threads::Mutex> __guard__ ( GlobalGDALMutex::instance() )
#else
  #define SCOPED_GDAL_LOCK
#endif


#endif // __GDAL_COMMON_H__
