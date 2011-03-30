
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class to render to off screen target, without needed a window.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __OSG_TOOLS_OFF_SCREEN_RENDERER_H__
#define __OSG_TOOLS_OFF_SCREEN_RENDERER_H__

#include "Minerva/OsgTools/Export.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Pointers/Pointers.h"
#include "Usul/Math/Vector2.h"
#include "Usul/Math/Matrix44.h"

#include "osg/ref_ptr"
#include "osg/Image"

namespace Minerva {
namespace OsgTools {


class MINERVA_OSG_TOOLS_EXPORT OffScreenRenderer : public Usul::Base::Referenced
{
public:
  typedef Usul::Base::Referenced BaseClass;
  typedef osg::ref_ptr<osg::Image> ImagePtr;

  USUL_DECLARE_REF_POINTERS ( OffScreenRenderer );

  // Set the scene.
  virtual void   scene ( osg::Node* ) = 0;

  // Render.
  virtual ImagePtr render() const = 0;

  // Set the view matrix.
  virtual void viewMatrix ( const Usul::Math::Matrix44d& matrix ) = 0;

protected:
  
  OffScreenRenderer();
  virtual ~OffScreenRenderer();

private:

};


}
}

#endif // __OSG_TOOLS_OFF_SCREEN_RENDERER_H__
