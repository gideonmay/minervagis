
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __MINERVA_FAVORITES_H__
#define __MINERVA_FAVORITES_H__

#include "Minerva/Qt/Widgets/Export.h"

#include "Minerva/Core/Data/Feature.h"

#include "Usul/Jobs/Job.h"
#include "Usul/Threads/Mutex.h"

#include "Serialize/XML/Macros.h"

#include "QtGui/QWidget"

#include <map>

namespace Helios { namespace Menus { class Menu; } }
namespace Ui { class Favorites; }

namespace Minerva {
namespace QtWidgets {


class MINERVA_QT_WIDGETS_EXPORT Favorites : 
  public QWidget
  
{
  Q_OBJECT
public:
  
  typedef QWidget BaseClass;
  typedef std::map < std::string, Minerva::Core::Data::Feature::RefPtr > FavoritesMap;
  typedef Helios::Menus::Menu Menu;
  
  /// Construction/Destruction.
  Favorites ( QWidget* parent = 0x0 );
  virtual ~Favorites();

  Menu*            menu ( Minerva::Core::Data::Feature *parent );

public slots:
  
  void             addLayer ( Minerva::Core::Data::Feature* layer );

private slots:
  
  void             _removeFavoriteButtonClicked();
  void             _onContextMenuShow ( const QPoint& pos );

  /// Build the tree.
  void             _buildTree();
  
private:

  void             _clear();

  /// Add a layer.
  void             _addLayer ( Minerva::Core::Data::Feature *parent, Minerva::Core::Data::Feature* layer );

  /// Build the menu.
  Menu*            _buildMenu ( const FavoritesMap& map, const std::string& name, Minerva::Core::Data::Feature *parent );

  /// Save/Restore state.
  void             _saveState();
  void             _restoreState();
  
  /// Get the filename.
  std::string      _filename() const;
  
  /// Read from server.
  void             _readFavoritesFromServer();

  FavoritesMap _serverFavorites;
  FavoritesMap _favoritesMap;
  Usul::Jobs::Job::RefPtr _downloadJob;
  Usul::Threads::Mutex _mutex;
  
  Ui::Favorites *_implementation;
  
  SERIALIZE_XML_DEFINE_MAP;
  SERIALIZE_XML_DEFINE_MEMBERS ( gis_favorites );
};


}
}

#endif // __MINERVA_FAVORITES_H__
