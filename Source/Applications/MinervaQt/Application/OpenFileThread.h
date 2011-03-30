
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Thread to open file.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __OPEN_FILE_THREAD_H__
#define __OPEN_FILE_THREAD_H__

#include "QtCore/QThread"


class OpenFileThread : public QThread
{
  Q_OBJECT;
  
public:
  
  typedef QThread BaseClass;
  
  OpenFileThread ( const std::string& filename );
  
signals:
  
  void documentLoaded ( void* );
  
private:
  
  virtual void run();
  
  std::string _filename;
};

#endif // __OPEN_FILE_THREAD_H__
