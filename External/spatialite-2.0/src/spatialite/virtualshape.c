/*

 virtualshape.c -- SQLite3 extension [VIRTUAL TABLE accessing Shapefile]

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
#include <sqlite3ext.h>
#include <spatialite/spatialite.h>
#include <spatialite/gaiageo.h>

static SQLITE_EXTENSION_INIT1

static struct sqlite3_module my_module;

typedef struct VirtualShapeStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    gaiaShapefilePtr Shp;	/* the Shapefile struct */
} VirtualShape;
typedef VirtualShape *VirtualShapePtr;

typedef struct VirtualShapeCursortStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualShapePtr pVtab;	/* Virtual table of this cursor */
    long current_row;		/* the current row ID */
    int blobSize;
    unsigned char *blobGeometry;
    int eof;			/* the EOF marker */
} VirtualShapeCursor;
typedef VirtualShapeCursor *VirtualShapeCursorPtr;

static int
vshp_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table connected to some shapefile */
    char buf[4096];
    char field[128];
    VirtualShapePtr p_vt;
    const char *path = NULL;
    gaiaDbfFieldPtr pFld;
/* checking for shapefile PATH */
    if (argc >= 4)
	path = argv[3];
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualShape module] CREATE VIRTUAL: no shapefile path was specified");
	  return SQLITE_ERROR;
      }
    p_vt = (VirtualShapePtr) sqlite3_malloc (sizeof (VirtualShape));
    if (!p_vt)
	return SQLITE_NOMEM;
    p_vt->pModule = &my_module;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
    p_vt->db = db;
    p_vt->Shp = gaiaAllocShapefile ();
/* trying to open files etc in order to ensure we actually have a genuine shapefile */
    gaiaOpenShpRead (p_vt->Shp, path);
    if (!(p_vt->Shp->Valid))
	SQLITE_ERROR;
/* preparing the COLUMNs for this VIRTUAL TABLE */
    strcpy (buf, "CREATE TABLE ");
    strcat (buf, argv[1]);
    strcat (buf, " (PKUID INT, Geometry BLOB");
    pFld = p_vt->Shp->Dbf->First;
    while (pFld)
      {
	  if (pFld->Type == 'N')
	    {
		if (pFld->Decimals > 0 || pFld->Length > 9)
		    sprintf (field, "%s DOUBLE", pFld->Name);
		else
		    sprintf (field, "%s INTEGER", pFld->Name);
	    }
	  else
	      sprintf (field, "%s VARCHAR(%d)", pFld->Name, pFld->Length);
	  strcat (buf, ", ");
	  strcat (buf, field);
	  pFld = pFld->Next;
      }
    strcat (buf, ")");
    if (sqlite3_declare_vtab (db, buf) != SQLITE_OK)
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualShape module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
	       buf);
	  return SQLITE_ERROR;
      }
    *ppVTab = (sqlite3_vtab *) p_vt;
    return SQLITE_OK;
}

static int
vshp_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some shapefile - simply aliases vshp_create() */
    return vshp_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vshp_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIndex)
{
/* best index selection */
    return SQLITE_OK;
}

static int
vshp_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualShapePtr p_vt = (VirtualShapePtr) pVTab;
    if (p_vt->Shp)
	gaiaFreeShapefile (p_vt->Shp);
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
vshp_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vshp_disconnect() */
    return vshp_disconnect (pVTab);
}

static void
vshp_read_row (VirtualShapeCursorPtr cursor)
{
/* trying to read a "row" from shapefile */
    int ret;
    gaiaGeomCollPtr geom;
    if (cursor->blobGeometry)
      {
	  free (cursor->blobGeometry);
	  cursor->blobGeometry = NULL;
      }
    ret = gaiaReadShpEntity (cursor->pVtab->Shp, cursor->current_row);
    if (!ret)
      {
	  if (!(cursor->pVtab->Shp->LastError))	/* normal SHP EOF */
	    {
		cursor->eof = 1;
		return;
	    }
	  /* an error occurred */
	  fprintf (stderr, "%s\n", cursor->pVtab->Shp->LastError);
	  cursor->eof = 1;
	  return;
      }
    cursor->current_row++;
    geom = cursor->pVtab->Shp->Dbf->Geometry;
    if (geom)
      {
	  /* preparing the BLOB representing Geometry */
	  gaiaToSpatiaLiteBlobWkb (geom, &(cursor->blobGeometry),
				   &(cursor->blobSize));
      }
}

static int
vshp_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    VirtualShapeCursorPtr cursor =
	(VirtualShapeCursorPtr) sqlite3_malloc (sizeof (VirtualShapeCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->pVtab = (VirtualShapePtr) pVTab;
    cursor->current_row = 0;
    cursor->blobGeometry = NULL;
    cursor->blobSize = 0;
    cursor->eof = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    vshp_read_row (cursor);
    return SQLITE_OK;
}

static int
vshp_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    if (cursor->blobGeometry)
	free (cursor->blobGeometry);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vshp_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	     int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    return SQLITE_OK;
}

static int
vshp_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching a next row from cursor */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    vshp_read_row (cursor);
    return SQLITE_OK;
}

static int
vshp_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    return cursor->eof;
}

static int
vshp_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	     int column)
{
/* fetching value for the Nth column */
    int nCol = 2;
    gaiaGeomCollPtr geom;
    gaiaDbfFieldPtr pFld;
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    if (column == 0)
      {
	  /* the PRIMARY KEY column */
	  sqlite3_result_int (pContext, cursor->current_row);
	  return SQLITE_OK;
      }
    if (column == 1)
      {
	  /* the GEOMETRY column */
	  geom = cursor->pVtab->Shp->Dbf->Geometry;
	  if (geom)
	      sqlite3_result_blob (pContext, cursor->blobGeometry,
				   cursor->blobSize, SQLITE_STATIC);
	  else
	      sqlite3_result_null (pContext);
	  return SQLITE_OK;
      }
    pFld = cursor->pVtab->Shp->Dbf->First;
    while (pFld)
      {
	  /* column values */
	  if (nCol == column)
	    {
		if (!(pFld->Value))
		    sqlite3_result_null (pContext);
		else
		  {
		      switch (pFld->Value->Type)
			{
			case GAIA_INT_VALUE:
			    sqlite3_result_int (pContext,
						pFld->Value->IntValue);
			    break;
			case GAIA_DOUBLE_VALUE:
			    sqlite3_result_double (pContext,
						   pFld->Value->DblValue);
			    break;
			case GAIA_TEXT_VALUE:
			    sqlite3_result_text (pContext,
						 pFld->Value->TxtValue,
						 strlen (pFld->Value->TxtValue),
						 SQLITE_STATIC);
			    break;
			default:
			    sqlite3_result_null (pContext);
			    break;
			}
		  }
		break;
	    }
	  nCol++;
	  pFld = pFld->Next;
      }
    return SQLITE_OK;
}

static int
vshp_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    *pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vshp_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	     sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    return SQLITE_READONLY;
}

static int
vshp_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vshp_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vshp_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vshp_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

int
sqlite3VirtualShapeInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_module.iVersion = 1;
    my_module.xCreate = &vshp_create;
    my_module.xConnect = &vshp_connect;
    my_module.xBestIndex = &vshp_best_index;
    my_module.xDisconnect = &vshp_disconnect;
    my_module.xDestroy = &vshp_destroy;
    my_module.xOpen = &vshp_open;
    my_module.xClose = &vshp_close;
    my_module.xFilter = &vshp_filter;
    my_module.xNext = &vshp_next;
    my_module.xEof = &vshp_eof;
    my_module.xColumn = &vshp_column;
    my_module.xRowid = &vshp_rowid;
    my_module.xUpdate = &vshp_update;
    my_module.xBegin = &vshp_begin;
    my_module.xSync = &vshp_sync;
    my_module.xCommit = &vshp_commit;
    my_module.xRollback = &vshp_rollback;
    my_module.xFindFunction = NULL;
    sqlite3_create_module_v2 (db, "VirtualShape", &my_module, NULL, 0);
    return rc;
}

int
virtualshape_extension_init (sqlite3 * db, const sqlite3_api_routines * pApi)
{
	SQLITE_EXTENSION_INIT2(pApi)
	return sqlite3VirtualShapeInit (db);
}
