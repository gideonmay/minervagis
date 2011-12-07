
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Gary Huber for AlphaPixel and Iowa State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __AOIINTERSECTION_H__
#define __AOIINTERSECTION_H__

#include "ogr_geometry.h"
#include "ogrsf_frmts.h"
#include "Export.h"

namespace AOI_Intersection {

typedef enum AOIIntersectionError
{
  no_error = 0,  // No errors
  NO_SPATIAL_REFERENCE = 1,  // Error creating Spatial Coordinate System for Acreage calculation
  NO_OUTPUT_DATASOURCE = 2,  // Error creating output data source
  NO_INPUT_LAYER = 3,  // No layer found in input data source
  NO_AOI_CLONE = 4,  // Error creating area of interest polygon clone
  NO_AOI_TRANSFORM = 5,  // Error creating coordinate transform for area of interest
  NO_ACRE_TRANSFORM = 6,  // Error creating coordinate transform for acreage calculation
  NO_OUTPUT_LAYER = 7,  // Error creating output layer
  NO_ACRE_FIELD = 8,  // Error creating output acreage field
  NO_AREA_FIELD = 9,  // Error creating output area percentage field
  NO_OUTPUT_FEATURE = 10,  // Error creating output feature
  NO_ACRE_OBJECT = 11,  // Error creating geometry object for acreage calculation
  NO_ACRE_TRANSFORMATION = 12,  // Error transforming acreage geometry object
  NO_FEATURE_ADDED = 13 // Error adding output feature to output layer
};

// Class for testing and reporting values of interest in bulk
class AOI_INTERSECTION_EXPORT IntersectionSummary
{
public:
  IntersectionSummary() : numLayers(0), totalInputPolyCt(0), intersectionCt(0), partEnclosedCt(0), aoiArea(0.0), totalIntersectionArea(0.0), totalInputPolyArea(0.0), totalPercentOfAoI(0.0), totalIntersectionAcres(0.0) {}

  int numLayers;
  unsigned int totalInputPolyCt,    // total number of polygons found in input file, all layers
      intersectionCt,               // number of polygons intersecting AoI, all layers
      partEnclosedCt;               // number of intersecting polygons that are partly but not wholly enclosed in AoI
  double aoiArea,                   // area of AoI in same units as input data file
      totalIntersectionArea,        // area of polygons intersected with AoI exclusive of parts that fall outside AoI in same units as input data file
      totalInputPolyArea,           // area of all input polygons in same units as input data file in same units as input data file
      totalPercentOfAoI,            // percent of AoI represented by intersected polygons
      totalIntersectionAcres;       // area of polygons intersected with AoI exclusive of parts that fall outside AoI in acres

};

// Class for performing the intersection of a vector polygon layer with an area of interest polygon.
// Builds an OGRDataSource to contain the intersection. Output geometry has fields that describe the acreage and percentage of the AoI polygon they represent
class AOI_INTERSECTION_EXPORT AoIIntersection
{

public:
  // constructor
  AoIIntersection() : lastError( no_error ) {}

  // Compares a source layer with an area of interest polygon and builds a new OGRDataSource to contain the overlapping elements
  OGRDataSource *intersectAoIWithLayers ( OGRDataSource *ogrDataSrc, OGRPolygon *aoiPoly, IntersectionSummary *summary, const char *outFmt );

  // Test to see if desired output format is available
  bool testFormatAvailable ( const char *outFmt );

  // Used for validation to make sure the results of the output OGRDataSource add up to the values calculated during output creation.
  // True means validation. False indicates that there is a discrepancy however small and it may be worth while to further investigate.
  bool compareIntersectionResults( OGRDataSource *resultData, IntersectionSummary *summary );

  // Deletes the OGRDataSource in the proper manner
  void destroyDataSource( OGRDataSource *dataSource );

  // Should an error occur the return will be one of the error codes listed above
  AOIIntersectionError fetchLastError( void )   { return lastError; }

  // Error codes translated into a readable message
  const char *fetchLastErrorMessage( void );

private:

  // Constructs the output data source
  OGRDataSource *buildIntersectionDataSource ( const char *outFmt );

  // Defines fields for our output geometry to use to store acreage and percentage of AoI
  OGRFeatureDefn *buildFeatureDefinition ( int& acreIndex, int& areaIndex, OGRFieldDefn*& acreFldDefn, OGRFieldDefn*& areaPctFldDefn );

  // Sets errors
  void setError( AOIIntersectionError err ) { lastError = err; }

  // Stores last set error
  AOIIntersectionError lastError;

};

}

#endif // __AOIINTERSECTION_H__
