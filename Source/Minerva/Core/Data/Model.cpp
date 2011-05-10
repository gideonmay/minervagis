
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

#include "Minerva/Core/Data/Model.h"
#include "Minerva/Core/Data/OsgModel.h"

#include "Minerva/Common/IElevationDatabase.h"
#include "Minerva/Common/IPlanetCoordinates.h"

#include "Minerva/OsgTools/StateSet.h"

#include "osgUtil/Optimizer"

using namespace Minerva::Core::Data;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Model::Model() :
  BaseClass(),
  _location(),
  _heading ( 0.0 ),
  _tilt ( 0.0 ),
  _roll ( 0.0 ),
  _toMeters ( 0.0254 ),
  _scale ( 1.0, 1.0, 1.0 ),
  _model ( 0x0 ),
  _optimize ( true ),
  _link ( 0x0 )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Model::~Model()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the matrix for the model.
//
///////////////////////////////////////////////////////////////////////////////

Model::Matrix Model::matrix ( Minerva::Common::IPlanetCoordinates* planet, Minerva::Common::IElevationDatabase* elevation ) const
{
#if 0
  osg::Vec3d location ( this->location() );

  // Get the height.
  const double height ( Minerva::Core::Data::getElevationAtPoint ( location, elevation, this->altitudeMode() ) );

  osg::Vec3d hpr ( this->orientation() );
  const double heading ( hpr[0] ), tilt ( hpr[1] ), roll ( hpr[2] );
  
  osg::Matrixd R ( 0x0 != planet ? planet->planetRotationMatrix ( location[1], location[0], height, heading ) : osg::Matrixd() );
  osg::Matrixd S ( osg::Matrixd::scale ( this->scale() * this->toMeters() ) );

  Matrix result ( S *
                  osg::Matrix::rotate ( osg::DegreesToRadians ( tilt ), osg::Vec3 ( 1.0, 0.0, 0.0 ) ) * 
                  osg::Matrix::rotate ( osg::DegreesToRadians ( roll ), osg::Vec3 ( 0.0, 1.0, 0.0 ) ) * R );
  return result;
#else
  return Model::matrix ( this->location(), this->orientation(), this->scale(), this->toMeters(), this->altitudeMode(), planet, elevation );
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the matrix for the model.
//
///////////////////////////////////////////////////////////////////////////////

Model::Matrix Model::matrix ( const osg::Vec3d& location, 
                              const osg::Vec3d& orientation, 
                              const osg::Vec3d& scale, 
                              double toMeters,
                              AltitudeMode altitudeMode,
                              Minerva::Common::IPlanetCoordinates* planet,
                              Minerva::Common::IElevationDatabase* elevation )
{
  // Get the height.
  const double height ( Minerva::Core::Data::getElevationAtPoint ( location, elevation, altitudeMode ) );

  osg::Vec3d hpr ( orientation );
  const double heading ( hpr[0] ), tilt ( hpr[1] ), roll ( hpr[2] );
  
  osg::Matrixd R ( 0x0 != planet ? planet->planetRotationMatrix ( location[1], location[0], height, heading ) : osg::Matrixd() );
  osg::Matrixd S ( osg::Matrixd::scale ( scale * toMeters ) );

  Matrix result ( S *
                  osg::Matrix::rotate ( osg::DegreesToRadians ( tilt ), osg::Vec3 ( 1.0, 0.0, 0.0 ) ) * 
                  osg::Matrix::rotate ( osg::DegreesToRadians ( roll ), osg::Vec3 ( 0.0, 1.0, 0.0 ) ) * R );
  return result;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the scene.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Model::_buildScene ( Style::RefPtr style, IPlanetCoordinates *planet, IElevationDatabase* elevation )
{
  osg::ref_ptr<OsgModel> mt ( new OsgModel ( this, this->model() ) );
  
  Matrix matrix ( this->matrix ( planet, elevation ) );
  mt->setMatrix ( matrix );

  if ( true == this->optimize() )
  {
    osgUtil::Optimizer optimizer;
    optimizer.optimize ( mt.get(), osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS );
  }

  // If there is a scale, turn on normalize.
  if ( true == this->_hasScale() )
    OsgTools::State::StateSet::setNormalize ( this->model(), true );

  mt->addChild ( this->model() );
  return mt.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the location.
//
///////////////////////////////////////////////////////////////////////////////

void Model::location ( const osg::Vec3d& location )
{
  Guard guard ( this->mutex() );
  _location = location;

  // Make new extents.
  Extents e ( Extents::Vertex ( location[0], location[1] ), Extents::Vertex ( location[0], location[1] ) );
  this->extents ( e );
  
  this->dirty ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the location.
//
///////////////////////////////////////////////////////////////////////////////

osg::Vec3d Model::location() const
{
  Guard guard ( this->mutex() );
  return _location;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the orientation.
//
///////////////////////////////////////////////////////////////////////////////

void Model::orientation( double heading, double  tilt, double  roll )
{
  Guard guard ( this->mutex() );
  _heading = heading;
  _tilt = tilt;
  _roll = roll;
  this->dirty ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the orientation.
//
///////////////////////////////////////////////////////////////////////////////

osg::Vec3d Model::orientation() const
{
  osg::Vec3d v;
  {
    Guard guard ( this->mutex() );
    v[0] = _heading;
    v[1] = _tilt;
    v[2] = _roll;
  }
  return v;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the scale.
//
///////////////////////////////////////////////////////////////////////////////

void Model::scale ( const osg::Vec3d& scale )
{
  Guard guard ( this->mutex() );
  _scale = scale;
  this->dirty ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the scale.
//
///////////////////////////////////////////////////////////////////////////////

osg::Vec3d Model::scale() const
{
  Guard guard ( this->mutex() );
  return _scale;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the model.
//
///////////////////////////////////////////////////////////////////////////////

void Model::model ( osg::Node* node )
{
  Guard guard ( this->mutex() );
  _model = node;
  this->dirty ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the model.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* Model::model() const
{
  Guard guard ( this->mutex() );
  return _model.get();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the scale to convert to meters.
//
///////////////////////////////////////////////////////////////////////////////

void Model::toMeters ( double scale )
{
  Guard guard ( this->mutex() );
  _toMeters = scale;
  this->dirty ( true );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the scale to convert to meters.
//
///////////////////////////////////////////////////////////////////////////////

double Model::toMeters() const
{
  Guard guard ( this->mutex() );
  return _toMeters;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the optimization flag.
//
///////////////////////////////////////////////////////////////////////////////

void Model::optimize ( bool state )
{
  Guard guard ( this->mutex() );
  _optimize = state;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the optimization flag.
//
///////////////////////////////////////////////////////////////////////////////

bool Model::optimize() const
{
  Guard guard ( this->mutex() );
  return _optimize;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return true if there is a scale component.
//
///////////////////////////////////////////////////////////////////////////////

bool Model::_hasScale() const
{
  Guard guard ( this->mutex() );
  return ( 1.0 != _toMeters || 1.0 != _scale[0] || 1.0 != _scale[1] || 1.0 != _scale[2] ); 
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the link to load the model.
//
///////////////////////////////////////////////////////////////////////////////

void Model::link ( Link::RefPtr link )
{
  Guard guard ( this->mutex() );
  _link = link;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the link to load the model.
//
///////////////////////////////////////////////////////////////////////////////

Link::RefPtr Model::link() const
{
  Guard guard ( this->mutex() );
  return _link;
}
