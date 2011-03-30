
#########################################################
#
#  CMakeLists for QtGeoRSS.
#
#########################################################

INCLUDE ( Cadkit.plugin )

set(COMPILE_GUARD "_COMPILING_OSSIM_LAYER_QT")
ADD_DEFINITIONS ("-D${COMPILE_GUARD}")

# ------------ Set Include Folders ----------------------
INCLUDE_DIRECTORIES (  ${OSG_INC_DIR}  ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set (SOURCES
./AddGeoRSSLayerWidget.cpp
./GeoRSSQtComponent.cpp
./GeoRSSItemWidget.cpp
)

SET(MOC_HDRS
  ./GeoRSSItemWidget.h
  ./AddGeoRSSLayerWidget.h
)

SET ( UIS
./AddGeoRSSLayerWidget.ui
./GeoRSSItemWidget.ui
)

QT4_WRAP_UI( UIS_HDRS ${UIS} )

# Make the moc files.
QT4_WRAP_CPP( MOC_SRCS ${MOC_HDRS} )

# ---------- Create a Shared Library
SET(PLUGIN_NAME "QtGeoRSS")
SET ( SOURCES ${SOURCES} ${MOC_SRCS} ${UIS_HDRS} )
SET ( COMPILE_GUARD "_COMPILING_GEO_RSS_LAYER_QT" )
SET ( CADKIT_LIBRARIES Usul MinervaCore QtTools MinervaGeoRSS )
SET ( OTHER_LIBRARIES ${QT_LIBRARIES} ${Boost_LIBRARIES} ${POSTGRESQL_LIBRARIES} )

CADKIT_ADD_PLUGIN ( ${PLUGIN_NAME} )

# Link the Library
LINK_CADKIT( ${PLUGIN_NAME} Usul QtTools MinervaCore )

TARGET_LINK_LIBRARIES( ${PLUGIN_NAME} ${Boost_LIBRARIES} )
