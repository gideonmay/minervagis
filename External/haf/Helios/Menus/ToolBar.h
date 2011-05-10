
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
//  Class that represents a toolbar.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _HELIOS_MENUS_TOOLBAR_H_
#define _HELIOS_MENUS_TOOLBAR_H_

#include "Helios/Menus/Button.h"

#include <vector>


namespace Helios {
namespace Menus {


class HELIOS_EXPORT ToolBar : public Helios::Menus::Item
{
public:

  typedef Helios::Menus::Item BaseClass;
  typedef std::vector < Button::RefPtr > ButtonList;
  typedef ButtonList::size_type Position;

  USUL_DECLARE_REF_POINTERS ( ToolBar );

  ToolBar ( const std::string &name );

  virtual void      accept ( Visitor &v ) { v.visit ( *this ); }
  void              append ( Button::RefPtr b ) { this->insert ( -1, b ); }

  bool              empty() const;

  Position          find ( Button::RefPtr ) const;
  Position          find ( const std::string &text ) const;

  void              insert ( Position, Button::RefPtr );

  bool              needToRebuild() const;
  void              needToRebuild ( bool state );
  bool              needToUpdate() const;
  void              needToUpdate ( bool state );

  void              prepend ( Button::RefPtr b ) { this->insert ( 0, b ); }

  unsigned int      size() const;

  void              remove ( Position );
  void              remove ( Button::RefPtr b )        { this->remove ( this->find ( b ) ); }
  void              remove ( const std::string &text ) { this->remove ( this->find ( text ) ); }

  void              visitItems ( Visitor & );

protected:

  virtual ~ToolBar();

private:

  ButtonList _buttons;
  bool _needToRebuild;
  bool _needToUpdate;
};


} // namespace Menus
} // namespace Helios


#endif // _HELIOS_MENUS_TOOLBAR_H_
