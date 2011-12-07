
include ( MinervaFunctions )
include(FindPackageHandleStandardArgs)


# Try to find Collada
FIND_PATH(COLLADA_INCLUDE_DIR dae.h
  $ENV{COLLADA_INC_DIR}
)

# Find the library
#FIND_LIBRARY(COLLADA_LIBRARY
	#NAMES collada14dom
	#PATHS
	#$ENV{COLLADA_LIB_DIR}
	#DOC "The name of the COLLADA library"
#)

SET(COLLADA_LIBRARY collada14dom)
minerva_find_library(${COLLADA_LIBRARY} "$ENV{COLLADA_LIB_DIR}")

#FIND_PACKAGE_HANDLE_STANDARD_ARGS(collada14dom DEFAULT_MSG ${COLLADA_LIBRARY} COLLADA_INCLUDE_DIR)

IF (COLLADA_INCLUDE_DIR AND COLLADA14DOM_LIBRARY)
  SET(COLLADA_FOUND TRUE)
ENDIF (COLLADA_INCLUDE_DIR AND COLLADA14DOM_LIBRARY)

add_subdirectory ( Kml )
add_subdirectory ( AoIIntersection )

IF ( SPATIALITE_FOUND AND SQLITE_FOUND )
	add_subdirectory ( OSM )
ENDIF ( SPATIALITE_FOUND AND SQLITE_FOUND )	

add_subdirectory ( GeoRSS )
add_subdirectory ( GeoNames )

IF(APPLE)
  OPTION ( BUILD_GDAL_PLUGIN_AS_BUNDLE "Build Minerva GDAL plugins as a bundle." OFF )
ENDIF(APPLE)

# Build these projects if we found GDAL.
IF(GDAL_FOUND)
	add_subdirectory ( GDAL )
	add_subdirectory ( GDALReadImage )
ENDIF(GDAL_FOUND)
