
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  A flat land model.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/GDAL/FlatLandModel.h"

#include "ogr_spatialref.h"
#include "ogr_srs_api.h"

using namespace Minerva::Layers::GDAL;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

FlatLandModel::FlatLandModel ( const std::string& projection ) : BaseClass(),
  _spatialReference(),
  _toSRStransform ( (OGRCoordinateTransformation*) 0x0, ::OCTDestroyCoordinateTransformation ),
  _fromSRStransform ( (OGRCoordinateTransformation*) 0x0, ::OCTDestroyCoordinateTransformation )
{
  _spatialReference.reset ( new OGRSpatialReference ( projection.c_str() ) );

  OGRSpatialReference wgs84;
  wgs84.SetWellKnownGeogCS ( "WGS84" );

  _toSRStransform.reset ( ::OGRCreateCoordinateTransformation ( &wgs84, _spatialReference.get() ) );
  _fromSRStransform.reset ( ::OGRCreateCoordinateTransformation ( _spatialReference.get(), &wgs84 ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

FlatLandModel::~FlatLandModel()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convert lat, lon, height to x,y,z.
//
///////////////////////////////////////////////////////////////////////////////

void FlatLandModel::latLonHeightToXYZ ( double lat, double lon, double elevation, double& x, double& y, double& z ) const
{
  x = lon;
  y = lat;
  z = elevation;

  if ( _toSRStransform )
  {
    _toSRStransform->Transform ( 1, &x, &y, &z );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Convert x,y,z to lat, lon, height.
//
///////////////////////////////////////////////////////////////////////////////

void FlatLandModel::xyzToLatLonHeight ( double x, double y, double z, double& lat, double& lon, double& elevation ) const
{
  lon = x;
  lat = y;
  elevation = z;

  if ( _fromSRStransform )
  {
    _fromSRStransform->Transform ( 1, &lon, &lat, &elevation );
  }
}

 
///////////////////////////////////////////////////////////////////////////////
//
//  Matrix to place items on the planet (i.e. local coordinates to world coordinates).
//
///////////////////////////////////////////////////////////////////////////////

FlatLandModel::Matrix FlatLandModel::planetRotationMatrix ( double lat, double lon, double elevation, double heading ) const
{
  return Matrix();
}
