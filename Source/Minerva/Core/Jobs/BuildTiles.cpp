
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Build four sub-tiles.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Jobs/BuildTiles.h"

#include "Usul/Convert/Convert.h"
#include "Usul/Pointers/Pointers.h"
#include "Usul/Threads/Safe.h"

using namespace Minerva::Core::Jobs;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

BuildTiles::BuildTiles ( Tile::RefPtr tile ) : BaseClass(),
  _tile ( tile ),
  _success ( false )
{
  if ( _tile.valid() )
  {
    const unsigned int level ( _tile->level() );
    this->priority ( -1 * static_cast<int> ( level ) );
  
    const Minerva::Core::TileEngine::Extents extents ( _tile->extents() );
    typedef Usul::Convert::Type<float,std::string> Converter;
    this->name ( Usul::Strings::format ( 
      "BuildTiles, Extents: [", 
      Converter::convert ( extents.minimum()[0] ), ", ", 
      Converter::convert ( extents.minimum()[1] ), ", ", 
      Converter::convert ( extents.maximum()[0] ), ", ", 
      Converter::convert ( extents.maximum()[1] ), "], level: ", 
      level ) );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

BuildTiles::~BuildTiles()
{
  _tile = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Build the sub-tiles.
//
///////////////////////////////////////////////////////////////////////////////

void BuildTiles::_started()
{
  // Make sure we have valid data.
  if ( false == _tile.valid() )
    return;
  
  // Have we been cancelled?
  if ( true == this->canceled() )
    this->cancel();

  // Ask the tile to split.
  _tile->split ( Usul::Jobs::Job::RefPtr ( this ) );

  // If we get to here it worked.
  Usul::Threads::Safe::set ( this->mutex(), true, _success );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Did it work?
//
///////////////////////////////////////////////////////////////////////////////

bool BuildTiles::success() const
{
  Guard guard ( this );
  return _success;
}
