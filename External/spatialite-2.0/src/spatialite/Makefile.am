
AM_CFLAGS = @OMIT_PROJ_FLAGS@ @OMIT_GEOS_FLAGS@

INCLUDES = @CFLAGS@
INCLUDES += -I$(top_srcdir)/src/headers

noinst_LTLIBRARIES = libspatialite.la 

libspatialite_la_SOURCES = \
     sqlite3.c \
     rtree.c \
     virtualshape.c \
     spatialite.c

