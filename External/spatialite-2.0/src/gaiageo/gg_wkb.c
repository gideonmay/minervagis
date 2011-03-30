/*

 gg_wkb.c -- Gaia common support for WKB encoded geometries
  
 version 1.0, 2008 May 6

 Author: Sandro Furieri a-furieri@lqt.it

 Copyright (C) 2008  Alessandro Furieri

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 
*/

#include <stdlib.h>
#include <stdio.h>

#include <spatialite/gaiageo.h>

static void
ParseWkbPoint (gaiaGeomCollPtr geo)
{
/* decodes a POINT from WKB */
    double x;
    double y;
    if (geo->size < geo->offset + 16)
	return;
    x = gaiaImport64 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
		      geo->endian_arch);
    geo->offset += 16;
    gaiaAddPointToGeomColl (geo, x, y);
}

static void
ParseWkbLine (gaiaGeomCollPtr geo)
{
/* decodes a LINESTRING from WKB */
    int points;
    int iv;
    double x;
    double y;
    gaiaLinestringPtr line;
    if (geo->size < geo->offset + 4)
	return;
    points =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    if (geo->size < geo->offset + (16 * points))
	return;
    line = gaiaAddLinestringToGeomColl (geo, points);
    for (iv = 0; iv < points; iv++)
      {
	  x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
			    geo->endian_arch);
	  gaiaSetPoint (line->Coords, iv, x, y);
	  geo->offset += 16;
      }
}

static void
ParseWkbPolygon (gaiaGeomCollPtr geo)
{
/* decodes a POLYGON from WKB */
    int rings;
    int nverts;
    int iv;
    int ib;
    double x;
    double y;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring;
    if (geo->size < geo->offset + 4)
	return;
    rings =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ib = 0; ib < rings; ib++)
      {
	  if (geo->size < geo->offset + 4)
	      return;
	  nverts =
	      gaiaImport32 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  geo->offset += 4;
	  if (geo->size < geo->offset + (16 * nverts))
	      return;
	  if (ib == 0)
	    {
		polyg = gaiaAddPolygonToGeomColl (geo, nverts, rings - 1);
		ring = polyg->Exterior;
	    }
	  else
	      ring = gaiaAddInteriorRing (polyg, ib - 1, nverts);
	  for (iv = 0; iv < nverts; iv++)
	    {
		x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
				  geo->endian_arch);
		y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
				  geo->endian_arch);
		geo->offset += 16;
		gaiaSetPoint (ring->Coords, iv, x, y);
	    }
      }
}

static void
ParseWkbGeometry (gaiaGeomCollPtr geo)
{
/* decodes a MULTIxx or GEOMETRYCOLLECTION from SpatiaLite BLOB */
    int entities;
    int type;
    int ie;
    if (geo->size < geo->offset + 4)
	return;
    entities =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ie = 0; ie < entities; ie++)
      {
	  if (geo->size < geo->offset + 5)
	      return;
	  type =
	      gaiaImport32 (geo->blob + geo->offset + 1, geo->endian,
			    geo->endian_arch);
	  geo->offset += 5;
	  switch (type)
	    {
	    case GAIA_POINT:
		ParseWkbPoint (geo);
		break;
	    case GAIA_LINESTRING:
		ParseWkbLine (geo);
		break;
	    case GAIA_POLYGON:
		ParseWkbPolygon (geo);
		break;
	    default:
		break;
	    };
      }
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromSpatiaLiteBlobWkb (const unsigned char *blob, unsigned int size)
{
/* decoding from SpatiaLite BLOB to GEOMETRY */
    int type;
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    gaiaGeomCollPtr geo = NULL;
    if (size < 45)
	return NULL;		/* cannot be an internal BLOB WKB geometry */
    if (*(blob + 0) != GAIA_MARK_START)
	return NULL;		/* failed to recognize START signature */
    if (*(blob + (size - 1)) != GAIA_MARK_END)
	return NULL;		/* failed to recognize END signature */
    if (*(blob + 38) != GAIA_MARK_MBR)
	return NULL;		/* failed to recognize MBR signature */
    if (*(blob + 1) == GAIA_LITTLE_ENDIAN)
	little_endian = 1;
    else if (*(blob + 1) == GAIA_BIG_ENDIAN)
	little_endian = 0;
    else
	return NULL;		/* unknown encoding; nor litte-endian neither big-endian */
    type = gaiaImport32 (blob + 39, little_endian, endian_arch);
    geo = gaiaAllocGeomColl ();
    geo->Srid = gaiaImport32 (blob + 2, little_endian, endian_arch);
    geo->endian_arch = endian_arch;
    geo->endian = little_endian;
    geo->blob = blob;
    geo->size = size;
    geo->offset = 43;
    switch (type)
      {
      case GAIA_POINT:
	  ParseWkbPoint (geo);
	  break;
      case GAIA_LINESTRING:
	  ParseWkbLine (geo);
	  break;
      case GAIA_POLYGON:
	  ParseWkbPolygon (geo);
	  break;
      case GAIA_MULTIPOINT:
      case GAIA_MULTILINESTRING:
      case GAIA_MULTIPOLYGON:
      case GAIA_GEOMETRYCOLLECTION:
	  ParseWkbGeometry (geo);
	  break;
      default:
	  break;
      };
    geo->MinX = gaiaImport64 (blob + 6, little_endian, endian_arch);
    geo->MinY = gaiaImport64 (blob + 14, little_endian, endian_arch);
    geo->MaxX = gaiaImport64 (blob + 22, little_endian, endian_arch);
    geo->MaxY = gaiaImport64 (blob + 30, little_endian, endian_arch);
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromSpatiaLiteBlobMbr (const unsigned char *blob, unsigned int size)
{
/* decoding from SpatiaLite BLOB to GEOMETRY [MBR only] */
    int type;
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    double minx;
    double miny;
    double maxx;
    double maxy;
    gaiaGeomCollPtr geo = NULL;
    gaiaPolygonPtr polyg;
    gaiaRingPtr ring;
    if (size < 45)
	return NULL;		/* cannot be an internal BLOB WKB geometry */
    if (*(blob + 0) != GAIA_MARK_START)
	return NULL;		/* failed to recognize START signature */
    if (*(blob + (size - 1)) != GAIA_MARK_END)
	return NULL;		/* failed to recognize END signature */
    if (*(blob + 38) != GAIA_MARK_MBR)
	return NULL;		/* failed to recognize MBR signature */
    if (*(blob + 1) == GAIA_LITTLE_ENDIAN)
	little_endian = 1;
    else if (*(blob + 1) == GAIA_BIG_ENDIAN)
	little_endian = 0;
    else
	return NULL;		/* unknown encoding; nor litte-endian neither big-endian */
    type = gaiaImport32 (blob + 39, little_endian, endian_arch);
    geo = gaiaAllocGeomColl ();
    polyg = gaiaAddPolygonToGeomColl (geo, 5, 0);
    ring = polyg->Exterior;
    minx = gaiaImport64 (blob + 6, little_endian, endian_arch);
    miny = gaiaImport64 (blob + 14, little_endian, endian_arch);
    maxx = gaiaImport64 (blob + 22, little_endian, endian_arch);
    maxy = gaiaImport64 (blob + 30, little_endian, endian_arch);
    gaiaSetPoint (ring->Coords, 0, minx, miny);	/* vertex # 1 */
    gaiaSetPoint (ring->Coords, 1, maxx, miny);	/* vertex # 2 */
    gaiaSetPoint (ring->Coords, 2, maxx, maxy);	/* vertex # 3 */
    gaiaSetPoint (ring->Coords, 3, minx, maxy);	/* vertex # 4 */
    gaiaSetPoint (ring->Coords, 4, minx, miny);	/* vertex # 5 [same as vertex # 1 to close the polygon] */
    return geo;
}

GAIAGEO_DECLARE void
gaiaToSpatiaLiteBlobWkb (gaiaGeomCollPtr geom, unsigned char **result,
			 int *size)
{
/* builds the SpatiaLite BLOB representation for this GEOMETRY */
    int ib;
    int iv;
    double x;
    double y;
    int entities = 0;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    int type;
    unsigned char *ptr;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaPointPtr point = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    int endian_arch = gaiaEndianArch ();
    gaiaMbrGeometry (geom);
/* how many entities, and of what kind, do we have ? */
    pt = geom->FirstPoint;
    while (pt)
      {
	  point = pt;
	  entities++;
	  n_points++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  line = ln;
	  entities++;
	  n_linestrings++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  polyg = pg;
	  entities++;
	  n_polygons++;
	  pg = pg->Next;
      }
    *size = 0;
    *result = NULL;
    if (n_points == 0 && n_polygons == 0 && n_linestrings == 0)
	return;
/* ok, we can determine the geometry class */
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
	type = GAIA_POINT;
    else if (n_points > 1 && n_linestrings == 0 && n_polygons == 0)
	type = GAIA_MULTIPOINT;
    else if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
	type = GAIA_LINESTRING;
    else if (n_points == 0 && n_linestrings > 1 && n_polygons == 0)
	type = GAIA_MULTILINESTRING;
    else if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
	type = GAIA_POLYGON;
    else if (n_points == 0 && n_linestrings == 0 && n_polygons > 1)
	type = GAIA_MULTIPOLYGON;
    else
	type = GAIA_GEOMETRYCOLLECTION;
/* and now we compute the size of BLOB */
    *size = 44;			/* header size */
    if (type == GAIA_POINT)
	*size += (sizeof (double) * 2);	/* two points */
    else if (type == GAIA_LINESTRING)
	*size += (4 + ((sizeof (double) * 2) * line->Points));	/* # points + [x,y] for each vertex */
    else if (type == GAIA_POLYGON)
      {
	  rng = polyg->Exterior;
	  *size += (8 + ((sizeof (double) * 2) * rng->Points));	/* # rings + # points + [x.y] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + ((sizeof (double) * 2) * rng->Points));	/* # points + [x,y] array - interior ring */
	    }
      }
    else
      {
	  /* this one is not a simple geometry; should be a MULTIxxxx or a GEOMETRYCOLLECTION */
	  *size += 4;		/* # entities */
	  point = geom->FirstPoint;
	  while (point)
	    {
		*size += 5;	/* entity header */
		*size += (sizeof (double) * 2);	/* two doubles for each POINT */
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		*size += 5;	/* entity header */
		*size += (4 + ((sizeof (double) * 2) * line->Points));	/* # points + [x,y] for each vertex */
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		*size += 5;	/* entity header */
		rng = polyg->Exterior;
		*size += (8 + ((sizeof (double) * 2) * rng->Points));	/* # rings + # points + [x.y] array - exterior ring */
		for (ib = 0; ib < polyg->NumInteriors; ib++)
		  {
		      rng = polyg->Interiors + ib;
		      *size += (4 + ((sizeof (double) * 2) * rng->Points));	/* # points + [x,y] array - interior ring */
		  }
		polyg = polyg->Next;
	    }
      }
    *result = malloc (*size);
    ptr = *result;
/* and finally we build the BLOB */
    if (type == GAIA_POINT)
      {
	  *ptr = GAIA_MARK_START;	/* START signatue */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimun X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimun Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximun X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximun Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POINT, 1, endian_arch);	/* class POINT */
	  gaiaExport64 (ptr + 43, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 51, point->Y, 1, endian_arch);	/* Y */
	  *(ptr + 59) = GAIA_MARK_END;	/* END signature */
      }
    else if (type == GAIA_LINESTRING)
      {
	  *ptr = GAIA_MARK_START;	/* START signatue */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimun X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimun Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximun X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximun Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_LINESTRING, 1, endian_arch);	/* class LINESTRING */
	  gaiaExport32 (ptr + 43, line->Points, 1, endian_arch);	/* # points */
	  ptr += 47;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, 1, endian_arch);
		gaiaExport64 (ptr + 8, y, 1, endian_arch);
		ptr += 16;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
      }
    else if (type == GAIA_POLYGON)
      {
	  *ptr = GAIA_MARK_START;	/* START signatue */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimun X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimun Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximun X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximun Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POLYGON, 1, endian_arch);	/* class POLYGON */
	  gaiaExport32 (ptr + 43, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 47, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 51;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPoint (rng->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		ptr += 16;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
		      ptr += 16;
		  }
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
      }
    else
      {
	  /* this one is a MULTIxxxx or a GEOMETRYCOLLECTION - building the main header */
	  *ptr = GAIA_MARK_START;	/* START signatue */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimun X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimun Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximun X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximun Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, type, 1, endian_arch);	/* geometric class */
	  gaiaExport32 (ptr + 43, entities, 1, endian_arch);	/* # entities */
	  ptr += 47;
	  point = geom->FirstPoint;
	  while (point)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		gaiaExport32 (ptr + 1, GAIA_POINT, 1, endian_arch);	/* class POINT */
		gaiaExport64 (ptr + 5, point->X, 1, endian_arch);	/* X */
		gaiaExport64 (ptr + 13, point->Y, 1, endian_arch);	/* Y */
		ptr += 21;
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		gaiaExport32 (ptr + 1, GAIA_LINESTRING, 1, endian_arch);	/* class LINESTRING */
		gaiaExport32 (ptr + 5, line->Points, 1, endian_arch);	/* # points */
		ptr += 9;
		for (iv = 0; iv < line->Points; iv++)
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y */
		      ptr += 16;
		  }
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		gaiaExport32 (ptr + 1, GAIA_POLYGON, 1, endian_arch);	/* class POLYGON */
		gaiaExport32 (ptr + 5, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
		rng = polyg->Exterior;
		gaiaExport32 (ptr + 9, rng->Points, 1, endian_arch);	/* # points - exterior ring */
		ptr += 13;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		      ptr += 16;
		  }
		for (ib = 0; ib < polyg->NumInteriors; ib++)
		  {
		      rng = polyg->Interiors + ib;
		      gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		      ptr += 4;
		      for (iv = 0; iv < rng->Points; iv++)
			{
			    gaiaGetPoint (rng->Coords, iv, &x, &y);
			    gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
			    gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
			    ptr += 16;
			}
		  }
		*ptr = GAIA_MARK_END;	/* END signature */
		polyg = polyg->Next;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
      }
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromWkb (const unsigned char *blob, unsigned int size)
{
/* decoding from WKB to GEOMETRY  */
    int type;
    int little_endian;
    gaiaGeomCollPtr geo = NULL;
    int endian_arch = gaiaEndianArch ();
    if (size < 5)
	return NULL;
    if (*(blob + 0) == 0x01)
	little_endian = GAIA_LITTLE_ENDIAN;
    else
	little_endian = GAIA_BIG_ENDIAN;
    type = gaiaImport32 (blob + 1, little_endian, endian_arch);
    geo = gaiaAllocGeomColl ();
    geo->Srid = -1;
    geo->endian_arch = endian_arch;
    geo->endian = little_endian;
    geo->blob = blob;
    geo->size = size;
    geo->offset = 5;
    switch (type)
      {
      case GAIA_POINT:
	  ParseWkbPoint (geo);
	  break;
      case GAIA_LINESTRING:
	  ParseWkbLine (geo);
	  break;
      case GAIA_POLYGON:
	  ParseWkbPolygon (geo);
	  break;
      case GAIA_MULTIPOINT:
      case GAIA_MULTILINESTRING:
      case GAIA_MULTIPOLYGON:
      case GAIA_GEOMETRYCOLLECTION:
	  ParseWkbGeometry (geo);
	  break;
      default:
	  break;
      };
    gaiaMbrGeometry (geo);
    return geo;
}

GAIAGEO_DECLARE char *
gaiaToHexWkb (gaiaGeomCollPtr geom)
{
/* builds the hexadecimal WKB representation for this GEOMETRY */
    unsigned char *wkb = NULL;
    int size = 0;
    char *hexbuf = NULL;
    int i;
    char hex[16];
    char *p;
    gaiaToWkb (geom, &wkb, &size);
    if (!wkb)
	return NULL;
    hexbuf = malloc ((size * 2) + 1);
    p = hexbuf;
    for (i = 0; i < size; i++)
      {
	  sprintf (hex, "%02X", *(wkb + i));
	  *p++ = hex[0];
	  *p++ = hex[1];
      }
    *p = '\0';
    return hexbuf;
}

GAIAGEO_DECLARE void
gaiaToWkb (gaiaGeomCollPtr geom, unsigned char **result, int *size)
{
/* builds the WKB representation for this GEOMETRY */
    int ib;
    int iv;
    double x;
    double y;
    int entities = 0;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    int type;
    unsigned char *ptr;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaPointPtr point = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    int endian_arch = gaiaEndianArch ();
    gaiaMbrGeometry (geom);
/* how many entities, and of what kind, do we have ? */
    pt = geom->FirstPoint;
    while (pt)
      {
	  point = pt;
	  entities++;
	  n_points++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  line = ln;
	  entities++;
	  n_linestrings++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  polyg = pg;
	  entities++;
	  n_polygons++;
	  pg = pg->Next;
      }
    *size = 0;
    *result = NULL;
    if (n_points == 0 && n_polygons == 0 && n_linestrings == 0)
	return;
/* ok, we can determine the geometry class */
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
	type = GAIA_POINT;
    else if (n_points > 1 && n_linestrings == 0 && n_polygons == 0)
	type = GAIA_MULTIPOINT;
    else if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
	type = GAIA_LINESTRING;
    else if (n_points == 0 && n_linestrings > 1 && n_polygons == 0)
	type = GAIA_MULTILINESTRING;
    else if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
	type = GAIA_POLYGON;
    else if (n_points == 0 && n_linestrings == 0 && n_polygons > 1)
	type = GAIA_MULTIPOLYGON;
    else
	type = GAIA_GEOMETRYCOLLECTION;
/* and now we compute the size of WKB */
    *size = 5;			/* header size */
    if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING ||
	type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	*size += 4;
    point = geom->FirstPoint;
    while (point)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING ||
	      type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	      *size += 5;
	  *size += (sizeof (double) * 2);	/* two doubles for each POINT */
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING ||
	      type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	      *size += 5;
	  *size += (4 + ((sizeof (double) * 2) * line->Points));	/* # points + [x,y] for each vertex */
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING ||
	      type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	      *size += 5;
	  rng = polyg->Exterior;
	  *size += (8 + ((sizeof (double) * 2) * rng->Points));	/* # rings + # points + [x.y] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + ((sizeof (double) * 2) * rng->Points));	/* # points + [x,y] array - interior ring */
	    }
	  polyg = polyg->Next;
      }
    *result = malloc (*size);
    ptr = *result;
/* and finally we build the WKB */
    *ptr = 0x01;		/* little endian byte order */
    gaiaExport32 (ptr + 1, type, 1, endian_arch);	/* the main CLASS TYPE */
    ptr += 5;
    if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING ||
	type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
      {
	  gaiaExport32 (ptr, entities, 1, endian_arch);	/* it's a collection; # entities */
	  ptr += 4;
      }
    point = geom->FirstPoint;
    while (point)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_GEOMETRYCOLLECTION)
	    {
		*ptr = 0x01;
		gaiaExport32 (ptr + 1, GAIA_POINT, 1, endian_arch);	/* it's a collection: the CLASS TYPE for this element */
		ptr += 5;
	    }
	  gaiaExport64 (ptr, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 8, point->Y, 1, endian_arch);	/* Y */
	  ptr += 16;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  if (type == GAIA_MULTILINESTRING || type == GAIA_GEOMETRYCOLLECTION)
	    {
		*ptr = 0x01;
		gaiaExport32 (ptr + 1, GAIA_LINESTRING, 1, endian_arch);	/* it's a collection: the CLASS TYPE for this element */
		ptr += 5;
	    }
	  gaiaExport32 (ptr, line->Points, 1, endian_arch);	/* # points */
	  ptr += 4;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y */
		ptr += 16;
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  if (type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	    {
		*ptr = 0x01;
		gaiaExport32 (ptr + 1, GAIA_POLYGON, 1, endian_arch);	/* it's a collection: the CLASS TYPE for this element */
		ptr += 5;
	    }
	  gaiaExport32 (ptr, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 4, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 8;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPoint (rng->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		ptr += 16;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
		      ptr += 16;
		  }
	    }
	  polyg = polyg->Next;
      }
}
