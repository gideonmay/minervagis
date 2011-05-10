
# include base libraries.
ADD_SUBDIRECTORY ( Usul )
ADD_SUBDIRECTORY ( XmlTree )
ADD_SUBDIRECTORY ( Serialize/XML )

IF(SQLITE_FOUND)
	ADD_SUBDIRECTORY ( Database/SQLite )
ENDIF()

ADD_SUBDIRECTORY ( Minerva )

IF(QT_FOUND)
  ADD_SUBDIRECTORY ( Applications/MinervaQt )
ENDIF(QT_FOUND)
	

# Should we build the VR components?
IF(BUILD_MINERVA_VR)

	# Find VR Juggler
	INCLUDE( FindVRJuggler )

	ADD_SUBDIRECTORY(Applications/MinervaVR)
	
ENDIF(BUILD_MINERVA_VR)
