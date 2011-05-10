
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Test for AnimationController.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Document/AnimationController.h"

#include "gtest/gtest.h"

using Minerva::Core::Data::Date;
using Minerva::Core::Data::TimeSpan;
using Minerva::Document::AnimationController;


///////////////////////////////////////////////////////////////////////////////
//
//  Test fixture.
//
///////////////////////////////////////////////////////////////////////////////

class AnimationControllerTest : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    begin = Date ( "2010-01-01" );
    end = Date ( "2010-01-02" );

    global = new TimeSpan ( begin, end );

    controller = new AnimationController;
    controller->globalTimeSpan ( global );
  }

  Date begin;
  Date end;
  TimeSpan::RefPtr global;
  AnimationController::RefPtr controller;
};


///////////////////////////////////////////////////////////////////////////////
//
//  Test setting the global time span.
//
///////////////////////////////////////////////////////////////////////////////

TEST_F(AnimationControllerTest,TestSettingGlobalTimeSpan)
{
  TimeSpan::RefPtr visible ( controller->visibleTimeSpan() );
  
  EXPECT_TRUE ( visible.valid() );
  EXPECT_TRUE ( visible->begin() == begin );
  EXPECT_TRUE ( visible->end() == begin );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Test the number of time steps.
//
///////////////////////////////////////////////////////////////////////////////

TEST_F(AnimationControllerTest,TestNumberOfTimeSteps)
{ 
  EXPECT_TRUE ( 3 == controller->getNumberOfTimeSteps() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Test setting the time step.
//
///////////////////////////////////////////////////////////////////////////////

TEST_F(AnimationControllerTest,TestSetTimeStep)
{
  boost::gregorian::date date ( 2010, 01, 01 );
  boost::posix_time::ptime t1 ( date );
  boost::posix_time::ptime t2 ( date, boost::posix_time::hours ( 8 ) );
  boost::posix_time::ptime t3 ( date, boost::posix_time::hours ( 16 ) );

  TimeSpan::RefPtr visible ( controller->visibleTimeSpan() );

  EXPECT_TRUE ( t1 == visible->begin().date() );
  EXPECT_TRUE ( t1 == visible->end().date() );

  controller->setCurrentTimeStep ( 1 );

  EXPECT_TRUE ( t2 == visible->begin().date() );
  EXPECT_TRUE ( t2 == visible->end().date() );

  controller->setCurrentTimeStep ( 2 );

  EXPECT_TRUE ( t3 == visible->begin().date() );
  EXPECT_TRUE ( t3 == visible->end().date() );
}
