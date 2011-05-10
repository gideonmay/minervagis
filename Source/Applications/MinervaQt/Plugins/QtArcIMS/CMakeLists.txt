
#########################################################
#
#  CMakeLists for WmsLayerQt.
#
#########################################################

INCLUDE ( Cadkit.plugin )

INCLUDE(${QT_USE_FILE})

# ------------ Set Include Folders ----------------------
INCLUDE_DIRECTORIES( ${OSG_INC_DIR}
                     ${QT_INCLUDE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} )

SET (SOURCES
  QtArcIMSComponent.cpp
  QtArcIMSComponent.h
)

# Set variables that the CADKIT_ADD_PLUGIN macro uses.
SET ( PLUGIN_NAME "QtArcIMS" )
SET ( COMPILE_GUARD "_COMPILING_ARC_IMS_LAYER_QT" )
SET ( CADKIT_LIBRARIES Usul MinervaCore MinervaQtWidgets )
SET ( OTHER_LIBRARIES ${QT_LIBRARIES} )

# Add the plugin.
CADKIT_ADD_PLUGIN ( ${PLUGIN_NAME} )
