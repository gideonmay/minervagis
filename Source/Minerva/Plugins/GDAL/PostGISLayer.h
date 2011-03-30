
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2006, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_LAYERS_POSTGIS_LAYER_H__
#define __MINERVA_LAYERS_POSTGIS_LAYER_H__

#include "Minerva/Plugins/GDAL/Export.h"
#include "Minerva/Plugins/GDAL/ConnectionInfo.h"
#include "Minerva/Plugins/GDAL/LabelData.h"

#include "Minerva/Core/Data/Date.h"
#include "Minerva/Core/Data/Container.h"
#include "Minerva/Core/Data/DataObject.h"
#include "Minerva/Core/Data/Geometry.h"

#include "Serialize/XML/Macros.h"

#include "Usul/Pointers/Pointers.h"
#include "Usul/Threads/Guard.h"
#include "Usul/Math/Vector2.h"

#include "osg/Vec4"

#ifdef _MSC_VER
#  pragma warning ( disable : 4561 )
#endif

#include <string>
#include <vector>
#include <iostream>

class OGRDataSource;
class OGRFeature;
class OGRGeometry;

namespace Minerva {
namespace Layers {
namespace GDAL {

  
class MINERVA_GDAL_EXPORT PostGISLayer : public Minerva::Core::Data::Container
{
public:
  
  /// Typedefs.
  typedef Minerva::Core::Data::Container            BaseClass;
  typedef Minerva::Core::Data::DataObject           DataObject;
  typedef Usul::Interfaces::IUnknown                IUnknown;
  typedef Minerva::Core::Data::Geometry             Geometry;
  typedef Minerva::Core::Data::Date              Date;
  typedef Minerva::Core::Data::Style                Style;

  /// Smart-pointer definitions.
  USUL_DECLARE_REF_POINTERS ( PostGISLayer );

  PostGISLayer();

  /// Clone this layer.
  virtual Minerva::Core::Data::Feature* clone() const;

  /// Get the extents.
  virtual Extents             calculateExtents();

  /// Get/Set the connection.
  void                        connection ( ConnectionInfo *connection );
  ConnectionInfo*             connection();
  const ConnectionInfo*       connection() const;
  
  /// Get/Set First date column name.
  void                        firstDateColumn ( const std::string& );
  const std::string&          firstDateColumn() const;

  /// Get/Set the tablename.
  void                        tablename ( const std::string& table );
  const std::string&          tablename() const;

  /// Get/Set the label column
  void                        labelColumn ( const std::string& column );
  const std::string&          labelColumn() const;

  /// Get/Set show label
  void                        showLabel ( bool b );
  bool                        showLabel() const;

  /// Get/Set last date column name.
  void                        lastDateColumn ( const std::string& );
  const std::string&          lastDateColumn() const;
  
  /// Get/Set the render bin.
  void                        renderBin ( Usul::Types::Uint32 bin );
  Usul::Types::Uint32         renderBin() const;

  /// Get/Set the alpha value.
  void                        alpha ( float a );
  float                       alpha () const;
  
  /// Serialize.
  virtual void                serialize ( XmlTree::Node &parent ) const;
  
  /// Set/get the style.
  void                        style ( Style::RefPtr style );
  Style::RefPtr               style() const;

  /// Get the projection as "Well Known Text".
  std::string                 projectionWKT();

  // Update.
  virtual void                updateNotify ( Minerva::Core::Data::CameraState* camera, 
                                             Minerva::Common::IPlanetCoordinates *planet, 
                                             Minerva::Common::IElevationDatabase *elevation );
  
  /// Set/get the updating state.
  void                        updating ( bool b );
  bool                        isUpdating() const;
  
protected:

  /// Use reference counting.
  virtual ~PostGISLayer();

  /// Copy constructor.
  PostGISLayer ( const PostGISLayer& layer );

  /// Build the data objects.
  virtual void                _buildDataObjects();
  
  void                        _connect();

  void                        _setDataObjectMembers ( DataObject* dataObject, OGRFeature* feature, OGRGeometry* geometry );

  /// Register members for serialization.
  void                        _registerMembers();

  void                        buildVectorData();
  void                        modifyVectorData();

private:

  OGRDataSource *_dataSource;
  std::string _tablename;
  std::string _labelColumn;
  Usul::Types::Uint32 _renderBin;
  ConnectionInfo::RefPtr _connection;
  LabelData::RefPtr            _labelData;
  float                        _alpha;
  bool                         _updating;
  std::string                  _firstDateColumn;
  std::string                  _lastDateColumn;
  Style::RefPtr                _style;

  SERIALIZE_XML_CLASS_NAME ( PostGISLayer );
};


}
}
}


#endif //__MINERVA_LAYERS_POSTGIS_LAYER_H__
