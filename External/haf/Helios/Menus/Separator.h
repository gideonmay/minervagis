
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
//  Class that represents a menu separator.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _HELIOS_MENUS_SEPARATOR_H_
#define _HELIOS_MENUS_SEPARATOR_H_

#include "Helios/Menus/Item.h"


namespace Helios {
namespace Menus {


class HELIOS_EXPORT Separator : public Helios::Menus::Item
{
public:

  typedef Helios::Menus::Item BaseClass;

  USUL_DECLARE_REF_POINTERS ( Separator );

  Separator();

  virtual void      accept ( Visitor &v ) { v.visit ( *this ); }

protected:

  virtual ~Separator();
};


} // namespace Menus
} // namespace Helios


#endif // _HELIOS_MENUS_SEPARATOR_H_
