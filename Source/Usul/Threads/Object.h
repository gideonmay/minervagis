
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Convenience class for guarding an object.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_THREADS_OBJECT_WITH_MUTEX_CLASS_H_
#define _USUL_THREADS_OBJECT_WITH_MUTEX_CLASS_H_

#include "Usul/Threads/Mutex.h"
#include "Usul/Threads/MutexTraits.h"


namespace Usul {
namespace Threads {


template < 
  class ValueType_, 
  class MutexTraits = Usul::Threads::MutexTraits<Usul::Threads::Mutex> 
  > 
struct Object
{
  /////////////////////////////////////////////////////////////////////////////
  //
  //  Typedefs
  //
  /////////////////////////////////////////////////////////////////////////////

  typedef ValueType_ ValueType;
  typedef ValueType value_type;
  typedef typename MutexTraits::MutexType MutexType;
  typedef typename MutexTraits::ReadLock ReadLock;
  typedef typename MutexTraits::WriteLock WriteLock;


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Constructors
  //
  /////////////////////////////////////////////////////////////////////////////

  Object() : _value(), _mutex()
  {
  }
  Object ( const ValueType &v ) : _value ( v ), _mutex()
  {
  }
  Object ( const Object &v ) : _value ( v._value ), _mutex()
  {
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Destructor
  //
  /////////////////////////////////////////////////////////////////////////////

  ~Object()
  {
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Get the mutex. Normal use does not require calling this function.
  //
  /////////////////////////////////////////////////////////////////////////////

  MutexType &mutex() const
  {
    return _mutex;
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Get a reference. Use with caution.
  //
  /////////////////////////////////////////////////////////////////////////////

  const ValueType &getReference() const
  {
    return _value;
  }
  ValueType &getReference()
  {
    return _value;
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Get a copy of the value.
  //
  /////////////////////////////////////////////////////////////////////////////

  ValueType getCopy() const
  {
    ReadLock lock ( _mutex );
    return _value;
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Set the value
  //
  /////////////////////////////////////////////////////////////////////////////

  void setValue ( const ValueType &v )
  {
    WriteLock lock ( _mutex );
    _value = v;
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Set the value
  //
  /////////////////////////////////////////////////////////////////////////////

  void setValue ( const Object &v )
  {
    this->setValue ( v.get() );
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Assignment
  //
  /////////////////////////////////////////////////////////////////////////////

  Object &operator = ( const Object &v )
  {
    this->setValue ( v );
    return *this;
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Assignment
  //
  /////////////////////////////////////////////////////////////////////////////

  Object &operator = ( const ValueType &v )
  {
    this->setValue ( v );
    return *this;
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  //  Typecast operator.
  //
  /////////////////////////////////////////////////////////////////////////////

  operator ValueType () const
  {
    return this->getCopy();
  }

private:

  ValueType _value;
  mutable MutexType _mutex;
};


} // namespace Threads
} // namespace Usul


#endif // _USUL_THREADS_OBJECT_WITH_MUTEX_CLASS_H_
