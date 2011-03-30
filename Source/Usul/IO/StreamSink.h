
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Redirect stdout and stderr to a functor.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _USUL_IO_STREAM_SINK_H_
#define _USUL_IO_STREAM_SINK_H_

#include "Usul/Config/Config.h"

#include "boost/iostreams/device/null.hpp"
#include "boost/iostreams/stream.hpp"

#include <iostream>


namespace Usul {
namespace IO {


///////////////////////////////////////////////////////////////////////////////
//
//  Simple stream sink that calls a functor.
//
///////////////////////////////////////////////////////////////////////////////

template < class F >
struct NullSink : public boost::iostreams::basic_null_sink < char >
{
  NullSink ( F f ) : _f ( f )
  {
  }
  std::streamsize write ( const char *s, std::streamsize n )
  {
    return _f ( s, n );
  }
private:
  F _f;
};


///////////////////////////////////////////////////////////////////////////////
//
//  Stream sink class that connects the functor to cout and cerr.
//
///////////////////////////////////////////////////////////////////////////////

template < class F > struct StreamSink
{
  typedef Usul::IO::NullSink < F > FunctorSink;
  typedef boost::iostreams::stream_buffer < FunctorSink > Buffer;

  StreamSink ( F f ) : _buffer()
  {
    _buffer.open ( FunctorSink ( f ) );
    std::cout.rdbuf ( &_buffer );
    std::cerr.rdbuf ( &_buffer );
  }

  ~StreamSink()
  {
    std::cout.rdbuf ( 0x0 );
    std::cerr.rdbuf ( 0x0 );
  }

private:

  Buffer _buffer;
};


///////////////////////////////////////////////////////////////////////////////
//
//  Helper function for creating a StreamSink.
//
///////////////////////////////////////////////////////////////////////////////

template < class F > StreamSink<F> makeStreamSink ( F f )
{
  return StreamSink<F>();
}


} // namespace IO
} // namespace Usul


#endif // _USUL_IO_STREAM_SINK_H_
