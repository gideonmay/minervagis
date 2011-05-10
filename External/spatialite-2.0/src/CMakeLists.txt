
ADD_DEFINITIONS ( "-DOMIT_GEOS -DOMIT_PROJ -D_CRT_SECURE_NO_WARNINGS ")

INCLUDE_DIRECTORIES ( "${CMAKE_CURRENT_SOURCE_DIR}/headers" ${SQLITE_INCLUDE_DIR} )

SET(SOURCES
./version.c
./gaiageo/gg_advanced.c
./gaiageo/gg_endian.c
./gaiageo/gg_geometries.c
./gaiageo/gg_relations.c
./gaiageo/gg_shape.c
./gaiageo/gg_transform.c
./gaiageo/gg_wkb.c
./gaiageo/gg_wkt.c
./headers/spatialite.h
./headers/spatialite/gaiageo.h
./headers/spatialite/rtree.h
./headers/spatialite/spatialite.h
./shell/shapefiles.c
./spatialite/rtree.c
./spatialite/spatialite.c
./spatialite/virtualshape.c	)

ADD_LIBRARY ( spatialite STATIC ${SOURCES} )
CADKIT_ADD_LIBRARY ( spatialite )

TARGET_LINK_LIBRARIES ( spatialite sqlite )
