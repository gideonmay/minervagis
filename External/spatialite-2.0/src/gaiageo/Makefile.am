
AM_CFLAGS = @OMIT_PROJ_FLAGS@ @OMIT_GEOS_FLAGS@

INCLUDES = @CFLAGS@
INCLUDES += -I$(top_srcdir)/src/headers

noinst_LTLIBRARIES = libgaiageo.la 

libgaiageo_la_SOURCES = \
     gg_endian.c \
     gg_geometries.c \
     gg_advanced.c \
     gg_wkb.c \
     gg_wkt.c \
     gg_transform.c \
     gg_shape.c \
     gg_relations.c

