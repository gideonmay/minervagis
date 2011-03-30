
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2004, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  The camera path class.
//
///////////////////////////////////////////////////////////////////////////////

#include "CameraPath.h"

#include "Serialize/XML/Deserialize.h"
#include "Serialize/XML/Serialize.h"

#include "Usul/Factory/RegisterCreator.h"
#include "Usul/File/Path.h"
#include "Usul/Strings/Case.h"
#include "Usul/Threads/Safe.h"

using namespace Minerva::Document;

USUL_FACTORY_REGISTER_CREATOR ( CameraPath );

USUL_IO_TEXT_DEFINE_READER_TYPE_VECTOR_3 ( CameraPath::Triplet );
USUL_IO_TEXT_DEFINE_WRITER_TYPE_VECTOR_3 ( CameraPath::Triplet );
SERIALIZE_XML_DECLARE_VECTOR_3_WRAPPER ( CameraPath::Triplet );


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

CameraPath::CameraPath() : BaseClass(),
  _values()
{
  this->_addMember ( "values", _values );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

CameraPath::~CameraPath()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the camera info.
//
///////////////////////////////////////////////////////////////////////////////

void CameraPath::camera ( Usul::Math::Vec3d &eye, Usul::Math::Vec3d &center, Usul::Math::Vec3d &up, unsigned int num ) const
{
  Guard guard ( this );
  if ( num < this->size() )
  {
    Triplet t ( _values.at ( num ) );
    eye = t[0];
    center = t[1];
    up = t[2];
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Append the camera information.
//
///////////////////////////////////////////////////////////////////////////////

void CameraPath::append ( const Usul::Math::Vec3d &eye, const Usul::Math::Vec3d &center, const Usul::Math::Vec3d &up )
{
  Guard guard ( this );
  _values.insert ( _values.end(), Triplet ( eye, center, up ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Prepend the camera information.
//
///////////////////////////////////////////////////////////////////////////////

void CameraPath::prepend ( const Usul::Math::Vec3d &eye, const Usul::Math::Vec3d &center, const Usul::Math::Vec3d &up )
{
  Guard guard ( this );
  _values.insert ( _values.begin(), Triplet ( eye, center, up ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Insert the camera between the two nearest.
//
///////////////////////////////////////////////////////////////////////////////

void CameraPath::insertBetweenNearest ( const Usul::Math::Vec3d &eye, const Usul::Math::Vec3d &center, const Usul::Math::Vec3d &up )
{
  Guard guard ( this );

  // If the path does not have at least two then append.
  if ( _values.size() < 2 )
  {
    this->append ( eye, center, up );
    return;
  }

  // The value to insert.
  const Triplet camera ( eye, center, up );

  // If there are two camers then it's trivial.
  if ( 2 == _values.size() )
  {
    _values.insert ( _values.begin() + 1, camera );
    return;
  }

  // Find the nearest camera.
  const unsigned int nearest ( this->_closest ( eye ) );
  if ( nearest >= _values.size() )
    throw std::runtime_error ( "Error 2921860230: Failed to find nearest camera" );

  // Is the nearest camera the first one?
  if ( 0 == nearest )
  {
    _values.insert ( _values.begin() + 1, camera );
    return;
  }

  // Is the nearest camera the last one?
  if ( ( _values.size() - 1 ) == nearest )
  {
    _values.insert ( _values.begin() + _values.size() - 1, camera );
    return;
  }

  // When we get here we know that there are at least 3 cameras, and that the 
  // nearest camera is not either end. See which neighboring camera is closer.
  const double distBefore ( eye.distanceSquared ( _values.at ( nearest - 1 )[0] ) );
  const double distAfter  ( eye.distanceSquared ( _values.at ( nearest + 1 )[0] ) );

  // Insert between nearest and the one before it.
  if ( distBefore < distAfter )
  {
    _values.insert ( _values.begin() + nearest, camera );
  }

  // Insert between nearest and the one after it.
  else
  {
    _values.insert ( _values.begin() + nearest + 1, camera );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Remove the camera closest to the given eye position.
//
///////////////////////////////////////////////////////////////////////////////

void CameraPath::removeNearest ( const Usul::Math::Vec3d &eye, const Usul::Math::Vec3d &, const Usul::Math::Vec3d & )
{
  Guard guard ( this );

  // Handle empty path.
  if ( true == _values.empty() )
    return;

  // Find the nearest camera.
  const unsigned int nearest ( this->_closest ( eye ) );
  if ( nearest >= _values.size() )
    throw std::runtime_error ( "Error 4726092730: Failed to find nearest camera" );

  // Remove the nearest camera.
  _values.erase ( _values.begin() + nearest );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Replace the camera closest to the given eye position with the new values.
//
///////////////////////////////////////////////////////////////////////////////

void CameraPath::replaceNearest ( const Usul::Math::Vec3d &eye, const Usul::Math::Vec3d &center, const Usul::Math::Vec3d &up )
{
  Guard guard ( this );

  // Handle empty path.
  if ( true == _values.empty() )
    return;

  // Find the nearest camera.
  const unsigned int nearest ( this->_closest ( eye ) );
  if ( nearest >= _values.size() )
    throw std::runtime_error ( "Error 4274807095: Failed to find nearest camera" );

  // Update the nearest camera.
  _values.at ( nearest ) = Triplet ( eye, center, up );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Close the path if we can.
//
///////////////////////////////////////////////////////////////////////////////

void CameraPath::closePath()
{
  Guard guard ( this );

  if ( false == this->canClose() )
    return;

  _values.insert ( _values.end(), _values.front() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Can we close the path?
//
///////////////////////////////////////////////////////////////////////////////

bool CameraPath::canClose() const
{
  Guard guard ( this );
  return ( _values.size() > 1 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Can we play the animation?
//
///////////////////////////////////////////////////////////////////////////////

bool CameraPath::canPlay() const
{
  Guard guard ( this );
  return ( _values.size() > 1 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clear all the paths.
//
///////////////////////////////////////////////////////////////////////////////

void CameraPath::clear()
{
  Guard guard ( this );
  _values.clear();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the number of cameras.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int CameraPath::size() const
{
  Guard guard ( this );
  return _values.size();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the values.
//
///////////////////////////////////////////////////////////////////////////////

void CameraPath::values ( Values &v, bool reverseOrder ) const
{
  Guard guard ( this );

  if ( true == reverseOrder )
  {
    v.assign ( _values.rbegin(), _values.rend() );
  }
  else
  {
    v.assign ( _values.begin(), _values.end() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find the closest camera.
//
///////////////////////////////////////////////////////////////////////////////

unsigned int CameraPath::_closest ( const Usul::Math::Vec3d &eye ) const
{
  // Copy the values.
  Values values ( Usul::Threads::Safe::get ( this->mutex(), _values ) );

  // Initialize.
  unsigned int nearest ( values.size() );
  double minDist ( std::numeric_limits<double>::max() );

  // Loop through the cameras.
  for ( unsigned int i = 0; i < values.size(); ++i )
  {
    const double currentDist ( eye.distanceSquared ( values.at(i)[0] ) );
    if ( currentDist < minDist )
    {
      minDist = currentDist;
      nearest = i;
    }
  }

  // Return nearest, which could be the end.
  return nearest;
}
