
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Convenient base class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_BASE_OBJECT_CLASS_H_
#define _USUL_BASE_OBJECT_CLASS_H_

#include "Usul/Base/Referenced.h"
#include "Usul/Pointers/Pointers.h"
#include "Usul/Threads/RecursiveMutex.h"
#include "Usul/Threads/Guard.h"

#include <string>


namespace Usul {
namespace Base {


class USUL_EXPORT Object : public Usul::Base::Referenced
{
public:

  // Typedefs.
  typedef Usul::Base::Referenced BaseClass;
  typedef Usul::Threads::RecursiveMutex Mutex;
  typedef Usul::Threads::Guard<Mutex> Guard;

  // Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( Object );

  // Get the mutex.
  Mutex &                   mutex() const;

  // Set/get the name.
  std::string               name() const;
  void                      name ( const std::string &s );

protected:

  // Construction.
  Object();
  Object ( const std::string& name );

  // Copy construction.
  Object ( const Object & );

  // No assignment.
  Object &operator = ( const Object & );

  // Use reference counting.
  virtual ~Object();

  // Reference to the name. Use wisely.
  std::string &             _getName() { return _name; }

private:

  void                      _destroy();

  std::string _name;
  mutable Mutex *_mutex;
};


} // namespace Base
} // namespace Usul


#endif //_USUL_BASE_OBJECT_CLASS_H_
