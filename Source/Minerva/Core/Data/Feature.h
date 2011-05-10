
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_CORE_DATA_FEATURE_H__
#define __MINERVA_CORE_DATA_FEATURE_H__

#include "Minerva/Core/Data/Object.h"
#include "Minerva/Core/Data/TimePrimitive.h"
#include "Minerva/Core/Data/LookAt.h"

#include "Minerva/Common/Extents.h"

#include "Usul/Interfaces/IUnknown.h"
#include "Usul/Threads/Atomic.h"
#include "Usul/Threads/Object.h"

#include "boost/function.hpp"
#include "boost/signals2/signal.hpp"

namespace Minerva { namespace Common { struct IPlanetCoordinates; struct IElevationDatabase; } }

namespace Minerva {
namespace Core {
  
class Visitor;
  
namespace Data {

class CameraState;
class Container;
class DataObject;

class MINERVA_EXPORT Feature : public Minerva::Core::Data::Object
{
  typedef Minerva::Core::Data::Object      BaseClass;
  typedef boost::signals2::signal<void ()> DataChangedListeners;
public:
  typedef Minerva::Core::Data::TimePrimitive  TimePrimitive;
  typedef Minerva::Common::Extents            Extents;
  typedef DataChangedListeners::slot_type ModifiedCallback;
  typedef boost::signals2::connection Connection;

  USUL_DECLARE_REF_POINTERS ( Feature );
  USUL_DECLARE_IUNKNOWN_MEMBERS;

  /// Get this as a container.
  virtual Container*     asContainer() { return 0x0; }

  /// Get this as a data object.
  virtual DataObject*    asDataObject() { return 0x0; }

  /// Accept the visitor.
  virtual void           accept ( Minerva::Core::Visitor& visitor );
  
  /// Clone this feature
  virtual Feature*       clone() const = 0;

  /// Set/get the description.
  void                   description ( const std::string& );
  std::string            description() const;

  /// Set/get the extents.
  void                   extents ( const Extents& e );
  Extents                extents() const;

  /// See if the given level falls within this layer's range of levels.
  virtual bool           isInLevelRange ( unsigned int level ) const;
  
  /// Set/get the look at.
  void                   lookAt ( LookAt* );
  LookAt*                lookAt() const;

  /// Set/get the name.
	void                   name ( const std::string& );
  std::string            name() const;

	/// Set/get the style url.
	void                   styleUrl ( const std::string& url );
	std::string            styleUrl() const;

  /// Set/get the time primitive.
  void                   timePrimitive ( TimePrimitive* );
  TimePrimitive*         timePrimitive() const;

  /// Set/get the visiblity.
	virtual void           visibilitySet ( bool b );
  bool                   visibility() const;
  
  // Add the listener.
  Connection             addDataChangedListener ( const ModifiedCallback& caller );
  
  // Remove the listener.
  void                   removeDataChangedListener ( const Connection& connection );
  
  // Get the number of children.
  virtual unsigned int        getNumChildNodes() const;
  
  // Get the child node.
  virtual Feature::RefPtr     getChildNode ( unsigned int which );

  /// Update.
  virtual void updateNotify ( CameraState* camera, Minerva::Common::IPlanetCoordinates *planet, Minerva::Common::IElevationDatabase *elevation );
  
protected:

  Feature();
  Feature ( const Feature& rhs );
  virtual ~Feature();

  /// Set the name.
  void                        _nameSet ( const std::string& name );

  /// Notify data changed listeners.
  void                        _notifyDataChangedListeners();
  
  /// Update the extents.
  void                        _updateExtents ( Feature *feature );
  void _expandExtents ( const Extents& extents );

private:

  typedef Usul::Threads::Object<std::string,Usul::Threads::MutexTraits<Usul::Threads::Mutex> > String;
  typedef Usul::Threads::Atomic<bool> Boolean;

  std::string _description;
  String _name;
	std::string _styleUrl;
  Boolean _visibility;
  LookAt::RefPtr _lookAt;
  TimePrimitive::RefPtr _timePrimitive;
  Extents _extents;
  DataChangedListeners _dataChangedListeners;
};


}
}
}

#endif // __MINERVA_CORE_DATA_FEATURE_H__
