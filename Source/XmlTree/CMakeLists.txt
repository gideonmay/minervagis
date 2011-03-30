
#########################################################
#
#  CMakeLists for XmlTree.
#
#########################################################

ADD_DEFINITIONS("-D_COMPILING_XML_TREE")

# Add include folders.
INCLUDE_DIRECTORIES( ${XERCESC_INCLUDE_DIR} 
					 ${Boost_INCLUDE_DIR}
					 "${PROJECT_SOURCE_DIR}/../" )


# Add source directory.
SOURCE_GROUP ( Source REGULAR_EXPRESSION ./* )

# List all the headers.
SET ( HEADERS
	./Document.h
	./Export.h
	./Functions.h
	./Loader.h
	./Node.h
	./Predicates.h
	./RegistryBuilder.h
	./RegistryIO.h
	./RegistryVisitor.h
	./ReplaceIllegalCharacters.h
	./Writer.h
	./XercesErrorHandler.h
	./XercesLife.h
	./XercesString.h
)


# List all the sources.
SET (SOURCES
    Document.cpp
    Node.cpp
    Loader.cpp
    XercesLife.cpp
    Writer.cpp
    Functions.cpp
    RegistryBuilder.cpp
    RegistryIO.cpp
    RegistryVisitor.cpp
    ReplaceIllegalCharacters.cpp
)

SET ( TARGET_NAME XmlTree )

# Create a Shared Library
ADD_LIBRARY( ${TARGET_NAME} SHARED ${HEADERS} ${SOURCES} )

# Add a library
CADKIT_ADD_LIBRARY ( ${TARGET_NAME} )

INSTALL(FILES ${HEADERS} DESTINATION include/cadkit/XmlTree )

# Link the Library
LINK_CADKIT( ${TARGET_NAME} Usul )

TARGET_LINK_LIBRARIES( ${TARGET_NAME} ${XERCESC_LIBRARY} )

#INSTALL( FILES ${XERCESC_LIBRARY}
#  DESTINATION bin
#)
