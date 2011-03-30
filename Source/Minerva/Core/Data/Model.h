
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class for generic model.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_DATA_MODEL_H__
#define __MINERVA_CORE_DATA_MODEL_H__

#include "Minerva/Core/Data/Geometry.h"
#include "Minerva/Core/Data/Link.h"

#include "osg/Vec3"
#include "osg/Node"

namespace Minerva { namespace Common { struct IElevationDatabase; struct IPlanetCoordinates; } }

namespace Minerva {
namespace Core {
namespace Data {
      

class MINERVA_EXPORT Model : public Minerva::Core::Data::Geometry
{
public:
  typedef Minerva::Core::Data::Geometry BaseClass;
  typedef osg::Matrixd Matrix;
  
  // Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( Model );
  
  Model();

  /// Set/get the link to load the model.
  void                  link ( Link::RefPtr );
  Link::RefPtr          link() const;
  
  /// Get/Set the location.
  void                  location ( const osg::Vec3d& );
  osg::Vec3d            location() const;
  
  /// Get/Set the orientation.
  void                  orientation( double  heading, double  tilt, double  roll );
  osg::Vec3d            orientation() const;
  
  /// Get/Set the scale.
  void                  scale ( const osg::Vec3d& );
  osg::Vec3d            scale() const;

  /// Get the matrix for the model.
  Matrix                matrix ( Minerva::Common::IPlanetCoordinates* planet, Minerva::Common::IElevationDatabase* elevation ) const;

  static Matrix         matrix ( const osg::Vec3d& location, 
                                 const osg::Vec3d& orientation, 
                                 const osg::Vec3d& scale, 
                                 double toMeters,
                                 AltitudeMode mode,
                                 Minerva::Common::IPlanetCoordinates* planet,
                                 Minerva::Common::IElevationDatabase* elevation );
  
  /// Get/Set the model.
  void                  model ( osg::Node* );
  osg::Node*            model() const;

  /// Set/get the scale to convert to meters.
  void                  toMeters ( double );
  double                toMeters() const;

  /// Set/get the flags for optimizing the node.
  void                  optimize ( bool );
  bool                  optimize() const;
  
protected:
  virtual ~Model();
  
  virtual osg::Node*    _buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation );

  bool _hasScale() const;

private:

  osg::Vec3d _location;
  double _heading;
  double _tilt;
  double _roll;
  double _toMeters;
  osg::Vec3d _scale;
  osg::ref_ptr<osg::Node> _model;
  bool _optimize;
  Link::RefPtr _link;
};


}
}
}


#endif // __MINERVA_CORE_DATA_MODEL_H__
