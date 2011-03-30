
#########################################################
#
#  CMakeLists for WmsLayerQt.
#
#########################################################

# Add include paths and librares for QT.
INCLUDE(${QT_USE_FILE})

# Set the link directories.
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

# ------------ Set Include Folders ----------------------
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR}
                     ${OSG_INC_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} )

# Set the sources.
SET (SOURCES
  WmsLayerQtComponent.h
  WmsLayerQtComponent.cpp
)

# Set variables that the CADKIT_ADD_PLUGIN macro uses.
SET ( PLUGIN_NAME "WmsLayerQt" )
SET ( SOURCES ${SOURCES} )
SET ( COMPILE_GUARD "_COMPILING_WMS_LAYER_QT" )
SET ( CADKIT_LIBRARIES Usul MinervaCore MinervaQtWidgets )
SET ( OTHER_LIBRARIES ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${QT_LIBRARIES} )

# Add the plugin.
CADKIT_ADD_PLUGIN ( ${PLUGIN_NAME} )
