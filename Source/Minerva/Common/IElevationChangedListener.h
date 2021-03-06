
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Interface to notify of an elevation change.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_INTERFACES_IELEVATION_CHANGED_LISTENER_H__
#define __MINERVA_INTERFACES_IELEVATION_CHANGED_LISTENER_H__

#include "Usul/Interfaces/IUnknown.h"

#include "Minerva/Common/Extents.h"
#include "Minerva/Common/IElevationData.h"

namespace Minerva {
namespace Common {


struct IElevationChangedListener : public Usul::Interfaces::IUnknown
{
  /// Smart-pointer definitions.
  USUL_DECLARE_QUERY_POINTERS ( IElevationChangedListener );

  /// Id for this interface.
  enum { IID = 2421981627u };

  typedef Minerva::Common::Extents Extents;
  typedef Minerva::Common::IElevationData::QueryPtr ElevationDataPtr;

  /// Elevation has changed within given extents.
  virtual bool elevationChangedNotify ( const Extents& extents, unsigned int level, ElevationDataPtr elevationData, Usul::Interfaces::IUnknown * caller = 0x0 ) = 0;

}; // struct IElevationChangedListener


} // End namespace Interfaces
} // End namespace Minerva

#endif // __MINERVA_INTERFACES_IELEVATION_CHANGED_LISTENER_H__
