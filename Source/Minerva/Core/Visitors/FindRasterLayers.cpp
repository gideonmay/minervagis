
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Visitors/FindRasterLayers.h"

using namespace Minerva::Core::Visitors;

FindRasterLayers::FindRasterLayers ( const Extents& extents, RasterLayers& container ) : BaseClass(),
  _extents ( extents ),
  _container ( container )
{
}


FindRasterLayers::~FindRasterLayers()
{
}


void FindRasterLayers::visit ( Minerva::Core::Layers::RasterLayer &raster )
{
  if ( raster.extents().intersects ( _extents ) )
  {
    _container.push_back ( &raster );
  }
}
