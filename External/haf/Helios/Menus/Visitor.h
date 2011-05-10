
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Derived from earlier work by Adam Kubach and Perry Miller found here:
//  http://sourceforge.net/projects/cadkit/
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Base class for menu visitors.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _HELIOS_MENUS_VISITOR_H_
#define _HELIOS_MENUS_VISITOR_H_

#include "Helios/Export.h"

#include "Usul/Base/Object.h"

#include "boost/noncopyable.hpp"

namespace Helios { namespace Menus { 
  class Button; class Menu; class Separator;
  class MenuBar; class ToolBar; class Item; } }


namespace Helios {
namespace Menus {


class HELIOS_EXPORT Visitor : 
  public Usul::Base::Object, 
  public boost::noncopyable
{
public:

  typedef Usul::Base::Object BaseClass;

  USUL_DECLARE_REF_POINTERS ( Visitor );

  // Call before using a second time.
  virtual void        reset(){}

  virtual void        visit ( Item & ){}
  virtual void        visit ( Button & ){}
  virtual void        visit ( Menu & );
  virtual void        visit ( MenuBar & );
  virtual void        visit ( Separator & ){}
  virtual void        visit ( ToolBar & );

protected:

  Visitor();
  virtual ~Visitor();
};


} // namespace Menus
} // namespace Helios


#endif // _HELIOS_MENUS_VISITOR_H_
