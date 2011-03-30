
INCLUDE_DIRECTORIES( 
		     ${Boost_INCLUDE_DIR}
		     ${OSG_INC_DIR} 
		     )
		     
LINK_DIRECTORIES ( ${Boost_LIBRARY_DIRS} )

set ( SOURCES
./Main.cpp )

add_executable ( MakeFrames ${SOURCES} )

target_link_libraries ( MakeFrames
  ${OPENTHREADS_LIBRARY}
  ${OSG_LIBRARY}
  ${OSGDB_LIBRARY}
  ${OSGTEXT_LIBRARY}
  ${OSGUTIL_LIBRARY}
  ${OSGVIEWER_LIBRARY}
  ${Boost_LIBRARIES}
  ${Boost_PROGRAM_OPTIONS_LIBRARY} )

target_link_libraries ( MakeFrames Usul MinervaCore MinervaDocument )

SET_TARGET_PROPERTIES(MakeFrames PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

INSTALL(TARGETS MakeFrames
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION bin )
