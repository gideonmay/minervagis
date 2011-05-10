
INCLUDE ( Cadkit.plugin )

add_subdirectory ( GeoRSSQt )

IF(POSTGRESQL_FOUND AND GDAL_FOUND )
	add_subdirectory(PostGISLayerQt)
ENDIF(POSTGRESQL_FOUND AND GDAL_FOUND )

add_subdirectory(QtArcIMS)
add_subdirectory(QtCustomGeometry)
add_subdirectory(WmsLayerQt)
