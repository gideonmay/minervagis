
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Build a path for animation.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_DOCUMENT_PATH_BUILDER_H__
#define __MINERVA_DOCUMENT_PATH_BUILDER_H__

#include "Minerva/Document/Export.h"

#include "Minerva/Core/TileEngine/Body.h"

#include "Usul/Interfaces/IAnimateMatrices.h"

namespace Minerva { namespace Core { namespace Data { class Camera; class Feature; } } }

#include "osg/Matrix"

namespace Minerva {
namespace Document {

class MINERVA_DOCUMENT_EXPORT PathBuilder
{
public:
  
  typedef Minerva::Core::Data::Feature Feature;
  typedef Minerva::Core::Data::Camera Camera;
  typedef Usul::Interfaces::IAnimateMatrices::Matrices Matrices;
  
  static Camera* makeCamera ( const Feature& feature, const Minerva::Core::TileEngine::Body& body );
  static void lookAtLayer ( Camera* camera, Feature * layer, Minerva::Core::TileEngine::Body::RefPtr body, Matrices& matrices );
  static void lookAtPoint ( Camera* camera, const Usul::Math::Vec2d& point, Minerva::Core::TileEngine::Body::RefPtr body, Matrices& matrics );
  
  static void generateAnimatePath ( 
                                   Camera* start, 
                                   Camera* end, 
                                   unsigned int numPoints,
                                   Minerva::Core::TileEngine::LandModel*, 
                                   Matrices& matrices );
  
private:
  
  static void _animatePath ( const Usul::Math::Vec3d& llh1, 
                             const Usul::Math::Vec3d& llh2, 
                             double percentMidpointHeightAtTransition,
                             const Usul::Math::Vec3ui &numPoints,
                             Minerva::Core::TileEngine::Body& body,
                             const osg::Matrixd& startingView,
                             Matrices& matrics );
};

}
}

#endif // __MINERVA_DOCUMENT_PATH_BUILDER_H__
