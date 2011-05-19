
#Include the Top Level Directory
INCLUDE_DIRECTORIES( "${PROJECT_SOURCE_DIR}/../" ${Boost_INCLUDE_DIR} )

ADD_DEFINITIONS("-D_COMPILING_USUL")

#########################################################
#
#  Add Source groups.
#
#########################################################

SOURCE_GROUP ( Adaptors    REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Adaptors/* )
SOURCE_GROUP ( Algorithms  REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Algorithms/* )
SOURCE_GROUP ( App         REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/App/* )
SOURCE_GROUP ( Base        REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Base/* )
SOURCE_GROUP ( Bits        REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Bits/* )
SOURCE_GROUP ( Cast        REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Cast/* )
SOURCE_GROUP ( CommandLine REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/CommandLine/* )
SOURCE_GROUP ( Components  REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Components/* )
SOURCE_GROUP ( Config      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Config/* )
SOURCE_GROUP ( Console     REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Console/* )
SOURCE_GROUP ( Containers  REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Containers/* )
SOURCE_GROUP ( Convert     REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Convert/* )
SOURCE_GROUP ( DLL         REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/DLL/* )
SOURCE_GROUP ( Diagnostics REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Diagnostics/* )
SOURCE_GROUP ( Documents   REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Documents/* )
SOURCE_GROUP ( Endian      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Endian/* )
SOURCE_GROUP ( Errors      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Errors/* )
SOURCE_GROUP ( Exceptions  REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Exceptions/* )
SOURCE_GROUP ( Export      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Export/* )
SOURCE_GROUP ( Factory     REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Factory/* )
SOURCE_GROUP ( File        REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/File/* )
SOURCE_GROUP ( Functions   REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Functions/* )
SOURCE_GROUP ( Headers     REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Headers/* )
SOURCE_GROUP ( ID          REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/ID/* )
SOURCE_GROUP ( IO          REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/IO/* )
SOURCE_GROUP ( Interfaces  REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Interfaces/* )
SOURCE_GROUP ( Jobs        REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Jobs/* )
SOURCE_GROUP ( MPL         REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/MPL/* )
SOURCE_GROUP ( Math        REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Math/* )
SOURCE_GROUP ( Memory      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Memory/* )
SOURCE_GROUP ( Network     REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Network/* )
SOURCE_GROUP ( Pointers    REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Pointers/* )
SOURCE_GROUP ( Policies    REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Policies/* )
SOURCE_GROUP ( Predicates  REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Predicates/* )
SOURCE_GROUP ( Preprocess  REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Preprocess/* )
SOURCE_GROUP ( Print       REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Print/* )
SOURCE_GROUP ( Properties  REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Properties/* )
SOURCE_GROUP ( Registry    REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Registry/* )
SOURCE_GROUP ( Resources   REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Resources/* )
SOURCE_GROUP ( Scope       REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Scope/* )
SOURCE_GROUP ( Shared      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Shared/* )
SOURCE_GROUP ( Signals     REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Signals/* )
SOURCE_GROUP ( Strings     REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Strings/* )
SOURCE_GROUP ( System      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/System/* )
SOURCE_GROUP ( Threads     REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Threads/* )
SOURCE_GROUP ( Types       REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Types/* )
SOURCE_GROUP ( Units       REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/Units/* )
SOURCE_GROUP ( User        REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/User/* )


#########################################################
#
#  List all the headers
#
#########################################################

SET ( HEADERS
./Adaptors/Random.h
./Algorithms/Cylinder.h
./Algorithms/Sphere.h
./Algorithms/TriStrip.h
./App/Application.h
./Base/InstanceManager.h
./Base/Object.h
./Base/Referenced.h
./Bits/Bits.h
./Cast/Cast.h
./CommandLine/Arguments.h
./Components/Factory.h
./Components/Loader.h
./Components/Manager.h
./Config/Config.h
./Console/Feedback.h
./Containers/Vector.h
./Convert/Convert.h
./Convert/Generic.h
./Convert/Matrix44.h
./Convert/Vector2.h
./Convert/Vector3.h
./Convert/Vector4.h
./Diagnostics/StackTrace.h
./Diagnostics/StackTraceLinux.h
./Diagnostics/StackTraceMac.h
./Diagnostics/StackTraceWindows.h
./Diagnostics/Write.h
./Diagnostics/WriteLinux.h
./Diagnostics/WriteMac.h
./Diagnostics/WriteWindows.h
./DLL/Exceptions.h
./DLL/Library.h
./Documents/Document.h
./Documents/FileInfo.h
./Endian/Endian.h
./Errors/Assert.h
./Errors/AssertPolicy.h
./Errors/Checker.h
./Errors/CompositePolicy.h
./Errors/DoNothingPolicy.h
./Errors/InvalidParameter.h
./Errors/PrintingPolicy.h
./Errors/PureVirtualCall.h
./Errors/Signals.h
./Errors/ThrowingPolicy.h
./Exceptions/Canceled.h
./Exceptions/Catch.h
./Exceptions/Exception.h
./Exceptions/IO.h
./Exceptions/Thrower.h
./Exceptions/TimedOut.h
./Export/Export.h
./Factory/BaseCreator.h
./Factory/ObjectFactory.h
./Factory/RegisterCreator.h
./Factory/ScopedRegistration.h
./Factory/TypeCreator.h
./File/LineEnding.h
./File/Log.h
./File/Path.h
./File/Temp.h
./Functions/Color.h
./Functions/Execute.h
./Functions/GUID.h
./Functions/SafeCall.h
./Headers/OpenGL.h
./Headers/Qt.h
./Headers/SStream.h
./Interfaces/IAnimateMatrices.h
./Interfaces/ICancel.h
./Interfaces/ICanceledStateGet.h
./Interfaces/IFrameInfo.h
./Interfaces/ILog.h
./Interfaces/IPasswordPrompt.h
./Interfaces/IPlugin.h
./Interfaces/IProgressBar.h
./Interfaces/IStatusBar.h
./Interfaces/IUnknown.h
./Interfaces/Qt/IMainWindow.h
./IO/BinaryReader.h
./IO/BinaryWriter.h
./IO/Matrix44.h
./IO/Redirect.h
./IO/StreamSink.h
./IO/TextReader.h
./IO/TextWriter.h
./IO/Vector2.h
./IO/Vector3.h
./IO/Vector4.h
./Jobs/Job.h
./Jobs/Manager.h
./Math/Absolute.h
./Math/Angle.h
./Math/Barycentric.h
./Math/Constants.h
./Math/Equality.h
./Math/Finite.h
./Math/Functions.h
./Math/Interpolate.h
./Math/Matrix44.h
./Math/MinMax.h
./Math/NaN.h
./Math/Power.h
./Math/Quaternion.h
./Math/Transpose.h
./Math/Vector2.h
./Math/Vector3.h
./Math/Vector4.h
./MPL/SameType.h
./MPL/StaticAssert.h
./MPL/TypeWrap.h
./Pointers/BoostSerialize.h
./Pointers/DoNothingPolicy.h
./Pointers/Exceptions.h
./Pointers/Functions.h
./Pointers/Intrusive.h
./Pointers/NoReferencePolicy.h
./Pointers/NullPolicy.h
./Pointers/Pointers.h
./Pointers/QueryPointer.h
./Pointers/RefCountPolicy.h
./Pointers/SmartPointer.h
./Policies/Update.h
./Predicates/CloseFloat.h
./Predicates/EqualVector.h
./Predicates/LessVector.h
./Predicates/Tolerance.h
./Predicates/UnaryPair.h
./Print/Matrix.h
./Print/Node.h
./Print/Vector.h
./Registry/Constants.h
./Registry/Convert.h
./Registry/Database.h
./Registry/Node.h
./Registry/Qt.h
./Registry/Visitor.h
./Scope/Caller.h
./Scope/CurrentDirectory.h
./Scope/RemoveFile.h
./Scope/Reset.h
./Scope/StreamState.h
./Scope/Timer.h
./Signals/Actions.h
./Signals/Handler.h
./Strings/Case.h
./Strings/Format.h
./Strings/Qt.h
./Strings/Split.h
./Strings/Unicode.h
./System/Clock.h
./System/DateTime.h
./System/Directory.h
./System/Environment.h
./System/Host.h
./System/LastError.h
./System/Memory.h
./System/Sleep.h
./Threads/Guard.h
./Threads/Map.h
./Threads/Mutex.h
./Threads/MutexTraits.h
./Threads/Named.h
./Threads/Pool.h
./Threads/Queue.h
./Threads/ReadWriteLocks.h
./Threads/ReadWriteMutex.h
./Threads/RecursiveMutex.h
./Threads/Safe.h
./Threads/Set.h
./Threads/Task.h
./Threads/ThreadId.h
./Threads/Variable.h
./Types/Types.h
./User/Directory.h
)


#########################################################
#
#  List all the Sources.
#
#########################################################

SET (SOURCES
./App/Application.cpp
./Base/InstanceManager.cpp
./Base/Object.cpp
./Base/Referenced.cpp
./CommandLine/Arguments.cpp
./Components/Manager.cpp
./Console/Feedback.cpp
./DLL/Library.cpp
./Diagnostics/StackTrace.cpp
./Diagnostics/Write.cpp
./Documents/Document.cpp
./Errors/Assert.cpp
./Factory/ObjectFactory.cpp
./File/Log.cpp
./File/Temp.cpp
./Functions/GUID.cpp
./Jobs/Job.cpp
./Jobs/Manager.cpp
./Registry/Database.cpp
./Registry/Node.cpp
./Registry/Visitor.cpp
./Scope/CurrentDirectory.cpp
./Scope/Timer.cpp
./System/Clock.cpp
./System/DateTime.cpp
./System/Directory.cpp
./System/Environment.cpp
./System/Host.cpp
./System/LastError.cpp
./System/Memory.cpp
./System/Sleep.cpp
./Threads/Mutex.cpp
./Threads/Named.cpp
./Threads/Pool.cpp
./Threads/ReadWriteMutex.cpp
./Threads/RecursiveMutex.cpp
./Threads/Task.cpp
./Types/Types.cpp
./User/Directory.cpp)

SET(TARGET_NAME Usul)

# Create a Shared Library
ADD_LIBRARY( ${TARGET_NAME} SHARED ${HEADERS} ${SOURCES} )

# Add a library
CADKIT_ADD_LIBRARY ( ${TARGET_NAME} )

install ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include/cadkit
            PATTERN "*.h"
            PATTERN "*.cpp" EXCLUDE
            PATTERN ".svn" EXCLUDE
            PATTERN "Debug" EXCLUDE
            PATTERN "Release" EXCLUDE
            PATTERN "CMakeLists.txt" EXCLUDE
            PATTERN "*~" EXCLUDE
            PATTERN "*.vcproj" EXCLUDE )

# Find needed system libraries.	
IF(WIN32)
     SET ( SYSTEM_LIBRARIES User32 rpcrt4 Ws2_32 Dbghelp )
ELSEIF(APPLE)
     FIND_LIBRARY(CARBON_LIBRARY Carbon)
     SET ( SYSTEM_LIBRARIES System ${CARBON_LIBRARY} )
ELSE(WIN32)
     FIND_PATH ( UUID_INCLUDE_DIR uuid/uuid.h )
     FIND_LIBRARY ( UUID_LIBRARY uuid )
     SET ( SYSTEM_LIBRARIES dl ${UUID_LIBRARY} )
     include_directories ( ${UUID_INCLUDE_DIR} )
ENDIF(WIN32)

# Link the Library
TARGET_LINK_LIBRARIES( ${TARGET_NAME} ${SYSTEM_LIBRARIES} ${Boost_THREAD_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} )

# Create the config file.
CONFIGURE_FILE ( ${CMAKE_CURRENT_SOURCE_DIR}/Config/Config.h.in.cmake ${CMAKE_CURRENT_SOURCE_DIR}/Config/Config.h )

