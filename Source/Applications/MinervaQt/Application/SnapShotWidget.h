
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author(s): Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __SNAP_SHOT_WIDGET_H__
#define __SNAP_SHOT_WIDGET_H__

#include "ui_SnapShot.h"

#include "QtGui/QWidget"

#include <vector>
#include <string>

class SnapShotWidget : public QWidget,
                       private Ui::SnapShot
{
  Q_OBJECT;
public:
  typedef QWidget BaseClass;
  typedef std::vector < std::string > Files;

  SnapShotWidget ( QWidget *parent = 0x0 );
  virtual ~SnapShotWidget();

signals:
  
  void takePicture ( QString filename, double frameScale, unsigned int numSamples );
  
protected:
  std::string      _filename();

private slots:

  void             on_snapShotButton_clicked();

private:

  std::string _lastFilename;
  unsigned int _count;
};


#endif // __SNAP_SHOT_WIDGET_H__
