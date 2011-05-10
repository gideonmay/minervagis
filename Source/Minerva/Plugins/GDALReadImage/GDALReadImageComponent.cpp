
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2005, Perry L Miller IV
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  The component class.
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/GDALReadImage/GDALReadImageComponent.h"

#include "Minerva/Plugins/GDAL/Convert.h"
#include "Minerva/Plugins/GDAL/Common.h"

#include "Usul/Components/Factory.h"
#include "Usul/File/Path.h"
#include "Usul/Scope/Caller.h"
#include "Usul/Strings/Case.h"

#include "boost/bind.hpp"

#include "gdal.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Initialize Gdal.
//
///////////////////////////////////////////////////////////////////////////////


namespace Detail
{
  struct Init
  {
    Init()
    {
      /// If there are no drivers for gdal, assume that it hasn't been initialized yet.
      if ( 0 == ::GDALGetDriverCount() )
      {
        ::GDALAllRegister();
      }
    }
    ~Init()
    {
      GDALDestroyDriverManager();
    }
  } _init;
}


#include <algorithm>

USUL_DECLARE_COMPONENT_FACTORY ( GDALReadImageComponent );
USUL_IMPLEMENT_IUNKNOWN_MEMBERS ( GDALReadImageComponent, GDALReadImageComponent::BaseClass );


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

GDALReadImageComponent::GDALReadImageComponent() : BaseClass()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

GDALReadImageComponent::~GDALReadImageComponent()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Query for the interface.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown *GDALReadImageComponent::queryInterface ( unsigned long iid )
{
  switch ( iid )
  {
  case Usul::Interfaces::IUnknown::IID:
  case Usul::Interfaces::IPlugin::IID:
    return static_cast < Usul::Interfaces::IPlugin * > ( this );
  case Minerva::Common::IReadImageFile::IID:
    return static_cast < Minerva::Common::IReadImageFile* > ( this );
  default:
    return 0x0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return true if this document reads given extension
//
///////////////////////////////////////////////////////////////////////////////

bool GDALReadImageComponent::canRead ( const std::string &file ) const
{
  const std::string ext ( Usul::Strings::lowerCase ( Usul::File::extension ( file ) ) );
  return ( ".jpg" == ext || ".gif" == ext || ".tif" == ext || ".tiff" == ext || ".png" == ext );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Read a filename and return an image (IReadImageFile).
//
///////////////////////////////////////////////////////////////////////////////

GDALReadImageComponent::ImagePtr GDALReadImageComponent::readImageFile ( const std::string& filename ) const
{
  SCOPED_GDAL_LOCK;
  
	// Open the file.
	GDALDataset *data ( static_cast<GDALDataset*> ( ::GDALOpen ( filename.c_str(), GA_ReadOnly ) ) );

	// Return if no data.
  if ( 0x0 == data )
    return 0x0;
  
	// Make sure data set is closed.
  Usul::Scope::Caller::RefPtr closeDataSet ( Usul::Scope::makeCaller ( boost::bind<void> ( GDALClose, boost::ref ( data ) ) ) );
  
  return ImagePtr ( Minerva::convert ( data ) );
}
