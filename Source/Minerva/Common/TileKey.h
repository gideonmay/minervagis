
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Information for a tile.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_COMMON_TILE_KEY_H__
#define __MINERVA_COMMON_TILE_KEY_H__

#include "Minerva/Common/Export.h"
#include "Minerva/Common/Extents.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Math/Vector2.h"
#include "Usul/Pointers/Pointers.h"

#include "boost/array.hpp"

namespace Minerva {
namespace Common {


  class MINERVA_COMMON_EXPORT TileKey : public Usul::Base::Referenced
{
public:
  
  USUL_DECLARE_REF_POINTERS ( TileKey );
  
  typedef Usul::Base::Referenced BaseClass;
  typedef boost::array<TileKey::RefPtr,4> ChildrenKeys;
  
  TileKey();
  
  // Indices for children
  enum Indices
  {
    LOWER_LEFT = 0,
    LOWER_RIGHT = 1,
    UPPER_LEFT = 2,
    UPPER_RIGHT = 3
  };
  
  void split ( ChildrenKeys& keys ) const;
  
  unsigned int row() const;
  void         row ( unsigned int );
  
  unsigned int column() const;
  void         column ( unsigned int );
  
  unsigned int level() const;
  void         level ( unsigned int );
  
  const Extents& extents() const;
  void           extents ( const Extents& );
  
  const Usul::Math::Vec2ui& imageSize() const;
  void                      imageSize ( const Usul::Math::Vec2ui& );
  
  const Usul::Math::Vec2ui& meshSize() const;
  void                      meshSize ( const Usul::Math::Vec2ui& );
  
protected:
  
  virtual ~TileKey();
  
private:
  
  unsigned int _row;
  unsigned int _column;
  unsigned int _level;
  Extents _extents;
  Usul::Math::Vec2ui _imageSize;
  Usul::Math::Vec2ui _meshSize;
};


}
}

#endif // __MINERVA_COMMON_TILE_KEY_H__
