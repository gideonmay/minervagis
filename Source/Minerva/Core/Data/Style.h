
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_LAYERS_KML_STYLE_H__
#define __MINERVA_LAYERS_KML_STYLE_H__

#include "Minerva/Core/Data/Object.h"
#include "Minerva/Core/Data/IconStyle.h"
#include "Minerva/Core/Data/LabelStyle.h"
#include "Minerva/Core/Data/LineStyle.h"
#include "Minerva/Core/Data/PointStyle.h"
#include "Minerva/Core/Data/PolyStyle.h"

#include "Minerva/Core/Declarations.h"

namespace Minerva {
namespace Core {
namespace Data {
      

class MINERVA_EXPORT Style : public Minerva::Core::Data::Object
{
public:
  typedef Minerva::Core::Data::Object BaseClass;
  
  USUL_DECLARE_REF_POINTERS ( Style );
  
  Style();

  /// Set/get the iconstyle.
  void           iconstyle ( IconStyle * );
  IconStyle*     iconstyle() const;

  /// Set/get the labelstyle.
  void           labelstyle ( LabelStyle * );
  LabelStyle*    labelstyle() const;

  /// Set/get the linestyle.
	void           linestyle ( LineStyle * );
	LineStyle*     linestyle() const;

	/// Set/get the polystyle.
	void           polystyle ( PolyStyle * );
	PolyStyle*     polystyle() const;

  /// Set/get the point style.
  void           pointstyle ( PointStyle * );
  PointStyle*    pointstyle() const;

protected:
  
  virtual ~Style();
  
private:

  IconStyle::RefPtr _iconstyle;
  LabelStyle::RefPtr _labelstyle;
  LineStyle::RefPtr _linestyle;
  PolyStyle::RefPtr _polystyle;
  PointStyle::RefPtr _pointstyle;

  SERIALIZE_XML_CLASS_NAME ( Style );
};


}
}
}

#endif // __MINERVA_LAYERS_KML_STYLE_H__
