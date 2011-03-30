
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_LAYERS_GDAL_SHAPE_FILE_LAYER_H__
#define __MINERVA_LAYERS_GDAL_SHAPE_FILE_LAYER_H__

#include "Minerva/Core/Data/Container.h"
#include "Minerva/Core/Data/Geometry.h"
#include "Minerva/Core/Data/LineStyle.h"

class OGRLayer;


namespace Minerva {
namespace Layers {
namespace GDAL {
  
class OGRVectorLayer : public Minerva::Core::Data::Container
{
public:

  /// Typedefs.
  typedef Minerva::Core::Data::Container BaseClass;
  typedef Minerva::Core::Data::Geometry  Geometry;

  /// Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( OGRVectorLayer );
  
  OGRVectorLayer();

  // Read the file.
  virtual void read ( const std::string &filename, Usul::Interfaces::IUnknown *caller = 0x0, Usul::Interfaces::IUnknown *progress = 0x0 );

  // Deserialize/serialize.
  virtual void                deserialize( const XmlTree::Node &node );
  virtual void                serialize ( XmlTree::Node &parent ) const;

protected:

  virtual ~OGRVectorLayer();

  void      _addLayer ( OGRLayer* layer, Usul::Interfaces::IUnknown *progress );

private:
  
  std::string _filename;

  Minerva::Core::Data::Style::RefPtr _defaultStyle;
  double _verticalOffset;

  SERIALIZE_XML_CLASS_NAME ( OGRVectorLayer );
};

}
}
}

#endif // __MINERVA_LAYERS_GDAL_SHAPE_FILE_LAYER_H__
