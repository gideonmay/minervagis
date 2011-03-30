
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Controller for animations.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_DOCUMENT_ANIMATION_CONTROLLER_H__
#define __MINERVA_DOCUMENT_ANIMATION_CONTROLLER_H__

#include "Minerva/Document/Export.h"

#include "Minerva/Core/Data/TimeSpan.h"

#include "Usul/Base/Referenced.h"
#include "Usul/Pointers/Pointers.h"

namespace Minerva {
namespace Document {

class MINERVA_DOCUMENT_EXPORT AnimationController : public Usul::Base::Referenced
{
public:

  typedef Usul::Base::Referenced BaseClass;
  typedef Minerva::Core::Data::TimeSpan TimeSpan;

  USUL_DECLARE_REF_POINTERS ( AnimationController );

  enum AnimationResult
  {
    ANIMATION_RESULT_CONTINUE,
    ANIMATION_RESULT_AT_BEGIN,
    ANIMATION_RESULT_AT_END,
    ANIMATION_RESULT_ERROR
  };

  AnimationController();

  // Set/get the global time span.
  void             globalTimeSpan ( TimeSpan::RefPtr );
  TimeSpan::RefPtr globalTimeSpan() const;

  // Set/get the visible time span.
  void             visibleTimeSpan ( TimeSpan::RefPtr );
  TimeSpan::RefPtr visibleTimeSpan() const;

  void resetVisibleTimeSpan();

  unsigned int getNumberOfTimeSteps() const;
  unsigned int getCurrentTimeStep() const;

  void setCurrentTimeStep ( unsigned int num );

  AnimationResult stepBackward();
  AnimationResult stepForward();

  void increaseStepSize();
  void decreaseStepSize();

  void setStepSize ( unsigned int hours );

protected:

  virtual ~AnimationController();

private:

  void _setVisibleTimeSpan ( const Minerva::Core::Data::Date& begin, const Minerva::Core::Data::Date& end );

  TimeSpan::RefPtr _globalTimeSpan;
  TimeSpan::RefPtr _visibleTimeSpan;
  boost::posix_time::time_duration _stepAmount;

};

}
}

#endif // __MINERVA_DOCUMENT_ANIMATION_CONTROLLER_H__
