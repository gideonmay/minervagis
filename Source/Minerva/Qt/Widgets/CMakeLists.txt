
INCLUDE ( Cadkit )

ADD_DEFINITIONS(${QT_DEFINITIONS})
INCLUDE(${QT_USE_FILE})

# ------------ Set Include Folders ----------------------
INCLUDE_DIRECTORIES( ${OSG_INC_DIR} 
                     ${QT_INCLUDE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${PROJECT_SOURCE_DIR}/External/haf/ )


# Add definitions
ADD_DEFINITIONS ("-D_COMPILING_MINERVA_QT_WIDGETS")

# List the Sources.
SET (SOURCES
./AddDirectoryDialog.cpp
./AddFileSystemWidget.cpp
./AlphasDialog.cpp
./AnimationControl.cpp
./BaseAddNetworkLayerWidget.cpp
./EditDataObjectDialog.cpp
./EditWmsLayerWidget.cpp
./Favorites.cpp
./FlyTo.cpp
./LayersTree.cpp
./OptionsDialog.cpp
./PointStyleWidget.cpp
./TreeControl.cpp
./TreeModel.cpp
./TreeNode.cpp
./WmsLayerItem.cpp
)

# List the headers.
SET ( HEADERS
./AddDirectoryDialog.h
./AddFileSystemWidget.h
./AddNetworkLayerWidget.h
./AlphasDialog.h
./AnimationControl.h
./BaseAddNetworkLayerWidget.h
./EditDataObjectDialog.h
./EditWmsLayerWidget.h
./Export.h
./Favorites.h
./FlyTo.h
./LayersTree.h
./OptionsDialog.h
./PointStyleWidget.h
./TreeControl.h
./TreeModel.h
./TreeNode.h
./WmsLayerItem.h
)

# These need to be moc'd.
SET(MOC_HDRS
  ./AddDirectoryDialog.h
  ./AddFileSystemWidget.h
  AnimationControl.h
  BaseAddNetworkLayerWidget.h
  AlphasDialog.h
  ./EditDataObjectDialog.h
  EditWmsLayerWidget.h
  OptionsDialog.h
  Favorites.h
  FlyTo.h
  LayersTree.h
  ./PointStyleWidget.h
  TreeControl.h
  TreeNode.h
)

SET(UIS
  ./AddDirectoryDialog.ui
  ./AddFileSystemWidget.ui
  ./AlphasDialog.ui
  ./AnimationControl.ui
  ./BaseAddNetworkLayerWidget.ui
  ./EditDataObjectDialog.ui
  ./EditWmsLayerWidget.ui
  ./Favorites.ui
  ./OptionsDialog.ui
  ./PointStyleWidget.ui
)

QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED )

QT4_WRAP_UI(UIS_HDRS ${UIS})

QT4_ADD_RESOURCES( RCC_SRCS MinervaQtWidgets.qrc )

# Create a Shared Library
ADD_LIBRARY( MinervaQtWidgets SHARED ${SOURCES} ${MOC_SRCS} ${UIS} ${UIS_HDRS} ${HEADERS} ${RCC_SRCS} )
CADKIT_ADD_LIBRARY ( MinervaQtWidgets )

install ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include/cadkit/Minerva/Qt
            PATTERN "*.h"	
            PATTERN "*.cpp" EXCLUDE
            PATTERN ".svn" EXCLUDE
            PATTERN "Debug" EXCLUDE
            PATTERN "Release" EXCLUDE
            PATTERN "CMakeLists.txt" EXCLUDE
            PATTERN "*~" EXCLUDE		  
            PATTERN "*.vcproj" EXCLUDE
			PATTERN "icons" EXCLUDE
			PATTERN "*.ui" EXCLUDE	
			PATTERN "*.qrc" EXCLUDE )

# Link the Library	
LINK_CADKIT( MinervaQtWidgets Usul MinervaCore MinervaDocument QtTools Helios )

TARGET_LINK_LIBRARIES( MinervaQtWidgets
   ${QT_LIBRARIES}
   ${Boost_REGEX_LIBRARY}
)

SOURCE_GROUP ( Source REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/* )
