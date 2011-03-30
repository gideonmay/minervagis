
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/Kml/LoadModel.h"
#include "Minerva/Plugins/Kml/ModelPostProcess.h"
#include "Minerva/Core/Data/ModelCache.h"
#include "Minerva/Network/Download.h"

#include "Minerva/OsgTools/Visitor.h"
#include "Minerva/OsgTools/StateSet.h"

#include "Usul/Convert/Convert.h"
#include "Usul/File/Path.h"
#include "Usul/Strings/Case.h"
#include "Usul/Threads/Mutex.h"
#include "Usul/Threads/Guard.h"

#include "osg/BlendFunc"
#include "osg/Material" 
#include "osg/TexEnvCombine"

#include "osgDB/ReadFile"

#include "osgUtil/Optimizer"

#ifdef HAVE_COLLADA
# include "dae.h"
# include "dom/domCOLLADA.h"
#endif

using namespace Minerva::Layers::Kml;


///////////////////////////////////////////////////////////////////////////////
//
//  Typedefs.
//
///////////////////////////////////////////////////////////////////////////////

typedef Usul::Threads::Mutex Mutex;
typedef Usul::Threads::Guard<Mutex> Guard;


///////////////////////////////////////////////////////////////////////////////
//
//  Mutex to guard reading of collada files.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  Mutex _readMutex;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

LoadModel::LoadModel() : _toMeters ( 0.0254 )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Load the model.
//
///////////////////////////////////////////////////////////////////////////////

osg::Node* LoadModel::operator() ( const std::string& filename, ModelCache *cache )
{
  const std::string ext ( Usul::Strings::lowerCase ( Usul::File::extension ( filename ) ) );

  // If it's a collada file, pre-process.  (dae stands for Digital Asset Exchange.)
  if ( "dae" == ext )
  {
    this->_preProcessCollada ( filename );
  }

  if ( 0x0 != cache && cache->hasModel ( filename ) )
    return cache->model ( filename );

  Guard guard ( Detail::_readMutex );
  osg::ref_ptr<osg::Node> node ( osgDB::readNodeFile ( filename ) );
  if ( node.valid() )
  {
    // Post-process.
    {
      ModelPostProcess nv;
      osg::ref_ptr<osg::NodeVisitor> visitor ( OsgTools::MakeVisitor<osg::Node>::make ( nv ) );
      node->accept ( *visitor );
    }

    osg::ref_ptr<osg::StateSet> ss ( node->getOrCreateStateSet() );

    OsgTools::State::StateSet::setTwoSidedLighting ( ss.get(), true );
  }

  if ( 0x0 != cache )
    cache->addModel ( filename, node.get() );

  return node.release();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Preprocess the file.
//
///////////////////////////////////////////////////////////////////////////////

void LoadModel::_preProcessCollada ( const std::string& filename )
{
#ifdef HAVE_COLLADA
  // Make sure only one thread reads at a time.
  Guard guard ( Detail::_readMutex );
  
  DAE dae;

  // Open the file.
  daeSmartRef<domCOLLADA> dom ( dae.open ( filename ) );

  // Return if the file was not opened.
  if ( 0x0 == dom )
    return;

  typedef std::vector<daeElement*> Elements;
  
  // Get all the images.
  Elements elements ( dae.getDatabase()->typeLookup ( domImage::ID() ) );

  for ( Elements::iterator iter = elements.begin(); iter != elements.end(); ++iter )
  {
    daeSmartRef<domImage> image ( daeSafeCast<domImage> ( *iter ) );
    if ( 0x0 != image )
    {
      if ( 0x0 != image->getInit_from() )
      {
        const std::string protocol ( image->getInit_from()->getValue().getProtocol() );
        if ( "http" == protocol )
        {
          const std::string uri ( image->getInit_from()->getValue().str() );
          std::string filename;
          if ( Minerva::Network::download ( uri, filename, true ) )
          {
            filename = "file:" + cdom::nativePathToUri ( filename );
            image->getInit_from()->getValue().set ( filename );
          }
        }
      }
    }
  }

  // Get the asset.
  domAssetRef asset ( dom->getAsset() );
  if ( 0x0 != asset )
  {
    domAsset::domUnitRef unit ( asset->getUnit() );
    if ( 0x0 != unit )
    {
      this->toMeters ( unit->getMeter() );
    }
  }

  dae.write ( filename );
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the to meters conversion.
//
///////////////////////////////////////////////////////////////////////////////

void LoadModel::toMeters ( double amount )
{
  _toMeters = amount;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the to meters conversion.
//
///////////////////////////////////////////////////////////////////////////////

double LoadModel::toMeters() const
{
  return _toMeters;
}
