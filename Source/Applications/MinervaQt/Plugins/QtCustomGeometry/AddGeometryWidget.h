
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author(s): Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __ADD_GEOMETRY_WIDGET_H__
#define __ADD_GEOMETRY_WIDGET_H__

#include "MinervaQt/Plugins/QtCustomGeometry/CompileGuard.h"

#include "ui_AddGeometryWidget.h"

#include "Usul/Interfaces/IUnknown.h"

#include "QtGui/QWidget"

namespace Minerva { namespace Core { namespace Data { class Feature; class Geometry; class Model; } } }
namespace osg { class Node; }
class QButtonGroup;

class AddGeometryWidget : public QWidget,
                          private Ui::AddGeometryWidget
{
  Q_OBJECT;
public:
  typedef QWidget BaseClass;
  typedef Minerva::Core::Data::Geometry Geometry;
  typedef Minerva::Core::Data::Model    Model;

  AddGeometryWidget ( QWidget *parent = 0x0 );
  virtual ~AddGeometryWidget();

  void             apply ( Minerva::Core::Data::Feature* parent );

private:

  void             _setMaterial ( osg::Node* ) const;

  Model*           _makeModel() const;

  Geometry*        _makeGeometry() const;
  Geometry*        _makeSphere() const;
  Geometry*        _makeTorus() const;
  Geometry*        _makeBox() const;
  Geometry*        _makeSquare() const;

  QButtonGroup *_group;
};


#endif // __ADD_GEOMETRY_WIDGET_H__
