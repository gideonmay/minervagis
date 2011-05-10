
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Functors for use with buttons.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _HELIOS_MENUS_FUNCTORS_H_
#define _HELIOS_MENUS_FUNCTORS_H_

#include "Usul/Config/Config.h"

#include "QtGui/QAction"


namespace Helios {
namespace Menus {
namespace Functors {


template < class F > struct UpdateCheckState
{
  UpdateCheckState ( F f ) : _f ( f ){}
  UpdateCheckState ( const UpdateCheckState &u ) : _f ( u._f ){}
  void operator () ( QAction &action )
  {
    action.setCheckable ( true );
    action.setChecked ( _f() );
  }
private:
  UpdateCheckState &operator = ( const UpdateCheckState & );
  F _f;
};
template < class F > UpdateCheckState<F> updateCheckState ( F f )
{
  return UpdateCheckState<F> ( f );
}


} // namespace Functors
} // namespace Menus
} // namespace Helios


#endif // _HELIOS_MENUS_FUNCTORS_H_
