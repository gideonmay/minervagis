
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2004, Perry L. Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Work-space container.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _GENERIC_NURBS_LIBRARY_WORK_CONTAINER_H_
#define _GENERIC_NURBS_LIBRARY_WORK_CONTAINER_H_

#include "GN/Macros/ErrorCheck.h"


namespace GN {
namespace Work {


template < class ContainerType_, class ErrorChecker_ > class Container
{
public:

  /////////////////////////////////////////////////////////////////////////////
  ///
  /// Useful typedefs.
  ///
  /////////////////////////////////////////////////////////////////////////////

  typedef ContainerType_                                ContainerType;
  typedef ErrorChecker_                                 ErrorCheckerType;
  typedef typename ContainerType::value_type            value_type;
  typedef typename ContainerType::size_type             size_type;
  typedef typename ContainerType::difference_type       difference_type;
  typedef typename ContainerType::reference             reference;
  typedef typename ContainerType::const_reference       const_reference;
  typedef Container < ContainerType, ErrorCheckerType > ThisType;


  /////////////////////////////////////////////////////////////////////////////
  ///
  /// Default constructor.
  ///
  /////////////////////////////////////////////////////////////////////////////

  Container() : _c()
  {
  }


  /////////////////////////////////////////////////////////////////////////////
  ///
  /// Copy constructor.
  ///
  /////////////////////////////////////////////////////////////////////////////

  Container ( const Container &c ) : _c ( c._c )
  {
  }


  /////////////////////////////////////////////////////////////////////////////
  ///
  /// Destructor.
  ///
  /////////////////////////////////////////////////////////////////////////////

  ~Container()
  {
  }


  /////////////////////////////////////////////////////////////////////////////
  ///
  /// Set the values of this container.
  ///
  /////////////////////////////////////////////////////////////////////////////

  void set ( const ThisType &c )
  {
    // Handle self.
    if ( &c != this )
      _c = c._c;
  }


  /////////////////////////////////////////////////////////////////////////////
  ///
  /// Assignment.
  ///
  /////////////////////////////////////////////////////////////////////////////

  Container &operator = ( const ThisType &c )
  {
    this->set ( c );
    return *this;
  }


  /////////////////////////////////////////////////////////////////////////////
  ///
  /// Resize the container.
  ///
  /////////////////////////////////////////////////////////////////////////////

  void resize ( size_type size )
  {
    _c.resize ( size );
  }


  /////////////////////////////////////////////////////////////////////////////
  ///
  /// Is it empty?
  ///
  /////////////////////////////////////////////////////////////////////////////

  bool empty() const
  {
    return _c.empty();
  }


  /////////////////////////////////////////////////////////////////////////////
  ///
  /// Get the size.
  ///
  /////////////////////////////////////////////////////////////////////////////

  size_type size() const
  {
    return _c.size();
  }


  /////////////////////////////////////////////////////////////////////////////
  ///
  /// Make sure the container can accommodate the size.
  ///
  /////////////////////////////////////////////////////////////////////////////

  void accommodate ( size_type s )
  {
    if ( s > this->size() )
      this->resize ( s );
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Random access.
  //
  /////////////////////////////////////////////////////////////////////////////

  reference operator [] ( size_type i )
  {
    GN_ERROR_CHECK ( i < this->size() );
    return _c[i];
  }
  const_reference operator [] ( size_type i ) const
  {
    GN_ERROR_CHECK ( i < this->size() );
    return _c[i];
  }
  
private:


  /////////////////////////////////////////////////////////////////////////////
  ///
  /// Data members.
  ///
  /////////////////////////////////////////////////////////////////////////////

  ContainerType _c;
};


}; // namespace Work
}; // namespace GN


#endif // _GENERIC_NURBS_LIBRARY_WORK_CONTAINER_H_
