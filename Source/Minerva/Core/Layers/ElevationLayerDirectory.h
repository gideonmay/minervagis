
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Layer that treats whole directory as a single elevation layer.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_ELEVATION_LAYER_DIRECTORY_H__
#define __MINERVA_CORE_ELEVATION_LAYER_DIRECTORY_H__

#include "Minerva/Core/Export.h"
#include "Minerva/Core/Data/Container.h"

namespace osg { class Image; }

namespace Minerva {
namespace Core {
namespace Layers {

class MINERVA_EXPORT ElevationLayerDirectory : public Minerva::Core::Data::Container
{
public:
  
  typedef Minerva::Core::Data::Container BaseClass;
  
  USUL_DECLARE_REF_POINTERS ( ElevationLayerDirectory );
  
  ElevationLayerDirectory();
  
  /// Clone.
  virtual Feature*      clone() const;
  
  /// Deserialize.
  virtual void          deserialize ( const XmlTree::Node& node );
  
  /// Set/get the directory.
  void                  directory ( const std::string& );
  std::string           directory() const;
  
  /// Read the directory.
  void                  readDirectory();
  
  /// Serialize.
  virtual void          serialize ( XmlTree::Node &parent ) const;

protected:
  
  virtual ~ElevationLayerDirectory();
  
  ElevationLayerDirectory ( const ElevationLayerDirectory& );
  
  // Get the number of children (ITreeNode).
  virtual unsigned int            getNumChildNodes() const;
  
  // Get the child node (ITreeNode).
  virtual Feature::RefPtr       getChildNode ( unsigned int which );
  
private:

  // No assignment.
  ElevationLayerDirectory& operator= ( const ElevationLayerDirectory& );
  
  std::string _directory;
  
  SERIALIZE_XML_CLASS_NAME ( ElevationLayerDirectory );
};

} // namespace Layers
} // namespace Core
} // namespace Minerva

#endif // __MINERVA_CORE_ELEVATION_GROUP_H__
