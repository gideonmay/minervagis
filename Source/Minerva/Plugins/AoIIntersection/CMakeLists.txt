
#########################################################
#
#  CMakeLists for AoI Intersection.
#
#########################################################

# Set link directories.
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

# Set include directories.

INCLUDE_DIRECTORIES( 
			 ${Boost_INCLUDE_DIR}
		     ${CADKIT_INC_DIR}
             ${GDAL_INCLUDE_DIR}
		     ${OSG_INC_DIR} )

# List the headers
SET ( HEADERS
	./AoIIntersection.h
	./Export.h
)
			 
# List the Sources
SET (SOURCES
	./AoIIntersection.cpp
)

ADD_DEFINITIONS ("-D_COMPILING_AOI_INTERSECTION")

SET ( TARGET_NAME AoIIntersection )

# Create a Shared Library
ADD_LIBRARY( ${TARGET_NAME} SHARED ${HEADERS} ${SOURCES} )
CADKIT_ADD_LIBRARY ( ${TARGET_NAME} )

# Link the Library	
LINK_CADKIT( ${TARGET_NAME}  )
TARGET_LINK_LIBRARIES( ${TARGET_NAME}
  ${Boost_LIBRARIES}
  ${OPENTHREADS_LIBRARY}
  ${GDAL_LIBRARY}
  ${OSG_LIBRARY}
  ${OSGTEXT_LIBRARY}
)
