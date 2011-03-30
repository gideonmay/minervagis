

SET(SOURCES ./sqlite3.c	)

ADD_LIBRARY ( sqlite STATIC ${SOURCES} )
CADKIT_ADD_LIBRARY ( sqlite )
