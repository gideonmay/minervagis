
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Perry L Miller IV
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Helper class for creating images.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Qt/Tools/Image.h"

#include "QtGui/QAbstractButton"
#include "QtGui/QAction"
#include "QtGui/QIcon"
#include "QtGui/QLabel"
#include "QtGui/QSplashScreen"
#include "QtGui/QWidget"

using namespace Minerva::QtTools;


///////////////////////////////////////////////////////////////////////////////
//
//  Helper struct to set the icon.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  template < class T > struct SetIcon;

  template <> struct SetIcon < QAbstractButton >
  {
    static void set ( QAbstractButton *b, QIcon icon )
    {
      if ( 0x0 != b )
      {
        b->setIcon ( icon );
      }
    }
  };

  template <> struct SetIcon < QWidget >
  {
    static void set ( QWidget *w, QIcon icon )
    {
      if ( 0x0 != w )
      {
        w->setWindowIcon ( icon );
      }
    }
  };

  template <> struct SetIcon < QAction >
  {
    static void set ( QAction *a, QIcon icon )
    {
      if ( 0x0 != a )
      {
        a->setIcon ( icon );
      }
    }
  };
}


///////////////////////////////////////////////////////////////////////////////
//
//  Helper function to set the icon.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  template < class T > void set ( const std::string &name, T *t )
  {
    if ( 0x0 == t )
    {
      return;
    }

    QIcon icon ( QString ( std::string ( ":/Images/" + name ).c_str() ) );
    if ( icon.isNull() )
    {
      return;
    }

    Detail::SetIcon<T>::set ( t, icon );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the icon.
//
///////////////////////////////////////////////////////////////////////////////

void Image::icon ( const std::string &name, QAbstractButton *button )
{
  Detail::set ( name, button );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the icon.
//
///////////////////////////////////////////////////////////////////////////////

void Image::icon ( const std::string &name, QWidget *widget )
{
  Detail::set ( name, widget );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the icon.
//
///////////////////////////////////////////////////////////////////////////////

void Image::icon ( const std::string &name, QAction *action )
{
  Detail::set ( name, action );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the pixmap.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  template < class T > void pixmap ( const std::string &name, T *t )
  {
    if ( 0x0 == t )
    {
      return;
    }

    QPixmap p ( QString ( std::string ( ":/Images/" + name ).c_str() ) );

    // If we have an image then set it.
    if ( ( false == p.isNull() ) && ( 0x0 != t ) )
    {
      t->setPixmap ( p );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the pixmap.
//
///////////////////////////////////////////////////////////////////////////////

void Image::pixmap ( const std::string &name, QLabel *label )
{
  Detail::pixmap ( name, label );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the pixmap.
//
///////////////////////////////////////////////////////////////////////////////

void Image::pixmap ( const std::string &name, QSplashScreen *splash )
{
  Detail::pixmap ( name, splash );
}
