
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Reference-counting base class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_REFERENCED_BASE_CLASS_H_
#define _USUL_REFERENCED_BASE_CLASS_H_

#include "Usul/Strings/Format.h"

#include "Usul/Threads/Atomic.h"

#include <stdexcept>

namespace Usul { namespace Interfaces { struct IUnknown; } }


namespace Usul {
namespace Base {


class USUL_EXPORT Referenced
{
public:

  /// Reference and unreference the instance.
  void                        ref();
  void                        unref ( bool allowDeletion = true );

  /// Get the reference count.
  unsigned long               refCount() const;

  const std::type_info& typeId() const;

protected:

  explicit Referenced();
  Referenced ( const Referenced &r );
  Referenced &operator = ( const Referenced &r );
  virtual ~Referenced();

private:

  Usul::Threads::Atomic<unsigned long> _refCount;
};


} // namespace Base
} // namespace Usul


#endif // _USUL_REFERENCED_BASE_CLASS_H_
