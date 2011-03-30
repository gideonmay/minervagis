
#########################################################
#
#  CMakeLists for GDALReadImage.
#
#########################################################

INCLUDE ( Cadkit.plugin )

# ------------ Set Include Folders ----------------------
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR}
					 ${CADKIT_INC_DIR} 
                     ${OSG_INC_DIR}
                     ${GDAL_INCLUDE_DIR} )

SET (SOURCES
	./GDALReadImageComponent.cpp
	./GDALReadImageComponent.h
)

# Set variables that the CADKIT_ADD_PLUGIN macro uses.
SET ( PLUGIN_NAME "GDALReadImage" )
SET ( COMPILE_GUARD "_COMPILING_GDAL_READ_IMAGE" )
SET ( CADKIT_LIBRARIES Usul )
SET ( OTHER_LIBRARIES ${GDAL_LIBRARY} ${OSG_LIBRARY} ${OPENTHREADS_LIBRARY} )

# Add the plugin.
CADKIT_ADD_PLUGIN( ${PLUGIN_NAME} )
