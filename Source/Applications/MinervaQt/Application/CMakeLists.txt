
#########################################################
#
#  CMakeLists for Minerva Qt Application.
#
#########################################################

# Set the target name.
SET ( TARGET_NAME MinervaQt )

INCLUDE_DIRECTORIES ( ${OSG_INC_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR} 
  "${CADKIT_INC_DIR}/GenericNurbs"
  ${PROJECT_SOURCE_DIR}/External/haf )

IF(WIN32)
	ADD_DEFINITIONS ( "-D_CRT_SECURE_NO_DEPRECATE" )
ENDIF(WIN32)

# List the headers.
SET ( HEADERS
./AddFileSystemComponent.h
./Application.h
./Constants.h
./Format.h
./MainWindow.h
./MainWindowBase.h
./MatrixAnimationComponent.h
./OpenFileThread.h
./Program.h
./QPasswordPromptWidget.h
./SnapShotWidget.h
./SplashScreen.h
./Timer.h
./TimerImpl.h
./TimerFactory.h
./Viewer.h
)

#List the Sources
SET (SOURCES
./AddFileSystemComponent.cpp
./Application.cpp
./Format.cpp
./main.cpp
./MainWindow.cpp
./MainWindowBase.cpp
./MatrixAnimationComponent.cpp
./OpenFileThread.cpp
./Program.cpp
./QPasswordPromptWidget.cpp
./SnapShotWidget.cpp
./SplashScreen.cpp
./Timer.cpp
./TimerImpl.cpp
./TimerFactory.cpp
./Viewer.cpp
)

SET(MOC_HDRS
  MainWindow.h
  MainWindowBase.h
  ./MatrixAnimationComponent.h
  OpenFileThread.h
  TimerImpl.h
  Viewer.h
  QPasswordPromptWidget.h
  SnapShotWidget.h
)

QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})
QT4_ADD_RESOURCES( RCC_SRCS QMinervaResources.qrc )
QT4_WRAP_UI(UIS_HDRS MainWindow.ui SnapShot.ui )

# If we are building on Windows...
IF(WIN32)
	# Add the resource file for the icon.
    SET( SOURCES ${SOURCES} Resources.rc )
	
	# Add Unicode preprocessor definition.
	ADD_DEFINITIONS("-DUNICODE")
ENDIF(WIN32)

# Adds includes and libraries for QT.
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS( ${QT_DEFINITIONS} )

# Create the executable.
ADD_EXECUTABLE( ${TARGET_NAME} ${HEADERS} ${SOURCES} ${MOC_SRCS} ${RCC_SRCS} ${UIS_HDRS} )

# Note: Set target properties have to be called after ADD_EXECUTABLE.

# Settings so the console does not appear when running (See: http://www.cmake.org/Wiki/VSConfigSpecificSettings).
IF(WIN32)	
	SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS" )
	SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS" )
	SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS" )
	SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS" )
ENDIF(WIN32)

SET ( TARGET_OUTPUT_NAME "Minerva" )
SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES OUTPUT_NAME ${TARGET_OUTPUT_NAME} )

# Add the target label.
SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES PROJECT_LABEL "App ${TARGET_NAME}")

# Add the debug postfix.
SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

# Link the Library	
LINK_CADKIT( ${TARGET_NAME} Usul XmlTree QtTools MinervaDocument MinervaQtWidgets MinervaOsgTools Helios )

TARGET_LINK_LIBRARIES( ${TARGET_NAME}
   ${QT_LIBRARIES}
   ${QT_QTMAIN_LIBRARY}
   ${QT_QTOPENGL_LIBRARY}
   ${OPENTHREADS_LIBRARY}
   ${OSG_LIBRARY}
)

# --- If we are on OS X copy all the embedded libraries to the app bundle
IF (APPLE AND ${CMAKE_GENERATOR} MATCHES "Xcode")

	# Make an application bundle if we are on OS X
	SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES MACOSX_BUNDLE TRUE )

	SET ( APPLICATION_PATH_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/${TARGET_OUTPUT_NAME}.app )
	SET ( APPLICATION_PATH_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/${TARGET_OUTPUT_NAME}.app )

	# This is a hack.  When CMake runs, copy the plugin file to both Debug and Release app packages.
	CONFIGURE_FILE ( ${CMAKE_CURRENT_SOURCE_DIR}/Plugins.xml ${APPLICATION_PATH_RELEASE}/Contents/Plugins/Minerva.plugins )
	CONFIGURE_FILE ( ${CMAKE_CURRENT_SOURCE_DIR}/Plugins.xml ${APPLICATION_PATH_DEBUG}/Contents/Plugins/Minerva.plugins )
	
        MakeOSXBundleApp ( ${TARGET_OUTPUT_NAME} ${PROJECT_BINARY_DIR} )

ELSE()

	# Copy the plugin file.
	#CONFIGURE_FILE ( ${CMAKE_CURRENT_SOURCE_DIR}/Plugins.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/configs/Minerva.plugins )

	# Install the target name.
	INSTALL(TARGETS ${TARGET_NAME}
  			RUNTIME DESTINATION bin
			  LIBRARY DESTINATION bin
  			ARCHIVE DESTINATION bin
	)

ENDIF()
