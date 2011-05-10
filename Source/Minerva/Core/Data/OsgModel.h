
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class for OpenSceneGraph model.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_DATA_OSG_MODEL_H__
#define __MINERVA_CORE_DATA_OSG_MODEL_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/ElevationData.h"
#include "Minerva/Core/Data/AltitudeMode.h"
#include "Minerva/Core/Data/Model.h"

#include "Minerva/Common/Extents.h"

#include "osg/MatrixTransform"

namespace Minerva { namespace Common { struct IElevationDatabase; struct IPlanetCoordinates; } }

namespace Minerva {
namespace Core {
namespace Data {


class MINERVA_EXPORT OsgModel : public osg::MatrixTransform
{
public:

  typedef osg::MatrixTransform BaseClass;
  typedef Minerva::Common::IElevationDatabase IElevationDatabase;
  typedef Minerva::Common::IPlanetCoordinates IPlanetCoordinates;
  typedef Minerva::Common::Extents Extents;
  typedef Minerva::Common::IElevationData::QueryPtr ElevationDataPtr;

  OsgModel ( Model::RefPtr model,
             osg::Node* node );

  /// Elevation has changed within given extents.
  void elevationChangedNotify ( const Extents& extents, 
                                unsigned int level, 
                                ElevationDataPtr elevationData, 
                                IPlanetCoordinates *planet, 
                                IElevationDatabase* elevation );

  virtual void traverse ( osg::NodeVisitor& nv );

protected:

  virtual ~OsgModel();

private:

  Model::RefPtr _model;
  osg::ref_ptr<osg::Node> _node;

  Usul::Threads::Mutex _mutex;
  bool _needsNewMatrix;
  Model::Matrix _matrix;
};

}
}
}

#endif // __MINERVA_CORE_DATA_OSG_MODEL_H__
