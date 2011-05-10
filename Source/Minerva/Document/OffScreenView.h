
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Offscreen View.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_DOCUMENT_OFFSCREEN_VIEW_H__
#define __MINERVA_DOCUMENT_OFFSCREEN_VIEW_H__

#include "Minerva/Document/Export.h"
#include "Minerva/Document/View.h"

#include "Minerva/OsgTools/FrameDump.h"
#include "Minerva/OsgTools/OffScreenRenderer.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Pointers/Pointers.h"

#include "Minerva/Core/Data/Camera.h"

#include "boost/shared_ptr.hpp"

namespace Minerva {
namespace Document {


class MINERVA_DOCUMENT_EXPORT OffScreenView : public View
{
public:

  typedef View BaseClass;
  typedef View::Document Document;
  typedef Minerva::OsgTools::FrameDump FrameDump;
  typedef boost::shared_ptr<FrameDump> FrameDumpPtr;

  USUL_DECLARE_REF_POINTERS ( OffScreenView );

  OffScreenView ( Document::RefPtr document, unsigned int width, unsigned int height );
  OffScreenView ( Document::RefPtr document, Minerva::OsgTools::OffScreenRenderer::RefPtr renderer );

  void frameDumpProperties ( const std::string &dir,
                             const std::string &base,
                             const std::string &ext,
                             unsigned int start = 0 );

  void render ( Minerva::Core::Data::Camera::RefPtr camera );

  void waitForDetail ( Minerva::Core::Data::Camera::RefPtr camera );

protected:

  virtual ~OffScreenView();

private:

  void _render ( Minerva::Core::Data::Camera::RefPtr camera, bool dumpFrame = true );

  Minerva::OsgTools::OffScreenRenderer::RefPtr _renderer;
  FrameDumpPtr _frameDump;
};

}
}

#endif // __MINERVA_DOCUMENT_OFFSCREEN_VIEW_H__
