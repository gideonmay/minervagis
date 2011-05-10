/*

 gg_shape.c -- Gaia shapefile handling
  
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
#include <string.h>
#include <math.h>
#include <float.h>
#include <errno.h>

#include <spatialite/gaiageo.h>

GAIAGEO_DECLARE void
gaiaFreeValue (gaiaValuePtr p)
{
/* frees all memory allocations for this DBF Field value */
    if (!p)
	return;
    if (p->TxtValue)
	free (p->TxtValue);
    free (p);
}

GAIAGEO_DECLARE void
gaiaSetNullValue (gaiaDbfFieldPtr field)
{
/* assignes a NULL value to some DBF field */
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_NULL_VALUE;
    field->Value->TxtValue = NULL;
}

GAIAGEO_DECLARE void
gaiaSetIntValue (gaiaDbfFieldPtr field, int value)
{
/* assignes an INTEGER value to some DBF field */
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_INT_VALUE;
    field->Value->TxtValue = NULL;
    field->Value->IntValue = value;
}

GAIAGEO_DECLARE void
gaiaSetDoubleValue (gaiaDbfFieldPtr field, double value)
{
/* assignes a DOUBLE value to some DBF field */
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_DOUBLE_VALUE;
    field->Value->TxtValue = NULL;
    field->Value->DblValue = value;
}

GAIAGEO_DECLARE void
gaiaSetStrValue (gaiaDbfFieldPtr field, char *str)
{
/* assignes a STRING value to some DBF field */
    int len = strlen (str);
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_TEXT_VALUE;
    field->Value->TxtValue = malloc (len + 1);
    strcpy (field->Value->TxtValue, str);
}

GAIAGEO_DECLARE gaiaDbfFieldPtr
gaiaAllocDbfField (char *name, unsigned char type,
		   int offset, unsigned char length, unsigned char decimals)
{
/* allocates and initializes a DBF Field definition */
    gaiaDbfFieldPtr p = malloc (sizeof (gaiaDbfField));
    int len = strlen (name);
    p->Name = malloc (len + 1);
    strcpy (p->Name, name);
    p->Type = type;
    p->Offset = offset;
    p->Length = length;
    p->Decimals = decimals;
    p->Value = NULL;
    p->Next = NULL;
    return p;
}

GAIAGEO_DECLARE void
gaiaFreeDbfField (gaiaDbfFieldPtr p)
{
/* frees all memory allocations for this DBF Field definition */
    if (!p)
	return;
    if (p->Name)
	free (p->Name);
    if (p->Value)
	gaiaFreeValue (p->Value);
    free (p);
}

GAIAGEO_DECLARE gaiaDbfListPtr
gaiaAllocDbfList ()
{
/* allocates and initializes the DBF Fields list */
    gaiaDbfListPtr list = malloc (sizeof (gaiaDbfList));
    list->RowId = 0;
    list->Geometry = NULL;
    list->First = NULL;
    list->Last = NULL;
    return list;
}

GAIAGEO_DECLARE void
gaiaFreeDbfList (gaiaDbfListPtr list)
{
/* frees all memory allocations related to DBF Fields list */
    gaiaDbfFieldPtr p;
    gaiaDbfFieldPtr pn;
    if (!list)
	return;
    p = list->First;
    while (p)
      {
	  pn = p->Next;
	  gaiaFreeDbfField (p);
	  p = pn;
      }
    free (p);
}

GAIAGEO_DECLARE gaiaDbfFieldPtr
gaiaAddDbfField (gaiaDbfListPtr list, char *name, unsigned char type,
		 int offset, unsigned char length, unsigned char decimals)
{
/* inserts a Field in the DBF Fields list */
  gaiaDbfFieldPtr p = NULL;
  if (!list)
    return NULL;
  p =	gaiaAllocDbfField (name, type, offset, length, decimals);
  if (!(list->First))
    list->First = p;
  if (list->Last)
    list->Last->Next = p;
  list->Last = p;
  return p;
}

GAIAGEO_DECLARE void
gaiaResetDbfEntity (gaiaDbfListPtr list)
{
/* resets data values */
    gaiaDbfFieldPtr p;
    if (!list)
	return;
    p = list->First;
    while (p)
      {
	  if (p->Value)
	      gaiaFreeValue (p->Value);
	  p->Value = NULL;
	  p = p->Next;
      }
    if (list->Geometry)
	gaiaFreeGeomColl (list->Geometry);
    list->Geometry = NULL;
}

GAIAGEO_DECLARE gaiaValuePtr
gaiaCloneValue (gaiaValuePtr org)
{
/* creating a new VARIANT value copied from the original one */
    gaiaValuePtr value;
    int len;
    value = malloc (sizeof (gaiaValue));
    value->Type = GAIA_NULL_VALUE;
    value->TxtValue = NULL;
    switch (org->Type)
      {
      case GAIA_INT_VALUE:
	  value->Type = GAIA_INT_VALUE;
	  value->IntValue = org->IntValue;
	  break;
      case GAIA_DOUBLE_VALUE:
	  value->Type = GAIA_DOUBLE_VALUE;
	  value->DblValue = org->DblValue;
	  break;
      case GAIA_TEXT_VALUE:
	  value->Type = GAIA_TEXT_VALUE;
	  len = strlen (org->TxtValue);
	  value->TxtValue = malloc (len + 1);
	  strcpy (value->TxtValue, org->TxtValue);
      };
    return value;
}

GAIAGEO_DECLARE gaiaDbfListPtr
gaiaCloneDbfEntity (gaiaDbfListPtr org)
{
/* creating a new DBF LIST copied from the original one */
    gaiaDbfFieldPtr p;
    gaiaDbfFieldPtr newFld;
    gaiaDbfListPtr entity = gaiaAllocDbfList ();
    entity->RowId = org->RowId;
    if (org->Geometry)
	entity->Geometry = gaiaCloneGeomColl (org->Geometry);
    p = org->First;
    while (p)
      {
	  newFld =
	      gaiaAddDbfField (entity, p->Name, p->Type, p->Offset, p->Length,
			       p->Decimals);
	  if (p->Value)
	      newFld->Value = gaiaCloneValue (p->Value);
	  p = p->Next;
      }
    return entity;
}

GAIAGEO_DECLARE gaiaShapefilePtr
gaiaAllocShapefile ()
{
/* allocates and initializes the Shapefile object */
    gaiaShapefilePtr shp = malloc (sizeof (gaiaShapefile));
    shp->endian_arch = 1;
    shp->Path = NULL;
    shp->Shape = -1;
    shp->flShp = NULL;
    shp->flShx = NULL;
    shp->flDbf = NULL;
    shp->Dbf = NULL;
    shp->BufShp = NULL;
    shp->ShpBfsz = 0;
    shp->BufDbf = NULL;
    shp->DbfHdsz = 0;
    shp->DbfReclen = 0;
    shp->DbfSize = 0;
    shp->DbfRecno = 0;
    shp->ShpSize = 0;
    shp->ShxSize = 0;
    shp->MinX = DBL_MAX;
    shp->MinY = DBL_MAX;
    shp->MaxX = DBL_MIN;
    shp->MaxY = DBL_MIN;
    shp->Valid = 0;
    shp->LastError = NULL;
    return shp;
}

GAIAGEO_DECLARE void
gaiaFreeShapefile (gaiaShapefilePtr shp)
{
/* frees all memory allocations related to the Shapefile object */
    if (shp->Path)
	free (shp->Path);
    if (shp->flShp)
	fclose (shp->flShp);
    if (shp->flShx)
	fclose (shp->flShx);
    if (shp->flDbf)
	fclose (shp->flDbf);
    if (shp->Dbf)
	gaiaFreeDbfList (shp->Dbf);
    if (shp->BufShp)
	free (shp->BufShp);
    if (shp->BufDbf)
	free (shp->BufDbf);
    if (shp->LastError)
	free (shp->LastError);
    free (shp);
}

GAIAGEO_DECLARE void
gaiaOpenShpRead (gaiaShapefilePtr shp, const char *path)
{
/* trying to open the shapefile and initial checkings */
    FILE *fl_shx = NULL;
    FILE *fl_shp = NULL;
    FILE *fl_dbf = NULL;
    char xpath[1024];
    int rd;
    unsigned char buf_shx[256];
    int size_shp;
    int size_shx;
    unsigned char *buf_shp = NULL;
    int buf_size = 1024;
    int shape;
    unsigned char bf[1024];
    int dbf_size;
    int dbf_reclen = 0;
    int dbf_recno;
    int off_dbf;
    int ind;
    char field_name[16];
    char *sys_err;
    char errMsg[1024];
    int len;
    int endian_arch = gaiaEndianArch ();
    gaiaDbfListPtr dbf_list = NULL;
    sprintf (xpath, "%s.shx", path);
    fl_shx = fopen (xpath, "rb");
    if (!fl_shx)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for reading: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.shp", path);
    fl_shp = fopen (xpath, "rb");
    if (!fl_shp)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for reading: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.dbf", path);
    fl_dbf = fopen (xpath, "rb");
    if (!fl_dbf)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for reading: %s", xpath,
		   sys_err);
	  goto no_file;
      }
/* reading SHX file header */
    rd = fread (buf_shx, sizeof (unsigned char), 100, fl_shx);
    if (rd != 100)
	goto error;
    if (gaiaImport32 (buf_shx + 0, GAIA_BIG_ENDIAN, endian_arch) != 9994)	/* checks the SHX magic number */
	goto error;
    size_shx = gaiaImport32 (buf_shx + 24, GAIA_BIG_ENDIAN, endian_arch);
/* reading SHP file header */
    buf_shp = malloc (sizeof (unsigned char) * buf_size);
    rd = fread (buf_shp, sizeof (unsigned char), 100, fl_shp);
    if (rd != 100)
	goto error;
    if (gaiaImport32 (buf_shp + 0, GAIA_BIG_ENDIAN, endian_arch) != 9994)	/* checks the SHP magic number */
	goto error;
    size_shp = gaiaImport32 (buf_shp + 24, GAIA_BIG_ENDIAN, endian_arch);
    shape = gaiaImport32 (buf_shp + 32, GAIA_LITTLE_ENDIAN, endian_arch);
    if (shape == 1 || shape == 11 || shape == 21 ||
	shape == 3 || shape == 13 || shape == 23 ||
	shape == 5 || shape == 15 || shape == 25)
	;
    else
	goto unsupported;
/* reading DBF file header */
    rd = fread (bf, sizeof (unsigned char), 32, fl_dbf);
    if (rd != 32)
	goto error;
    if (*bf != 0x03)		/* checks the DBF magic number */
	goto error;
    dbf_recno = gaiaImport32 (bf + 4, GAIA_LITTLE_ENDIAN, endian_arch);
    dbf_size = gaiaImport16 (bf + 8, GAIA_LITTLE_ENDIAN, endian_arch);
    dbf_reclen = gaiaImport16 (bf + 10, GAIA_LITTLE_ENDIAN, endian_arch);
    dbf_size--;
    off_dbf = 0;
    dbf_list = gaiaAllocDbfList ();
    for (ind = 32; ind < dbf_size; ind += 32)
      {
	  /* fetches DBF fields definitions */
	  rd = fread (bf, sizeof (unsigned char), 32, fl_dbf);
	  if (rd != 32)
	      goto error;
	  memcpy (field_name, bf, 11);
	  field_name[11] = '\0';
	  gaiaAddDbfField (dbf_list, field_name, *(bf + 11), off_dbf,
			   *(bf + 16), *(bf + 17));
	  off_dbf += *(bf + 16);
      }
    len = strlen (path);
    shp->Path = malloc (len + 1);
    strcpy (shp->Path, path);
    shp->ReadOnly = 1;
    shp->Shape = shape;
    shp->flShp = fl_shp;
    shp->flShx = fl_shx;
    shp->flDbf = fl_dbf;
    shp->Dbf = dbf_list;
/* saving the SHP buffer */
    shp->BufShp = buf_shp;
    shp->ShpBfsz = buf_size;
/* allocating DBF buffer */
    shp->BufDbf = malloc (sizeof (unsigned char) * dbf_reclen);
    shp->DbfHdsz = dbf_size + 1;
    shp->DbfReclen = dbf_reclen;
    shp->Valid = 1;
    shp->endian_arch = endian_arch;
    return;
  no_file:
/* one of shapefile's files can't be accessed */
    if (shp->LastError)
	free (shp->LastError);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    if (fl_shx)
	fclose (fl_shx);
    if (fl_shp)
	fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
  error:
/* the shapefile is invalid or corrupted */
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s' is corrupted / has invalid format", path);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (buf_shp)
	free (buf_shp);
    fclose (fl_shx);
    fclose (fl_shp);
    fclose (fl_dbf);
    return;
  unsupported:
/* the shapefile has an unrecognized shape type */
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s' shape=%d is not supported", path, shape);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (buf_shp)
	free (buf_shp);
    fclose (fl_shx);
    fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
}

GAIAGEO_DECLARE void
gaiaOpenShpWrite (gaiaShapefilePtr shp, const char *path, int shape,
		  gaiaDbfListPtr dbf_list)
{
/* trying to create the shapefile */
    FILE *fl_shx = NULL;
    FILE *fl_shp = NULL;
    FILE *fl_dbf = NULL;
    char xpath[1024];
    unsigned char *buf_shp = NULL;
    int buf_size = 1024;
    unsigned char *dbf_buf = NULL;
    gaiaDbfFieldPtr fld;
    char *sys_err;
    char errMsg[1024];
    int len;
    short dbf_reclen = 0;
    int shp_size = 0;
    int shx_size = 0;
    unsigned short dbf_size = 0;
    int endian_arch = gaiaEndianArch ();
    buf_shp = malloc (buf_size);
/* trying to open shapefile files */
    sprintf (xpath, "%s.shx", path);
    fl_shx = fopen (xpath, "wb");
    if (!fl_shx)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for writing: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.shp", path);
    fl_shp = fopen (xpath, "wb");
    if (!fl_shp)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for writing: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.dbf", path);
    fl_dbf = fopen (xpath, "wb");
    if (!fl_dbf)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for writing: %s", xpath,
		   sys_err);
	  goto no_file;
      }
/* allocating DBF buffer */
    dbf_reclen = 1;		/* an extra byte is needed because in DBF rows first byte is a marker for deletion */
    fld = dbf_list->First;
    while (fld)
      {
	  /* computing the DBF record length */
	  dbf_reclen += fld->Length;
	  fld = fld->Next;
      }
    dbf_buf = malloc (dbf_reclen);
/* writing an empty SHP file header */
    memset (buf_shp, 0, 100);
    fwrite (buf_shp, 1, 100, fl_shp);
    shp_size = 50;		/* note: shapefile [SHP and SHX] counts sizes in WORDS of 16 bits, not in bytes of 8 bits !!!! */
/* writing an empty SHX file header */
    memset (buf_shp, 0, 100);
    fwrite (buf_shp, 1, 100, fl_shx);
    shx_size = 50;
/* writing the DBF file header */
    memset (buf_shp, '\0', 32);
    fwrite (buf_shp, 1, 32, fl_dbf);
    dbf_size = 32;		/* note: DBF counts sizes in bytes */
    fld = dbf_list->First;
    while (fld)
      {
	  /* exporting DBF Fields specifications */
	  memset (buf_shp, 0, 32);
	  if (strlen (fld->Name) > 10)
	      memcpy (buf_shp, fld->Name, 10);
	  else
	      memcpy (buf_shp, fld->Name, strlen (fld->Name));
	  *(buf_shp + 11) = fld->Type;
	  *(buf_shp + 16) = fld->Length;
	  *(buf_shp + 17) = fld->Decimals;
	  fwrite (buf_shp, 1, 32, fl_dbf);
	  dbf_size += 32;
	  fld = fld->Next;
      }
    fwrite ("\r", 1, 1, fl_dbf);	/* this one is a special DBF delimiter that closes file header */
    dbf_size++;
/* setting up the SHP struct */
    len = strlen (path);
    shp->Path = malloc (len + 1);
    strcpy (shp->Path, path);
    shp->ReadOnly = 0;
    if (shape == GAIA_POINT)
	shp->Shape = 1;
    if (shape == GAIA_MULTIPOINT)
	shp->Shape = 8;
    if (shape == GAIA_LINESTRING)
	shp->Shape = 3;
    if (shape == GAIA_POLYGON)
	shp->Shape = 5;
    shp->flShp = fl_shp;
    shp->flShx = fl_shx;
    shp->flDbf = fl_dbf;
    shp->Dbf = dbf_list;
    shp->BufShp = buf_shp;
    shp->ShpBfsz = buf_size;
    shp->BufDbf = dbf_buf;
    shp->DbfHdsz = dbf_size + 1;
    shp->DbfReclen = dbf_reclen;
    shp->DbfSize = dbf_size;
    shp->DbfRecno = 0;
    shp->ShpSize = shp_size;
    shp->ShxSize = shx_size;
    shp->MinX = DBL_MAX;
    shp->MinY = DBL_MAX;
    shp->MaxX = DBL_MIN;
    shp->MaxY = DBL_MIN;
    shp->Valid = 1;
    shp->endian_arch = endian_arch;
    return;
  no_file:
/* one of shapefile's files can't be created/opened */
    if (shp->LastError)
	free (shp->LastError);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    if (buf_shp)
	free (buf_shp);
    if (fl_shx)
	fclose (fl_shx);
    if (fl_shp)
	fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
}

GAIAGEO_DECLARE int
gaiaReadShpEntity (gaiaShapefilePtr shp, int current_row)
{
/* trying to read an entity from shapefile */
    unsigned char buf[512];
    int rd;
    int skpos;
    int offset;
    int off_shp;
    int i;
    int sz;
    int shape;
    double x;
    double y;
    int points;
    int n;
    int n1;
    int base;
    int start;
    int end;
    int iv;
    int ind;
    char errMsg[1024];
    int len;
    gaiaGeomCollPtr geom = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring = NULL;
    gaiaDbfFieldPtr pFld;
/* positioning and reading the SHX file */
    offset = 100 + (current_row * 8);	/* 100 bytes for the header + current row displacement; each SHX row = 8 bytes */
    skpos = fseek (shp->flShx, offset, SEEK_SET);
    if (skpos != 0)
	goto eof;
    rd = fread (buf, sizeof (unsigned char), 8, shp->flShx);
    if (rd != 8)
	goto eof;
    off_shp = gaiaImport32 (buf, GAIA_BIG_ENDIAN, shp->endian_arch);
/* positioning and reading the DBF file */
    offset = shp->DbfHdsz + (current_row * shp->DbfReclen);
    skpos = fseek (shp->flDbf, offset, SEEK_SET);
    if (skpos != 0)
	goto error;
    rd = fread (shp->BufDbf, sizeof (unsigned char),
		shp->DbfReclen, shp->flDbf);
    if (rd != shp->DbfReclen)
	goto error;
/* positioning and reading corresponding SHP entity - geometry */
    offset = off_shp * 2;
    skpos = fseek (shp->flShp, offset, SEEK_SET);
    if (skpos != 0)
	goto error;
    rd = fread (buf, sizeof (unsigned char), 12, shp->flShp);
    if (rd != 12)
	goto error;
    sz = gaiaImport32 (buf + 4, GAIA_BIG_ENDIAN, shp->endian_arch);
    shape = gaiaImport32 (buf + 8, GAIA_LITTLE_ENDIAN, shp->endian_arch);
    if (shape != shp->Shape)
	goto error;
    if ((sz * 2) > shp->ShpBfsz)
      {
	  /* current buffer is too small; we need to allocate a bigger buffer */
	  free (shp->BufShp);
	  shp->ShpBfsz = sz * 2;
	  shp->BufShp = malloc (sizeof (unsigned char) * shp->ShpBfsz);
      }
    if (shape == 1 || shape == 11 || shape == 21)
      {
	  /* shape point */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 16, shp->flShp);
	  if (rd != 16)
	      goto error;
	  x = gaiaImport64 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  y = gaiaImport64 (shp->BufShp + 8, GAIA_LITTLE_ENDIAN,
			    shp->endian_arch);
	  geom = gaiaAllocGeomColl ();
	  gaiaAddPointToGeomColl (geom, x, y);
      }
    if (shape == 3 || shape == 13 || shape == 23)
      {
	  /* shape polyline */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
			     shp->endian_arch);
	  base = 8 + (n * 4);
	  start = 0;
	  for (ind = 0; ind < n; ind++)
	    {
		if (ind < (n - 1))
		    end =
			gaiaImport32 (shp->BufShp + 8 + ((ind + 1) * 4),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    end = n1;
		points = end - start;
		line = gaiaAllocLinestring (points);
		points = 0;
		for (iv = start; iv < end; iv++)
		  {
		      x = gaiaImport64 (shp->BufShp + base + (iv * 16),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      y = gaiaImport64 (shp->BufShp + base + (iv * 16) + 8,
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      gaiaSetPoint (line->Coords, points, x, y);
		      start++;
		      points++;
		  }
		if (!geom)
		    geom = gaiaAllocGeomColl ();
		gaiaInsertLinestringInGeomColl (geom, line);
	    }
      }
    if (shape == 5 || shape == 15 || shape == 25)
      {
	  /* shape polygon */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
			     shp->endian_arch);
	  base = 8 + (n * 4);
	  start = 0;
	  for (ind = 0; ind < n; ind++)
	    {
		if (ind < (n - 1))
		    end =
			gaiaImport32 (shp->BufShp + 8 + ((ind + 1) * 4),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    end = n1;
		points = end - start;
		ring = gaiaAllocRing (points);
		points = 0;
		for (iv = start; iv < end; iv++)
		  {
		      x = gaiaImport64 (shp->BufShp + base + (iv * 16),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      y = gaiaImport64 (shp->BufShp + base + (iv * 16) + 8,
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      gaiaSetPoint (ring->Coords, points, x, y);
		      start++;
		      points++;
		  }
		if (!geom)
		  {
		      /* new geometry - new need to allocate a new POLYGON */
		      geom = gaiaAllocGeomColl ();
		      polyg = gaiaInsertPolygonInGeomColl (geom, ring);
		  }
		else
		  {
		      gaiaClockwise (ring);
		      if (ring->Clockwise)
			{
			    /* this one is a POLYGON exterior ring - we need to allocate e new POLYGON */
			    polyg = gaiaInsertPolygonInGeomColl (geom, ring);
			}
		      else
			{
			    /* adding an interior ring to current POLYGON */
			    gaiaAddRingToPolyg (polyg, ring);
			}
		  }
	    }
      }
/* setting up the current SHP ENTITY */
    gaiaResetDbfEntity (shp->Dbf);
    shp->Dbf->RowId = current_row;
    shp->Dbf->Geometry = geom;
/* fetching the DBF values */
    pFld = shp->Dbf->First;
    while (pFld)
      {
	  memcpy (buf, shp->BufDbf + pFld->Offset + 1, pFld->Length);
	  buf[pFld->Length] = '\0';
	  if (*buf == '\0')
	      gaiaSetNullValue (pFld);
	  else
	    {
		if (pFld->Type == 'N')
		  {
		      if (pFld->Decimals > 0 || pFld->Length > 9)
			  gaiaSetDoubleValue (pFld, atof ((char *) buf));
		      else
			  gaiaSetIntValue (pFld, atoi ((char *) buf));
		  }
		else
		  {
		      for (i = strlen ((char *) buf) - 1; i > 1; i--)
			{
			    /* cleaning up trailing spaces */
			    if (buf[i] == ' ')
				buf[i] = '\0';
			    else
				break;
			}
		      gaiaSetStrValue (pFld, (char *) buf);
		  }
	    }
	  pFld = pFld->Next;
      }
    if (shp->LastError)
	free (shp->LastError);
    shp->LastError = NULL;
    return 1;
  eof:
    if (shp->LastError)
	free (shp->LastError);
    shp->LastError = NULL;
    return 0;
  error:
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s' is corrupted / has invalid format", shp->Path);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    return 0;
}

static void
gaiaSaneClockwise (gaiaPolygonPtr polyg)
{
/*
/ when exporting POLYGONs to SHAPEFILE, we must guarantee that:
/ - all EXTERIOR RING must be clockwise
/ - all INTERIOR RING must be anti-clockwise
/
/ this function checks for the above conditions,
/ and if needed inverts the rings
*/
    int ib;
    int iv;
    int iv2;
    double x;
    double y;
    gaiaRingPtr new_ring;
    gaiaRingPtr ring = polyg->Exterior;
    gaiaClockwise (ring);
    if (!(ring->Clockwise))
      {
	  /* exterior ring needs inversion */
	  new_ring = gaiaAllocRing (ring->Points);
	  iv2 = 0;
	  for (iv = ring->Points - 1; iv >= 0; iv--)
	    {
		gaiaGetPoint (ring->Coords, iv, &x, &y);
		gaiaSetPoint (new_ring->Coords, iv2, x, y);
		iv2++;
	    }
	  polyg->Exterior = new_ring;
	  gaiaFreeRing (ring);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  gaiaClockwise (ring);
	  if (ring->Clockwise)
	    {
		/* interior ring needs inversion */
		new_ring = gaiaAllocRing (ring->Points);
		iv2 = 0;
		for (iv = ring->Points - 1; iv >= 0; iv--)
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		      gaiaSetPoint (new_ring->Coords, iv2, x, y);
		      iv2++;
		  }
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      gaiaGetPoint (new_ring->Coords, iv, &x, &y);
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
		gaiaFreeRing (new_ring);
	    }
      }
}

GAIAGEO_DECLARE int
gaiaWriteShpEntity (gaiaShapefilePtr shp, gaiaDbfListPtr entity)
{
/* trying to write an entity into shapefile */
    char dummy[128];
    char fmt[16];
    int endian_arch = shp->endian_arch;
    gaiaDbfFieldPtr fld;
    int iv;
    int tot_ln;
    int tot_v;
    int this_size;
    int ix;
    double x;
    double y;
    int len;
/* writing the DBF record */
    memset (shp->BufDbf, '\0', shp->DbfReclen);
    *(shp->BufDbf) = ' ';	/* in DBF first byte of each row marks for validity or deletion */
    fld = entity->First;
    while (fld)
      {
	  /* transferring field values */
	  switch (fld->Type)
	    {
	    case 'L':
		if (!(fld->Value))
		    *(shp->BufDbf + fld->Offset) = '?';
		else if (fld->Value->Type != GAIA_INT_VALUE)
		    *(shp->BufDbf + fld->Offset + 1) = '?';
		else
		  {
		      if (fld->Value->IntValue == 0)
			  *(shp->BufDbf + fld->Offset + 1) = 'N';
		      else
			  *(shp->BufDbf + fld->Offset + 1) = 'Y';
		  }
		break;
	    case 'D':
		memset (shp->BufDbf + fld->Offset + 1, '0', 8);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_TEXT_VALUE)
			{
			    if (strlen (fld->Value->TxtValue) == 8)
				memcpy (shp->BufDbf + fld->Offset + 1,
					fld->Value->TxtValue, 8);
			}
		  }
		break;
	    case 'C':
		memset (shp->BufDbf + fld->Offset + 1, ' ', fld->Length);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_TEXT_VALUE)
			{
			    if (strlen (fld->Value->TxtValue) < fld->Length)
				memcpy (shp->BufDbf + fld->Offset + 1,
					fld->Value->TxtValue,
					strlen (fld->Value->TxtValue));
			    else
				memcpy (shp->BufDbf + fld->Offset + 1,
					fld->Value->TxtValue, fld->Length);
			}
		  }
		break;
	    case 'N':
		memset (shp->BufDbf + fld->Offset + 1, '\0', fld->Length);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_INT_VALUE)
			{
			    sprintf (dummy, "%d", fld->Value->IntValue);
			    if (strlen (dummy) <= fld->Length)
				memcpy (shp->BufDbf + fld->Offset + 1, dummy,
					strlen (dummy));
			}
		      if (fld->Value->Type == GAIA_DOUBLE_VALUE)
			{
			    sprintf (fmt, "%%1.%dlf", fld->Decimals);
			    sprintf (dummy, fmt, fld->Value->DblValue);
			    if (strlen (dummy) <= fld->Length)
				memcpy (shp->BufDbf + fld->Offset + 1, dummy,
					strlen (dummy));
			}
		  }
		break;
	    };
	  fld = fld->Next;
      }
    if (!(entity->Geometry))
      {
	  strcpy (dummy, "NULL geometry");
	  if (shp->LastError)
	      free (shp->LastError);
	  len = strlen (dummy);
	  shp->LastError = malloc (len + 1);
	  strcpy (shp->LastError, dummy);
	  return 0;
      }
/* updates the shapefile main MBR-BBOX */
    gaiaMbrGeometry (entity->Geometry);
    if (entity->Geometry->MinX < shp->MinX)
	shp->MinX = entity->Geometry->MinX;
    if (entity->Geometry->MaxX > shp->MaxX)
	shp->MaxX = entity->Geometry->MaxX;
    if (entity->Geometry->MinY < shp->MinY)
	shp->MinY = entity->Geometry->MinY;
    if (entity->Geometry->MaxY > shp->MaxY)
	shp->MaxY = entity->Geometry->MaxY;
    if (shp->Shape == 1)
      {
	  /* this one is expected to be a POINT */
	  gaiaPointPtr pt = entity->Geometry->FirstPoint;
	  if (!pt)
	    {
		strcpy (dummy,
			"a POINT is expected, but there is no POINT in geometry");
		if (shp->LastError)
		    free (shp->LastError);
		len = strlen (dummy);
		shp->LastError = malloc (len + 1);
		strcpy (shp->LastError, dummy);
		return 0;
	    }
	  /* inserting POINT entity into SHX file */
	  gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
	  gaiaExport32 (shp->BufShp + 4, 10, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
	  fwrite (shp->BufShp, 1, 8, shp->flShx);
	  (shp->ShxSize) += 4;	/* updating current SHX file poisition [in 16 bits words !!!] */
	  /* inserting POINT into SHP file */
	  gaiaExport32 (shp->BufShp, shp->DbfRecno, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
	  gaiaExport32 (shp->BufShp + 4, 10, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
	  gaiaExport32 (shp->BufShp + 8, 1, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POINT */
	  gaiaExport64 (shp->BufShp + 12, pt->X, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports X coordinate */
	  gaiaExport64 (shp->BufShp + 20, pt->Y, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports Y coordinate */
	  fwrite (shp->BufShp, 1, 28, shp->flShp);
	  (shp->ShpSize) += 14;	/* updating current SHP file poisition [in 16 bits words !!!] */
      }
    if (shp->Shape == 3)
      {
	  /* this one is expected to be a LINESTRING / MULTILINESTRING */
	  gaiaLinestringPtr line;
	  tot_ln = 0;
	  tot_v = 0;
	  line = entity->Geometry->FirstLinestring;
	  while (line)
	    {
		/* computes # lines and total # points */
		tot_v += line->Points;
		tot_ln++;
		line = line->Next;
	    }
	  if (!tot_ln)
	    {
		strcpy (dummy,
			"a LINESTRING is expected, but there is no LINESTRING in geometry");
		if (shp->LastError)
		    free (shp->LastError);
		len = strlen (dummy);
		shp->LastError = malloc (len + 1);
		strcpy (shp->LastError, dummy);
		return 0;
	    }
	  this_size = 22 + (2 * tot_ln) + (tot_v * 8);	/* size [in 16 bits words !!!] for this SHP entity */
	  if ((this_size * 2) + 1024 > shp->ShpBfsz)
	    {
		/* current buffer is too small; we need to allocate a bigger one */
		free (shp->BufShp);
		shp->ShpBfsz = (this_size * 2) + 1024;
		shp->BufShp = malloc (shp->ShpBfsz);
	    }
	  /* inserting LINESTRING or MULTILINESTRING in SHX file */
	  gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
	  gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
	  fwrite (shp->BufShp, 1, 8, shp->flShx);
	  (shp->ShxSize) += 4;
	  /* inserting LINESTRING or MULTILINESTRING in SHP file */
	  gaiaExport32 (shp->BufShp, shp->DbfRecno, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
	  gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
	  gaiaExport32 (shp->BufShp + 8, 3, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POLYLINE */
	  gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
	  gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			GAIA_LITTLE_ENDIAN, endian_arch);
	  gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			GAIA_LITTLE_ENDIAN, endian_arch);
	  gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			GAIA_LITTLE_ENDIAN, endian_arch);
	  gaiaExport32 (shp->BufShp + 44, tot_ln, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # lines in this polyline */
	  gaiaExport32 (shp->BufShp + 48, tot_v, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
	  tot_v = 0;		/* resets points counter */
	  ix = 52;		/* sets current buffer offset */
	  line = entity->Geometry->FirstLinestring;
	  while (line)
	    {
		/* exports start point index for each line */
		gaiaExport32 (shp->BufShp + ix, tot_v, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		tot_v += line->Points;
		ix += 4;
		line = line->Next;
	    }
	  line = entity->Geometry->FirstLinestring;
	  while (line)
	    {
		/* exports points for each line */
		for (iv = 0; iv < line->Points; iv++)
		  {
		      /* exports a POINT [x,y] */
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		      gaiaExport64 (shp->BufShp + ix, x, GAIA_LITTLE_ENDIAN,
				    endian_arch);
		      ix += 8;
		      gaiaExport64 (shp->BufShp + ix, y, GAIA_LITTLE_ENDIAN,
				    endian_arch);
		      ix += 8;
		  }
		line = line->Next;
	    }
	  fwrite (shp->BufShp, 1, ix, shp->flShp);
	  (shp->ShpSize) += (ix / 2);	/* updating current SHP file poisition [in 16 bits words !!!] */
      }
    if (shp->Shape == 5)
      {
	  /* this one is expected to be a POLYGON or a MULTIPOLYGON */
	  gaiaPolygonPtr polyg;
	  gaiaRingPtr ring;
	  int ib;
	  tot_ln = 0;
	  tot_v = 0;
	  polyg = entity->Geometry->FirstPolygon;
	  while (polyg)
	    {
		/* computes # rings and total # points */
		gaiaSaneClockwise (polyg);	/* we must assure that exterior ring is clockwise, and interior rings are anti-clockwise */
		ring = polyg->Exterior;	/* this one is the exterior ring */
		tot_v += ring->Points;
		tot_ln++;
		for (ib = 0; ib < polyg->NumInteriors; ib++)
		  {
		      /* that ones are the interior rings */
		      ring = polyg->Interiors + ib;
		      tot_v += ring->Points;
		      tot_ln++;
		  }
		polyg = polyg->Next;
	    }
	  if (!tot_ln)
	    {
		strcpy (dummy,
			"a POLYGON is expected, but there is no POLYGON in geometry");
		if (shp->LastError)
		    free (shp->LastError);
		len = strlen (dummy);
		shp->LastError = malloc (len + 1);
		strcpy (shp->LastError, dummy);
		return 0;
	    }
	  this_size = 22 + (2 * tot_ln) + (tot_v * 8);	/* size [in 16 bits words !!!] for this SHP entity */
	  if ((this_size * 2) + 1024 > shp->ShpBfsz)
	    {
		/* current buffer is too small; we need to allocate a bigger one */
		free (shp->BufShp);
		shp->ShpBfsz = (this_size * 2) + 1024;
		shp->BufShp = malloc (shp->ShpBfsz);
	    }
	  /* inserting POLYGON or MULTIPOLYGON in SHX file */
	  gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
	  gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
	  fwrite (shp->BufShp, 1, 8, shp->flShx);
	  (shp->ShxSize) += 4;
	  /* inserting POLYGON or MULTIPOLYGON in SHP file */
	  gaiaExport32 (shp->BufShp, shp->DbfRecno, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
	  gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
	  gaiaExport32 (shp->BufShp + 8, 5, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POLYGON */
	  gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
	  gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			GAIA_LITTLE_ENDIAN, endian_arch);
	  gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			GAIA_LITTLE_ENDIAN, endian_arch);
	  gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			GAIA_LITTLE_ENDIAN, endian_arch);
	  gaiaExport32 (shp->BufShp + 44, tot_ln, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # rings in this polygon */
	  gaiaExport32 (shp->BufShp + 48, tot_v, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
	  tot_v = 0;		/* resets points counter */
	  ix = 52;		/* sets current buffer offset */
	  polyg = entity->Geometry->FirstPolygon;
	  while (polyg)
	    {
		/* exports start point index for each line */
		ring = polyg->Exterior;	/* this one is the exterior ring */
		gaiaExport32 (shp->BufShp + ix, tot_v, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		tot_v += ring->Points;
		ix += 4;
		for (ib = 0; ib < polyg->NumInteriors; ib++)
		  {
		      /* that ones are the interior rings */
		      ring = polyg->Interiors + ib;
		      gaiaExport32 (shp->BufShp + ix, tot_v, GAIA_LITTLE_ENDIAN,
				    endian_arch);
		      tot_v += ring->Points;
		      ix += 4;
		  }
		polyg = polyg->Next;
	    }
	  polyg = entity->Geometry->FirstPolygon;
	  while (polyg)
	    {
		/* exports points for each ring */
		ring = polyg->Exterior;	/* this one is the exterior ring */
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      /* exports a POINT [x,y] - exterior ring */
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		      gaiaExport64 (shp->BufShp + ix, x, GAIA_LITTLE_ENDIAN,
				    endian_arch);
		      ix += 8;
		      gaiaExport64 (shp->BufShp + ix, y, GAIA_LITTLE_ENDIAN,
				    endian_arch);
		      ix += 8;
		  }
		for (ib = 0; ib < polyg->NumInteriors; ib++)
		  {
		      /* that ones are the interior rings */
		      ring = polyg->Interiors + ib;
		      for (iv = 0; iv < ring->Points; iv++)
			{
			    /* exports a POINT [x,y] - interior ring */
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			    gaiaExport64 (shp->BufShp + ix, x,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			    gaiaExport64 (shp->BufShp + ix, y,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		  }
		polyg = polyg->Next;
	    }
	  fwrite (shp->BufShp, 1, ix, shp->flShp);
	  (shp->ShpSize) += (ix / 2);
      }
/* inserting entity in DBF file */
    fwrite (shp->BufDbf, 1, shp->DbfReclen, shp->flDbf);
    (shp->DbfRecno)++;
    return 1;
}

GAIAGEO_DECLARE void
gaiaFlushShpHeaders (gaiaShapefilePtr shp)
{
/* updates the various file headers */
    FILE *fl_shp = shp->flShp;
    FILE *fl_shx = shp->flShx;
    FILE *fl_dbf = shp->flDbf;
    int shp_size = shp->ShpSize;
    int shx_size = shp->ShxSize;
    int dbf_size = shp->DbfSize;
    int dbf_reclen = shp->DbfReclen;
    int dbf_recno = shp->DbfRecno;
    int endian_arch = shp->endian_arch;
    double minx = shp->MinX;
    double miny = shp->MinY;
    double maxx = shp->MaxX;
    double maxy = shp->MaxY;
    unsigned char *buf_shp = shp->BufShp;
/* writing the SHP file header */
    fseek (fl_shp, 0, SEEK_SET);	/* repositioning at SHP file start */
    gaiaExport32 (buf_shp, 9994, GAIA_BIG_ENDIAN, endian_arch);	/* SHP magic number */
    gaiaExport32 (buf_shp + 4, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 8, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 12, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 16, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 20, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 24, shp_size, GAIA_BIG_ENDIAN, endian_arch);	/* SHP file size - measured in 16 bits words !!! */
    gaiaExport32 (buf_shp + 28, 1000, GAIA_LITTLE_ENDIAN, endian_arch);	/* version */
    gaiaExport32 (buf_shp + 32, shp->Shape, GAIA_LITTLE_ENDIAN, endian_arch);	/* ESRI shape */
    gaiaExport64 (buf_shp + 36, minx, GAIA_LITTLE_ENDIAN, endian_arch);	/* the MBR/BBOX for the whole shapefile */
    gaiaExport64 (buf_shp + 44, miny, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 52, maxx, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 60, maxy, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 68, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 76, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 84, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 92, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    fwrite (buf_shp, 1, 100, fl_shp);
/* writing the SHX file header */
    fseek (fl_shx, 0, SEEK_SET);	/* repositioning at SHX file start */
    gaiaExport32 (buf_shp, 9994, GAIA_BIG_ENDIAN, endian_arch);	/* SHP magic number */
    gaiaExport32 (buf_shp + 4, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 8, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 12, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 16, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 20, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 24, shx_size, GAIA_BIG_ENDIAN, endian_arch);	/* SHXfile size - measured in 16 bits words !!! */
    gaiaExport32 (buf_shp + 28, 1000, GAIA_LITTLE_ENDIAN, endian_arch);	/* version */
    gaiaExport32 (buf_shp + 32, shp->Shape, GAIA_LITTLE_ENDIAN, endian_arch);	/* ESRI shape */
    gaiaExport64 (buf_shp + 36, minx, GAIA_LITTLE_ENDIAN, endian_arch);	/* the MBR for the whole shapefile */
    gaiaExport64 (buf_shp + 44, miny, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 52, maxx, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 60, maxy, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 68, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 76, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 84, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 92, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    fwrite (buf_shp, 1, 100, fl_shx);
/* writing the DBF file header */
    *buf_shp = 0x1a;		/* DBF - this is theEOF marker */
    fwrite (buf_shp, 1, 1, fl_dbf);
    fseek (fl_dbf, 0, SEEK_SET);	/* repositioning at DBF file start */
    memset (buf_shp, '\0', 32);
    *buf_shp = 0x03;		/* DBF magic number */
    *(buf_shp + 1) = 1;		/* this is supposed to be the last update date [Year, Month, Day], but we ignore it at all */
    *(buf_shp + 2) = 1;
    *(buf_shp + 3) = 1;
    gaiaExport32 (buf_shp + 4, dbf_recno, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # records in this DBF */
    gaiaExport16 (buf_shp + 8, dbf_size, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the file header size */
    gaiaExport16 (buf_shp + 10, dbf_reclen, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the record length */
    fwrite (buf_shp, 1, 32, fl_dbf);
}
