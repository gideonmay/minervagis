
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/OsgTools/ConvertMatrix.h"

#include "gtest/gtest.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Test
//
///////////////////////////////////////////////////////////////////////////////

TEST(MatrixConvertTest,UsulToOsgMatrix)
{
  Usul::Math::Matrix44d m0 ( 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 );
  osg::Matrixd m1 ( Usul::Convert::Type<Usul::Math::Matrix44d,osg::Matrixd>::convert ( m0 ) ); 

  ASSERT_DOUBLE_EQ ( m0 ( 0, 0 ), m1 ( 0, 0 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 0, 1 ), m1 ( 1, 0 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 0, 2 ), m1 ( 2, 0 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 0, 3 ), m1 ( 3, 0 ) );
  
  ASSERT_DOUBLE_EQ ( m0 ( 1, 0 ), m1 ( 0, 1 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 1, 1 ), m1 ( 1, 1 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 1, 2 ), m1 ( 2, 1 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 1, 3 ), m1 ( 3, 1 ) );

  ASSERT_DOUBLE_EQ ( m0 ( 2, 0 ), m1 ( 0, 2 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 2, 1 ), m1 ( 1, 2 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 2, 2 ), m1 ( 2, 2 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 2, 3 ), m1 ( 3, 2 ) );

  ASSERT_DOUBLE_EQ ( m0 ( 3, 0 ), m1 ( 0, 3 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 3, 1 ), m1 ( 1, 3 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 3, 2 ), m1 ( 2, 3 ) );
  ASSERT_DOUBLE_EQ ( m0 ( 3, 3 ), m1 ( 3, 3 ) );
}
