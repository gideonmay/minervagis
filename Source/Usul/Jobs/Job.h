
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002, Perry L. Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Job class.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_JOBS_JOB_CLASS_H_
#define _USUL_JOBS_JOB_CLASS_H_

#include "Usul/Base/Object.h"
#include "Usul/Interfaces/ICancel.h"
#include "Usul/Interfaces/ICanceledStateGet.h"

#include <iosfwd>

namespace Usul { namespace Jobs { class Manager; } }
namespace Usul { namespace Jobs { namespace Detail { class Task; } } }
namespace Usul { namespace Jobs { namespace Helper { class ScopedDone; } } }


namespace Usul {
namespace Jobs {


class USUL_EXPORT Job : public Usul::Base::Object,
                        public Usul::Interfaces::ICancel,
                        public Usul::Interfaces::ICanceledStateGet
{
public:

  // Useful typedefs.
  typedef Usul::Base::Object BaseClass;
  typedef Usul::Interfaces::IUnknown IUnknown;

  // Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( Job );

  // Usul::Interfaces::IUnknown members.
  USUL_DECLARE_IUNKNOWN_MEMBERS;

  // Cancel the job.
  virtual void              cancel();

  // Was the job canceled?
  virtual bool              canceled() const;

  // Return this job's id.
  unsigned long             id() const;

  // Is the job done?
  bool                      isDone() const;

  // Get/Set the priority.
  void                      priority( int );
  int                       priority() const;

  // Overload to return an accurate indication of success.
  virtual bool              success() const;

protected:

  // Constructors
  Job();

  // Use reference counting.
  virtual ~Job();

  // Called when the job is cancelled.
  virtual void              _cancelled();

  // Called when the job encounters an error.
  virtual void              _error();

  // Called when the job finishes normally.
  virtual void              _finished();

  // Set the id. Protected so only the Job Manager can set it.
  void                      _setId ( unsigned int value );

  // Called when the job starts.
  virtual void              _started();

private:

  typedef Usul::Jobs::Helper::ScopedDone ScopedDone;

  // No copying or assignment.
  Job ( const Job & );
  Job &operator = ( const Job & );

  void                      _destroy();

  void                      _setDone ( bool );

  void                      _threadCancelled();
  void                      _threadError    ();
  void                      _threadFinished ();
  void                      _threadStarted  ();

	// Use namespaces here, or gcc 4.0 will give an error.
  friend class Usul::Jobs::Detail::Task;
  friend class Usul::Jobs::Manager;
  friend class Helper::ScopedDone;

  unsigned long _id;
  bool _done;
  bool _canceled;
  int _priority;
};


///////////////////////////////////////////////////////////////////////////////
//
//  Template job class.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  template < class F > class GenericJob : public Job
  {
  public:

    typedef Job BaseClass;

    GenericJob ( F f ) : 
      BaseClass(),
      _f ( f )
    {
    }
  protected:

    virtual ~GenericJob()
    {
    }

  private:

    virtual void _started()
    {
      _f();
    }

    F _f;
  };
}


///////////////////////////////////////////////////////////////////////////////
//
//  Helper function to make template job class.
//
///////////////////////////////////////////////////////////////////////////////

template < class F > Job *create ( F f )
{
  return new Detail::GenericJob<F> ( f );
}


} // namespace Jobs
} // namespace Usul


#endif // _USUL_JOBS_JOB_CLASS_H_
