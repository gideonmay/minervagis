
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_DATA_POINT_STYLE_H__
#define __MINERVA_CORE_DATA_POINT_STYLE_H__

#include "Minerva/Core/Data/ColorStyle.h"

namespace Minerva {
namespace Core {
namespace Data {


class MINERVA_EXPORT PointStyle : public Minerva::Core::Data::ColorStyle
{
public:
  typedef Minerva::Core::Data::ColorStyle BaseClass;

  USUL_DECLARE_REF_POINTERS ( PointStyle );

  PointStyle();

  enum PrimitiveType
  {
    NONE,
    POINT,
    SPHERE
  };

  /// Set/get the primitiveId.
  PrimitiveType primitiveId() const;
  void          primitiveId ( PrimitiveType type );

  /// Set/get the size.
  void        size ( float size );
  float       size() const;

protected:

  virtual ~PointStyle();

private:

  PrimitiveType _type;
  float _size;

  SERIALIZE_XML_CLASS_NAME ( PointStyle );
};


}
}
}

#endif // __MINERVA_CORE_DATA_POINT_STYLE_H__
