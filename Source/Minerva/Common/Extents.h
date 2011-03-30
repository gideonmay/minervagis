
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_COMMON_EXTENTS_H__
#define __MINERVA_COMMON_EXTENTS_H__

#include "Usul/Math/MinMax.h"

#include "Usul/Math/Vector2.h"

#include <stdexcept>


namespace Minerva {
namespace Common {

class Extents
{
public:

  typedef Usul::Math::Vec2d Vertex;
  typedef Vertex::value_type value_type;
  typedef value_type ValueType;
  typedef Extents ThisType;

  /// Construction/Destruction.
  Extents();
  Extents ( ValueType minLon, ValueType minLat, ValueType maxLon, ValueType maxLat );
  Extents ( const Vertex& min, const Vertex& max );
  Extents ( const Extents& rhs );
  ~Extents();

  /// Assignment.
  Extents& operator = ( const Extents& rhs );

  /// Does the extents contain vertex v?
  bool                  contains ( const Vertex& v ) const;
  
  /// Get the center of the extents.
  Vertex                center() const;

  /// Expand by the extents or vertex.
  void                  expand ( const Extents& extents );
  void                  expand ( const Vertex& v );

  /// Does the extent intersect this extent.
  bool                  intersects ( const Extents& extents ) const;

  /// Get the min.
  const Vertex &        minimum() const;
  ValueType             minLon() const;
  ValueType             minLat() const;

  /// Get the max.
  const Vertex &        maximum() const;
  ValueType             maxLon() const;
  ValueType             maxLat() const;

  /// Bracket operator, largely for serialization.
  ValueType             operator [] ( unsigned int ) const;
  ValueType &           operator [] ( unsigned int );

  /// Split the extents.
  void                  split ( Extents &ll, Extents &lr, Extents &ul, Extents &ur ) const;

private:

  Vertex _min;
  Vertex _max;
};


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents::Extents() :
  _min ( 0.0, 0.0 ),
  _max ( 0.0, 0.0 )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents::Extents ( const Vertex& min, const Vertex& max ) :
  _min ( min ),
  _max ( max )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents::Extents ( ValueType minLon, ValueType minLat, ValueType maxLon, ValueType maxLat ) :
  _min ( minLon, minLat ),
  _max ( maxLon, maxLat )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy Constructor.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents::Extents ( const Extents& rhs ) :
  _min ( rhs._min ),
  _max ( rhs._max )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Assignment.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents& Extents::operator = ( const Extents& rhs )
{
  _min = rhs._min;
  _max = rhs._max;
  return *this;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents::~Extents()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the min.
//
///////////////////////////////////////////////////////////////////////////////

inline const Extents::Vertex &Extents::minimum() const
{
  return _min;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the max.
//
///////////////////////////////////////////////////////////////////////////////

inline const Extents::Vertex &Extents::maximum() const
{
  return _max;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the min latitude.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents::ValueType Extents::minLat() const
{
  return _min[1];
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the max latitude.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents::ValueType Extents::maxLat() const
{
  return _max[1];
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the min longitide.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents::ValueType Extents::minLon() const
{
  return _min[0];
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the max longitide.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents::ValueType Extents::maxLon() const
{
  return _max[0];
}


///////////////////////////////////////////////////////////////////////////////
//
//  Bracket operator to help with serialization.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents::ValueType Extents::operator [] ( unsigned int i ) const
{
  if ( i > 3 )
  {
    throw std::runtime_error ( "Error 2037139798: Index out of range" );
  }
  return ( ( i < 2 ) ? _min[i] : _max[i-2] );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Bracket operator to help with serialization.
//
///////////////////////////////////////////////////////////////////////////////

inline Extents::ValueType &Extents::operator [] ( unsigned int i )
{
  if ( i > 3 )
  {
    throw std::runtime_error ( "Error 8676398430: Index out of range" );
  }
  return ( ( i < 2 ) ? _min[i] : _max[i-2] );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Expand by the extents.
//
///////////////////////////////////////////////////////////////////////////////

inline void Extents::expand ( const Extents& extents )
{
  Vertex zero ( 0.0, 0.0 );

  // If we are invalid.
  if ( zero == _min && zero == _max )
  {
    _min = extents.minimum();
    _max = extents.maximum();
  }
  else
  {
    const Vertex mn ( extents.minimum() );
    const Vertex mx ( extents.maximum() );

    _min[0] = Usul::Math::minimum ( _min[0], mn[0] );
    _min[1] = Usul::Math::minimum ( _min[1], mn[1] );

    _max[0] = Usul::Math::maximum ( _max[0], mx[0] );
    _max[1] = Usul::Math::maximum ( _max[1], mx[1] );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Expand by the vertex.
//
///////////////////////////////////////////////////////////////////////////////

inline void Extents::expand ( const Vertex& v )
{
  Vertex zero ( 0.0, 0.0 );

  // If we are invalid.
  if ( zero == _min && zero == _max )
  {
    _min = v;
    _max = v;
  }
  else
  {
    _min[0] = Usul::Math::minimum ( _min[0], v[0] );
    _min[1] = Usul::Math::minimum ( _min[1], v[1] );

    _max[0] = Usul::Math::maximum ( _max[0], v[0] );
    _max[1] = Usul::Math::maximum ( _max[1], v[1] );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Does the extent intersect this extent?
//
///////////////////////////////////////////////////////////////////////////////

inline bool Extents::intersects ( const Extents& extents ) const
{
  const Vertex mn ( extents.minimum() );
  const Vertex mx ( extents.maximum() );
  
  return ( Usul::Math::maximum ( _min[0], mn[0] ) <= Usul::Math::minimum ( _max[0], mx[0] ) &&
           Usul::Math::maximum ( _min[1], mn[1] ) <= Usul::Math::minimum ( _max[1], mx[1] ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Does the extents contain vertex v?
//
///////////////////////////////////////////////////////////////////////////////

inline bool Extents::contains ( const Vertex& v ) const
{
  const Vertex mn ( this->minimum() );
  const Vertex mx ( this->maximum() );

  return mn[0] <= v[0] && mn[1] <= v[1] && mx[0] >= v[0] && mx[1] >= v[1];
}
  

///////////////////////////////////////////////////////////////////////////////
//
//  Get the center of the extents.
//
///////////////////////////////////////////////////////////////////////////////
  
inline Extents::Vertex Extents::center() const
{
  return ( Vertex ( this->minimum() + this->maximum() ) / 2.0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Split the extents.
//
///////////////////////////////////////////////////////////////////////////////

inline void Extents::split ( Extents &ll, Extents &lr, Extents &ul, Extents &ur ) const
{
  const Vertex &mn ( this->minimum() );
  const Vertex &mx ( this->maximum() );
  const Vertex md ( ( mx + mn ) * static_cast < ValueType > ( 0.5 ) );

  ll = Extents ( Vertex ( mn[0], mn[1] ), Vertex ( md[0], md[1] ) );
  lr = Extents ( Vertex ( md[0], mn[1] ), Vertex ( mx[0], md[1] ) );
  ul = Extents ( Vertex ( mn[0], md[1] ), Vertex ( md[0], mx[1] ) );
  ur = Extents ( Vertex ( md[0], md[1] ), Vertex ( mx[0], mx[1] ) );
}

  
} // namespace Core
} // namespace Minerva


#endif // __MINERVA_CORE_EXTENTS_H__
