
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Common/TileKey.h"

#include "gtest/gtest.h"

typedef Minerva::Common::TileKey TileKey;

TEST(TileKeyTest,Split)
{
  TileKey::RefPtr key ( new TileKey );
  key->row ( 0 );
  key->column ( 0 );
  key->level ( 0 );
  key->extents ( Minerva::Common::Extents ( -180, -90, 180, 90 ) );
  
  TileKey::ChildrenKeys childrenKeys;
  key->split ( childrenKeys );
  
  EXPECT_EQ ( 1u, childrenKeys[0]->level() );
  EXPECT_EQ ( 1u, childrenKeys[1]->level() );
  EXPECT_EQ ( 1u, childrenKeys[2]->level() );
  EXPECT_EQ ( 1u, childrenKeys[3]->level() );
  
  EXPECT_EQ ( 0u, childrenKeys[TileKey::UPPER_LEFT]->row() );
  EXPECT_EQ ( 0u, childrenKeys[TileKey::UPPER_LEFT]->column() );
  
  EXPECT_EQ ( 1u, childrenKeys[TileKey::LOWER_LEFT]->row() );
  EXPECT_EQ ( 0u, childrenKeys[TileKey::LOWER_LEFT]->column() );
  
  EXPECT_EQ ( 0u, childrenKeys[TileKey::UPPER_RIGHT]->row() );
  EXPECT_EQ ( 1u, childrenKeys[TileKey::UPPER_RIGHT]->column() );
  
  EXPECT_EQ ( 1u, childrenKeys[TileKey::LOWER_RIGHT]->row() );
  EXPECT_EQ ( 1u, childrenKeys[TileKey::LOWER_RIGHT]->column() );
}
