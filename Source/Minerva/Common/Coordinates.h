
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class to hold lon,lat,height coordinates.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_COMMON_COORDINATES_H__
#define __MINERVA_COMMON_COORDINATES_H__

#include "Minerva/Common/Export.h"
#include "Minerva/Common/Extents.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Math/Vector3.h"
#include "Usul/Pointers/Pointers.h"

#include <vector>

namespace Minerva {
namespace Common {


class MINERVA_COMMON_EXPORT Coordinates : public Usul::Base::Referenced
{
public:

  USUL_DECLARE_REF_POINTERS ( Coordinates );

  typedef Usul::Base::Referenced BaseClass;
  typedef std::vector<Usul::Math::Vec3d> Vector;
  typedef Vector::const_iterator const_iterator;
  typedef Vector::const_reverse_iterator const_reverse_iterator;
  typedef Vector::value_type value_type;
  typedef Minerva::Common::Extents Extents;

  Coordinates();

  const_iterator begin() const;
  const_iterator end() const;

  const_reverse_iterator rbegin() const;
  const_reverse_iterator rend() const;

  const Usul::Math::Vec3d& at ( unsigned int i ) const;

  void addPoint ( double lon, double lat, double altitude );

  bool empty() const;

  const Extents& extents() const;

  // Reserve enough room.
  void reserve ( unsigned int num );

  std::size_t size() const;

protected:

  virtual ~Coordinates();

private:

  Vector _coordinates;
  Minerva::Common::Extents _extents;
};


}
}

#endif // __MINERVA_COMMON_COORDINATES_H__
