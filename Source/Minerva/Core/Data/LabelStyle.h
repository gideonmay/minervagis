
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_DATA_LABEL_STYLE_H__
#define __MINERVA_CORE_DATA_LABEL_STYLE_H__

#include "Minerva/Core/Data/ColorStyle.h"

namespace Minerva {
namespace Core {
namespace Data {


class MINERVA_EXPORT LabelStyle : public Minerva::Core::Data::ColorStyle
{
public:
  typedef Minerva::Core::Data::ColorStyle BaseClass;

  USUL_DECLARE_REF_POINTERS ( LabelStyle );

  LabelStyle();

  /// Set/get the scale.
  void        scale ( float scale );
  float       scale() const;

protected:

  virtual ~LabelStyle();

private:

  float _scale;

  SERIALIZE_XML_CLASS_NAME ( LabelStyle );
};


}
}
}

#endif // __MINERVA_CORE_DATA_LABEL_STYLE_H__
