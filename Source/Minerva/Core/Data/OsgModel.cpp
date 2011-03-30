
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Class for OpenSceneGraph model.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Data/OsgModel.h"

#include "boost/bind.hpp"

using namespace Minerva::Core::Data;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

OsgModel::OsgModel ( Model::RefPtr model, osg::Node* node ) : BaseClass(),
  _model ( model ),
  _node ( node ),
  _mutex(),
  _needsNewMatrix ( false ),
  _matrix()
{
  this->setDataVariance ( osg::Object::DYNAMIC );

  if ( _model )
  {
    _model->addElevationChangedListener ( boost::bind ( &OsgModel::elevationChangedNotify, this, _1, _2, _3, _4, _5 ) );
  }

  // This makes sure that the update visitor is always called.
  // This isn't optimal, but it fixes multi-threading issues.
  this->setNumChildrenRequiringUpdateTraversal ( this->getNumChildrenRequiringUpdateTraversal() + 1 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

OsgModel::~OsgModel()
{
  if ( _model )
  {
    _model->removeElevationChangedListener ( boost::bind ( &OsgModel::elevationChangedNotify, this, _1, _2, _3, _4, _5 ) );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Traverse this node.
//
///////////////////////////////////////////////////////////////////////////////

void OsgModel::traverse ( osg::NodeVisitor& nv )
{
  if ( osg::NodeVisitor::UPDATE_VISITOR == nv.getVisitorType() )
  {
    Usul::Threads::Guard<Usul::Threads::Mutex> guard ( _mutex );
    if ( _needsNewMatrix )
    {
      this->setMatrix ( _matrix );
      _needsNewMatrix = false;
      this->dirtyBound();
    }
  }

  BaseClass::traverse ( nv );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The elevation has changed.
//
///////////////////////////////////////////////////////////////////////////////

void OsgModel::elevationChangedNotify ( const Extents& extents, 
                                        unsigned int level, 
                                        ElevationDataPtr elevationData, 
                                        IPlanetCoordinates *planet, 
                                        IElevationDatabase* elevation )
{
  if ( _model )
  {
    Usul::Threads::Guard<Usul::Threads::Mutex> guard ( _mutex );
    _matrix = _model->matrix ( planet, elevation );
    _needsNewMatrix = true;
  }
}
