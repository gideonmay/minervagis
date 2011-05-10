

ADD_DEFINITIONS("-D_COMPILING_SERIALIZE_XML")

# Set Include Folders
INCLUDE_DIRECTORIES( ${XERCESC_INCLUDE_DIR} "${PROJECT_SOURCE_DIR}/../../" )

SET ( HEADERS
./BuiltInType.h
./DataMemberMap.h
./Deserialize.h
./Export.h
./Macros.h
./MemberBase.h
./PointerMapMember.h
./SequenceMember.h
./Serialize.h
./SimpleDataMember.h
./SmartPointer.h
./SmartPointerMember.h
./StandardString.h
./TypeWrapper.h
./ValueMapMember.h
)

#List the Sources
SET (SOURCES
    DataMemberMap.cpp
    MemberBase.cpp
)

# Create a Shared Library
ADD_LIBRARY(SerializeXML SHARED ${SOURCES} ${HEADERS} )
CADKIT_ADD_LIBRARY(SerializeXML)

INSTALL(FILES ${HEADERS} DESTINATION include/cadkit/Serialize/XML )

# Link the Library
LINK_CADKIT(SerializeXML Usul XmlTree )

TARGET_LINK_LIBRARIES(SerializeXML
	${XERCESC_LIBRARY}
)

