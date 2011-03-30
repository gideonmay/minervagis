
INCLUDE( FindVRJuggler22 )


################################################################
#
#  Find cppdom.
#
################################################################

FIND_LIBRARY( CPPDOM_LIB cppdom-1_0_0 cppdom 
  "$ENV{VJ_DEPS_DIR}/lib64"
  "$ENV{VJ_DEPS_DIR}/lib"
  "$ENV{CPPDOM_ROOT}/lib64"
  "$ENV{CPPDOM_ROOT}/lib"
  /usr/local/lib64
  /usr/local/lib
  /usr/lib64
  /usr/lib
)

# Look for a root installation
FIND_PATH(CPPDOM_INC_DIR cppdom/cppdom.h
	"$ENV{VJ_DEPS_DIR}/include/"
	"$ENV{CPPDOM_ROOT}/include/"
	PATH_SUFFIXES cppdom-1.0.0
	              cppdom-0.7.10
	DOC "The root of an installed cppdom installation"
)

IF(CPPDOM_INC_DIR AND CPPDOM_LIB)
	MESSAGE (STATUS "Found Cppdom: ${CPPDOM_LIB}")
	SET(VJ_DEPS_LIBS ${VJ_DEPS_LIBS} ${CPPDOM_LIB})
ENDIF(CPPDOM_INC_DIR AND CPPDOM_LIB)


# Look for gmtl installation
FIND_PATH( GMTL_INC_DIR gmtl/gmtl.h
	"$ENV{VJ_DEPS_DIR}/include/"
	"$ENV{GMTL_ROOT}/include/"
	PATH_SUFFIXES gmtl-0.6.0
	              gmtl-0.5.4
	DOC "The root of an installed cppdom installation"
)
