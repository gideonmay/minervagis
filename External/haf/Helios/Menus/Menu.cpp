
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
//  Class that represents a menu.
//
///////////////////////////////////////////////////////////////////////////////

#include "Helios/Menus/Menu.h"

#include "Usul/Functions/SafeCall.h"

#include <algorithm>
#include <vector>

using namespace Helios::Menus;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor
//
///////////////////////////////////////////////////////////////////////////////

Menu::Menu ( const std::string &text ) : BaseClass ( text ), 
  _items()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor
//
///////////////////////////////////////////////////////////////////////////////

Menu::~Menu()
{
  USUL_TRY_BLOCK
  {
    _items.clear();
  }
  USUL_DEFINE_CATCH_BLOCKS ( "3932752002" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find the item.
//
///////////////////////////////////////////////////////////////////////////////

Menu::Position Menu::find ( Item::RefPtr item ) const
{
  Guard guard ( this );
  Items::const_iterator i ( std::find ( _items.begin(), _items.end(), item ) );
  return ( ( _items.end() == i ) ? -1 : std::distance ( _items.begin(), i ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Helper predicate for finding the item with given text.
//
///////////////////////////////////////////////////////////////////////////////

namespace Helper
{
  struct HasText
  {
    HasText ( const std::string &text ) : _text ( text ){}
    bool operator () ( const Helios::Menus::Item::RefPtr &item )
    {
      return ( ( true == item.valid() ) ? ( item->text() == _text ) : false );
    }
  private:
    const std::string _text;
  };
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find the item.
//
///////////////////////////////////////////////////////////////////////////////

Menu::Position Menu::find ( const std::string &text ) const
{
  Guard guard ( this );
  Items::const_iterator i ( std::find_if 
    ( _items.begin(), _items.end(), Helper::HasText ( text ) ) );
  return ( ( _items.end() == i ) ? -1 : std::distance ( _items.begin(), i ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the item.
//
///////////////////////////////////////////////////////////////////////////////

Item::RefPtr Menu::at ( Position p )
{
  Guard guard ( this );
  return ( ( ( p >= 0 ) && ( p < _items.size() ) ) ? _items.at ( p ) : Item::RefPtr ( 0x0 ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find or create menu.
//
///////////////////////////////////////////////////////////////////////////////

Menu::RefPtr Menu::menu ( const std::string &text, Position p )
{
  Guard guard ( this );

  // Look for it.
  Item::RefPtr item ( this->at ( this->find ( text ) ) );
  Menu::RefPtr m ( dynamic_cast < Menu * > ( item.get() ) );
  if ( true == m.valid() )
    return m;

  // Make a new one and insert it.
  m = Menu::RefPtr ( new Menu ( text ) );
  this->insert ( p, m.get() );
  return m;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove the item.
//
///////////////////////////////////////////////////////////////////////////////

void Menu::remove ( Position i )
{
  Guard guard ( this );

  if ( ( i < 0 ) || ( i >= _items.size() ) )
    return;

  _items.erase ( _items.begin() + i );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear all the items.
//
///////////////////////////////////////////////////////////////////////////////

void Menu::clear()
{
  Guard guard ( this );
  _items.clear();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Insert the item.
//
///////////////////////////////////////////////////////////////////////////////

void Menu::insert ( Position i, Item::RefPtr item )
{
  Guard guard ( this );

  if ( false == item.valid() )
    return;

  if ( ( -1 == i ) || ( i > _items.size() ) )
    i = _items.size();

  _items.insert ( _items.begin() + i, item );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Visit the items.
//
///////////////////////////////////////////////////////////////////////////////

void Menu::visitItems ( Visitor &v )
{
  Guard guard ( this );
  for ( Items::iterator i = _items.begin(); i != _items.end(); ++i )
  {
    Item::RefPtr item ( *i );
    if ( true == item.valid() )
    {
      item->accept ( v );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Is the menu empty?
//
///////////////////////////////////////////////////////////////////////////////

bool Menu::empty() const
{
  Guard guard ( this );
  return _items.empty();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove empty menus.
//
///////////////////////////////////////////////////////////////////////////////

void Menu::purge()
{
  Guard guard ( this );

  Items items;
  items.reserve ( _items.size() );

  for ( Items::iterator i = _items.begin(); i != _items.end(); ++i )
  {
    Item::RefPtr item ( *i );
    if ( true == item.valid() )
    {
      Menu::RefPtr m ( dynamic_cast < Menu * > ( item.get() ) );
      if ( true == m.valid() )
      {
        m->purge();
        if ( false == m->empty() )
        {
          items.push_back ( m.get() );
        }
      }
      else
      {
        items.push_back ( item );
      }
    }
  }

  _items = items;
}
