
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Widget to edit PointStyle
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_QT_WIDGETS_POINTSTYLEWIDGET_H__
#define __MINERVA_QT_WIDGETS_POINTSTYLEWIDGET_H__

#include "Minerva/Core/Data/PointStyle.h"

#include <QWidget>

namespace Ui { class PointStyleWidget; }

namespace Minerva {
namespace QtWidgets {


class PointStyleWidget : public QWidget {
    Q_OBJECT
public:

  PointStyleWidget(QWidget *parent = 0);
  ~PointStyleWidget();

  void setPointStyle ( Minerva::Core::Data::PointStyle::RefPtr );
  Minerva::Core::Data::PointStyle::RefPtr getPointStyle();

private:
    Ui::PointStyleWidget *ui;
    Minerva::Core::Data::PointStyle::RefPtr _pointStyle;
};

}
}

#endif // __MINERVA_QT_WIDGETS_POINTSTYLEWIDGET_H__
