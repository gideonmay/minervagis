
#########################################################
#
#  CMakeLists for Minerva GeoNames.
#
#########################################################

# Set link directories.
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

# Set include directories.

INCLUDE_DIRECTORIES( 
			 ${Boost_INCLUDE_DIR}
		     ${CADKIT_INC_DIR}
		     ${OSG_INC_DIR} )

# List the headers
SET ( HEADERS
	./City.h
	./CityLayer.h
	./Export.h
)
			 
# List the Sources
SET (SOURCES
	./City.cpp
	./CityLayer.cpp
)

ADD_DEFINITIONS ("-D_COMPILING_MINERVA_GEO_NAMES")

SET ( TARGET_NAME MinervaGeoNames )

# Create a Shared Library
ADD_LIBRARY( ${TARGET_NAME} SHARED ${HEADERS} ${SOURCES} )
CADKIT_ADD_LIBRARY ( ${TARGET_NAME} )

# Link the Library	
LINK_CADKIT( ${TARGET_NAME} Usul MinervaOsgTools SerializeXML XmlTree MinervaCore MinervaNetwork )
TARGET_LINK_LIBRARIES( ${TARGET_NAME}
	${Boost_LIBRARIES}
  ${OPENTHREADS_LIBRARY}
  ${OSG_LIBRARY}
  ${OSGTEXT_LIBRARY}
)
