
#########################################################
#
#  CMakeLists for Minerva Netwro library.
#
#########################################################

ADD_DEFINITIONS("-D_COMPILING_MINERVA_NETWORK")

IF(MSVC)
	ADD_DEFINITIONS("-DCURL_STATICLIB -D_CRT_SECURE_NO_WARNINGS")
ENDIF(MSVC)

LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

# ------------ Set Include Folders ----------------------
INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} 	${CURL_INCLUDE_DIR} )


#########################################################
#
#  List all the headers
#
#########################################################

SET ( HEADERS
./Export.h
./Download.h
./GeoCode.h
./Http.h
./Names.h
./WMS.h
)


#########################################################
#
#  List the sources.
#
#########################################################

SET (SOURCES
./Download.cpp
./GeoCode.cpp
./Http.cpp
./WMS.cpp
)

SET ( TARGET_NAME MinervaNetwork )

# Create a Shared Library
ADD_LIBRARY(${TARGET_NAME} SHARED ${SOURCES} ${HEADERS} )

# Add a library
CADKIT_ADD_LIBRARY ( ${TARGET_NAME} )

install ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include/cadkit/Minerva/
            PATTERN "*.h"	
            PATTERN "*.cpp" EXCLUDE
            PATTERN ".svn" EXCLUDE
            PATTERN "Debug" EXCLUDE
            PATTERN "Release" EXCLUDE
            PATTERN "CMakeLists.txt" EXCLUDE
            PATTERN "*~" EXCLUDE		  
            PATTERN "*.vcproj" EXCLUDE )
                    
# Link the Library
LINK_CADKIT( ${TARGET_NAME} Usul XmlTree )
TARGET_LINK_LIBRARIES(${TARGET_NAME}
	${Boost_DATE_TIME_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${CURL_LIBRARY}
)

IF(WIN32)
	TARGET_LINK_LIBRARIES( ${TARGET_NAME} Ws2_32 wldap32 )
ENDIF(WIN32)
