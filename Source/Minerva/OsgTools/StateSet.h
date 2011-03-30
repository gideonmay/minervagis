
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV and John K. Grant
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class for working with the state.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _OSG_TOOLS_STATE_CLASS_H_
#define _OSG_TOOLS_STATE_CLASS_H_

#include "Minerva/OsgTools/Export.h"

namespace osg { class Vec4f; class Node; class StateSet; class Material; }


namespace OsgTools {
namespace State {


struct MINERVA_OSG_TOOLS_EXPORT StateSet
{
  // Set/get the lighting state
  static void               setLighting  ( osg::Node *node, bool state );
  static void               setLighting  ( osg::StateSet *ss, bool state );

  // Set/Get the two sided lighting state.
  static void               setTwoSidedLighting ( osg::Node *node, bool state );
  static void               setTwoSidedLighting ( osg::StateSet *ss, bool state );

  // Set/get the normalization state.
  static void               setNormalize ( osg::Node *node, bool state );
  static void               setNormalize ( osg::StateSet *ss, bool state );

  // Make filled polygons draw with flat shading.
  static void               setPolygonsTextures ( osg::StateSet* ss, bool todo );

  // Set/get the polygon mode.
  static void               setPolygonMode ( osg::Node *node, unsigned int face, unsigned int mode );

  // Set/Get line width
  static void               setLineWidth ( osg::StateSet *ss, float width );
  static void               setLineWidth ( osg::Node *node, float width );

  // Set/Get point size.
  static void               setPointSize ( osg::StateSet *ss, float width );
  static void               setPointSize ( osg::Node *node, float width );

  // Materials.
  static void               setMaterial ( osg::Node *node, osg::Material *mat );
  static void               setMaterial ( osg::StateSet *ss, osg::Material *mat );
  static void               setMaterial ( osg::Node *node, const osg::Vec4f &ambient, const osg::Vec4f &diffuse, float alpha );
  static void               setMaterialRandom ( osg::Node *node );
  static void               setMaterialDefault ( osg::Node *node );
  static osg::Material *    getMaterialDefault();
  static void               removeMaterial ( osg::Node *node );
  static void               removeMaterial ( osg::StateSet *ss );

  // Set the alpha value. Adds default material if needed.
  static void               setAlpha ( osg::Node *node, float );
  static void               setAlpha ( osg::StateSet *ss, float );
};


}; // namespace State
}; // namespace OsgTools


#endif // _OSG_TOOLS_STATE_CLASS_H_
