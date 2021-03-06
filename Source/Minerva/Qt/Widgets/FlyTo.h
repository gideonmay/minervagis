
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_FLY_TO_H__
#define __MINERVA_FLY_TO_H__

#include "Minerva/Qt/Widgets/Export.h"

#include "QtGui/QWidget"

class QLineEdit;

namespace Minerva {
namespace QtWidgets {


class MINERVA_QT_WIDGETS_EXPORT FlyTo : public QWidget
{
  Q_OBJECT;
public:
  typedef QWidget BaseClass;
  
  FlyTo ( QWidget *parent );
  virtual ~FlyTo();
  
signals:
  
  void flyToLocation ( double lon, double lat );
  
protected slots:
  void _onFlyToClicked();
  
private:
  QLineEdit *_lineEdit;
};


}
}

#endif // __MINERVA_FLY_TO_H__
