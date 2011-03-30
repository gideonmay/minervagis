
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
//  Class that represents a button.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _HELIOS_MENUS_BUTTON_H_
#define _HELIOS_MENUS_BUTTON_H_

#include "Helios/Menus/Item.h"
#include "Helios/Menus/Action.h"

#include <string>


namespace Helios {
namespace Menus {


class HELIOS_EXPORT Button : public Helios::Menus::Item
{
public:

  // Useful typedefs.
  typedef Helios::Menus::Item BaseClass;
  typedef Action::Command Command;
  typedef Action::Update Update;

  // Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( Button );

  // Constructor
  Button ( const std::string &text, 
           const std::string &icon,
           const std::string &status,
           Command command,
           Update update = Update() ) :
    BaseClass ( text ),
    _icon ( icon ),
    _status ( status ),
    _command ( command ),
    _update ( update )
  {
  }

  // Constructor
  template < class Function >
  Button ( const std::string &text, 
           const std::string &icon,
           const std::string &status,
           Function function ) :
    BaseClass ( text ),
    _icon ( icon ),
    _status ( status ),
    _command ( function ),
    _update()
  {
  }

  // Constructor
  template < class Function, class Update >
  Button ( const std::string &text, 
           const std::string &icon,
           const std::string &status,
           Function function,
           Update update ) :
    BaseClass ( text ),
    _icon ( icon ),
    _status ( status ),
    _command ( function ),
    _update ( update )
  {
  }

  virtual void      accept ( Visitor &v ) { v.visit ( *this ); }

  Command           commandGet() const;

  std::string       iconGet() const;

  std::string       statusGet() const;

  std::string       textGet() const;

  Update            updateGet() const;

protected:

  // Destructor
  virtual ~Button();

private:

  std::string _icon;
  std::string _status;
  Command _command;
  Update _update;
};


} // namespace Menus
} // namespace Helios


#endif // _HELIOS_MENUS_BUTTON_H_
