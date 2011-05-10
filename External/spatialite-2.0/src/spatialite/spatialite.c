/*

 spatialite.c -- SQLite3 spatial extension

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

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include <sqlite3ext.h>
#include <spatialite/gaiageo.h>
#include <spatialite/spatialite.h>
#include <spatialite.h>

static SQLITE_EXTENSION_INIT1

struct spatial_index_str
{
/* a struct to implement a linked list of spatial-indexes */
    char Valid;
    char *IndexName;
    char *Column;
    struct spatial_index_str *Next;
};

static void
fnct_InitSpatialMetaData (sqlite3_context * context, int argc,
			  sqlite3_value ** argv)
{
/* SQL function:
/ InitSpatialMetaData(void)
/
/ creates the SPATIAL_REF_SYS and GEOMETRY_COLUMNS tables
/ returns 1 on success
/ 0 on failure
*/
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
/* creating the SPATIAL_REF_SYS tables */
    strcpy (sql, "CREATE TABLE spatial_ref_sys ");
    strcat (sql, "(srid INTEGER NOT NULL PRIMARY KEY, ");
    strcat (sql, "auth_name VARCHAR(256) NOT NULL, ");
    strcat (sql, "auth_srid INTEGER NOT NULL, ");
    strcat (sql, "ref_sys_name VARCHAR(256), ");
    strcat (sql, "proj4text VARCHAR(2048) NOT NULL)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* setting a trigger to ensure referential integrity on DELETE */
    strcpy (sql,
	    "CREATE TRIGGER fkd_refsys_geocols BEFORE DELETE ON spatial_ref_sys ");
    strcat (sql, "FOR EACH ROW BEGIN ");
    strcat (sql,
	    "SELECT RAISE(ROLLBACK, 'delete on table ''spatial_ref_sys'' violates constraint: ''geometry_columns.srid''') ");
    strcat (sql,
	    "WHERE (SELECT srid FROM geometry_columns WHERE srid = OLD.srid) IS NOT NULL; ");
    strcat (sql, "END;");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* creating the GEOMETRY_COLUMN tables */
    strcpy (sql, "CREATE TABLE geometry_columns ");
    strcat (sql, "(f_table_name VARCHAR(256) NOT NULL, ");
    strcat (sql, "f_geometry_column VARCHAR(256) NOT NULL, ");
    strcat (sql, "type VARCHAR(30) NOT NULL, ");
    strcat (sql, "coord_dimension INTEGER NOT NULL, ");
    strcat (sql, "srid INTEGER, ");
    strcat (sql, "spatial_index_enabled INTEGER NOT NULL)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* setting a trigger to ensure referential integrity on INSERT */
    strcpy (sql,
	    "CREATE TRIGGER fki_geocols_refsys BEFORE INSERT ON geometry_columns ");
    strcat (sql, "FOR EACH ROW BEGIN ");
    strcat (sql,
	    "SELECT RAISE(ROLLBACK, 'insert on table ''geometry_columns'' violates constraint: ''spatial_ref_sys.srid''') ");
    strcat (sql, "WHERE  NEW.srid IS NOT NULL ");
    strcat (sql,
	    "AND (SELECT srid FROM spatial_ref_sys WHERE srid = NEW.srid) IS NULL; ");
    strcat (sql, "END;");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* setting a trigger to ensure referential integrity on UPDATE */
    strcpy (sql,
	    "CREATE TRIGGER fku_geocols_refsys BEFORE UPDATE ON geometry_columns ");
    strcat (sql, "FOR EACH ROW BEGIN ");
    strcat (sql,
	    "SELECT RAISE(ROLLBACK, 'update on table ''geometry_columns'' violates constraint: ''spatial_ref_sys.srid''') ");
    strcat (sql, "WHERE  NEW.srid IS NOT NULL ");
    strcat (sql,
	    "AND (SELECT srid FROM spatial_ref_sys WHERE srid = NEW.srid) IS NULL; ");
    strcat (sql, "END;");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* creating an UNIQUE INDEX */
    strcpy (sql, "CREATE UNIQUE INDEX idx_geocols ON geometry_columns ");
    strcat (sql, "(f_table_name, f_geometry_column)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "InitSpatiaMetaData() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static int
recoverGeomColumn (sqlite3 * sqlite, const unsigned char *table,
		   const unsigned char *column, int xtype, int srid)
{
/* checks if TABLE.COLUMN exists and has the required features */
    int ok = 1;
    char sql[1024];
    char *type;
    char *aliasType;
    sqlite3_stmt *stmt;
    gaiaGeomCollPtr geom;
    const void *blob_value;
    int len;
    int ret;
    int i_col;
    sprintf (sql, "SELECT %s FROM %s", column, table);
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "recoverGeomColumn: error %d \"%s\"\n",
		   sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  return 0;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* cecking Geometry features */
		geom = NULL;
		for (i_col = 0; i_col < sqlite3_column_count (stmt); i_col++)
		  {
		      if (sqlite3_column_type (stmt, i_col) != SQLITE_BLOB)
			  ok = 0;
		      else
			{
			    blob_value = sqlite3_column_blob (stmt, i_col);
			    len = sqlite3_column_bytes (stmt, i_col);
			    geom = gaiaFromSpatiaLiteBlobWkb (blob_value, len);
			    if (!geom)
				ok = 0;
			    else
			      {
				  if (geom->Srid != srid)
				      ok = 0;
				  gaiaGeometryType (geom, &type);
				  gaiaGeometryAliasType (geom, &aliasType);
				  if (type && aliasType)
				    {
					switch (xtype)
					  {
					  case GAIA_POINT:
					      if (strcasecmp (type, "POINT") ==
						  0
						  || strcasecmp (aliasType,
								 "POINT") == 0)
						  ;
					      else
						  ok = 0;
					      break;
					  case GAIA_MULTIPOINT:
					      if (strcasecmp
						  (type, "MULTIPOINT") == 0
						  || strcasecmp (aliasType,
								 "MULTIPOINT")
						  == 0)
						  ;
					      else
						  ok = 0;
					      break;
					  case GAIA_LINESTRING:
					      if (strcasecmp
						  (type, "LINESTRING") == 0
						  || strcasecmp (aliasType,
								 "LINESTRING")
						  == 0)
						  ;
					      else
						  ok = 0;
					      break;
					  case GAIA_MULTILINESTRING:
					      if (strcasecmp
						  (type, "MULTILINESTRING") == 0
						  || strcasecmp (aliasType,
								 "MULTILINESTRING")
						  == 0)
						  ;
					      else
						  ok = 0;
					      break;
					  case GAIA_POLYGON:
					      if (strcasecmp (type, "POLYGON")
						  == 0
						  || strcasecmp (aliasType,
								 "POLYGON") ==
						  0)
						  ;
					      else
						  ok = 0;
					      break;
					  case GAIA_MULTIPOLYGON:
					      if (strcasecmp
						  (type, "MULTIPOLYGON") == 0
						  || strcasecmp (aliasType,
								 "MULTIPOLYGON")
						  == 0)
						  ;
					      else
						  ok = 0;
					      break;
					  case GAIA_GEOMETRYCOLLECTION:
					      if (strcasecmp
						  (type,
						   "GEOMETRYCOLLECTION") == 0
						  || strcasecmp (aliasType,
								 "GEOMETRYCOLLECTION")
						  == 0)
						  ;
					      else
						  ok = 0;
					      break;
					  };
				    }
				  else
				      ok = 0;
				  gaiaFreeGeomColl (geom);
				  if (type)
				      free (type);
				  if (aliasType);
				  free (aliasType);
			      }
			}
		  }
	    }
	  if (!ok)
	      break;
      }
    ret = sqlite3_finalize (stmt);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "recoverGeomColumn: error %d \"%s\"\n",
		   sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  return 0;
      }
    return ok;
}

static void
buildSpatialIndex (sqlite3 * sqlite, const unsigned char *table, char *col_name)
{
/* loading a SpatialIndex [RTree] */
    char sql[2048];
    char dummy[512];
    char *errMsg = NULL;
    int ret;
    sqlite3_stmt *stmt;
    int i_col;
    int type_value;
    int pk;
    double minx;
    double maxx;
    double miny;
    double maxy;
    strcpy (sql, "SELECT ROWID, ");
    sprintf (dummy, "MbrMinX(%s), ", col_name);
    strcat (sql, dummy);
    sprintf (dummy, "MbrMaxX(%s), ", col_name);
    strcat (sql, dummy);
    sprintf (dummy, "MbrMinY(%s), ", col_name);
    strcat (sql, dummy);
    sprintf (dummy, "MbrMaxY(%s) ", col_name);
    strcat (sql, dummy);
    sprintf (dummy, "FROM %s", table);
    strcat (sql, dummy);
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "buildSpatialIndex: error %d \"%s\"\n",
		   sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  return;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* fetching MBRs and inserting them into the SpatialIndex */
		pk = -1;
		minx = DBL_MAX;
		maxx = DBL_MIN;
		miny = DBL_MAX;
		maxy = DBL_MIN;
		for (i_col = 0; i_col < sqlite3_column_count (stmt); i_col++)
		  {
		      /* fetching column values */
		      type_value = sqlite3_column_type (stmt, i_col);
		      if (type_value == SQLITE_INTEGER && i_col == 0)
			  pk = sqlite3_column_int (stmt, i_col);
		      else if (type_value == SQLITE_FLOAT)
			{
			    switch (i_col)
			      {
			      case 1:
				  minx = sqlite3_column_double (stmt, i_col);
				  break;
			      case 2:
				  maxx = sqlite3_column_double (stmt, i_col);
				  break;
			      case 3:
				  miny = sqlite3_column_double (stmt, i_col);
				  break;
			      case 4:
				  maxy = sqlite3_column_double (stmt, i_col);
				  break;
			      }
			}
		  }
		sprintf (dummy,
			 "INSERT INTO idx_%s_%s (pkid, xmin, xmax, ymin, ymax) VALUES (%d, %1.4lf, %1.4lf, %1.4lf, %1.4lf)",
			 table, col_name, pk, minx, maxx, miny, maxy);
		ret = sqlite3_exec (sqlite, dummy, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "buildSpatialIndex: error %d \"%s\"\n",
		   sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  return;
      }
    return;
  error:
    fprintf (stderr, "buildSpatialIndex: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    ret = sqlite3_finalize (stmt);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "buildSpatialIndex: error %d \"%s\"\n",
		   sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
      }
}

static void
updateGeometryTriggers (sqlite3 * sqlite, const unsigned char *table,
			const unsigned char *column)
{
/* updates triggers for some Spatial Column */
    char sql[256];
    char trigger[4096];
    char **results;
    int ret;
    int rows;
    int columns;
    int i;
    char col_type[32];
    char col_srid[32];
    char col_index[32];
    int srid;
    int index;
    int len;
    char *errMsg = NULL;
    char dummy[512];
    struct spatial_index_str *first_idx = NULL;
    struct spatial_index_str *last_idx = NULL;
    struct spatial_index_str *curr_idx;
    struct spatial_index_str *next_idx;
    sprintf (sql,
	     "SELECT type, srid, spatial_index_enabled FROM geometry_columns WHERE f_table_name = '%s' AND f_geometry_column = '%s'",
	     table, column);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, &errMsg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "updateTableTriggers: \"%s\"\n", errMsg);
	  sqlite3_free (errMsg);
	  return;
      }
    for (i = 1; i <= rows; i++)
      {
	  /* preparing the triggers */
	  strcpy (col_type, results[(i * columns)]);
	  strcpy (col_srid, results[(i * columns) + 1]);
	  strcpy (col_index, results[(i * columns) + 2]);
	  srid = atoi (col_srid);
	  index = atoi (col_index);
	  /* deleting the old INSERT trigger TYPE [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS gti_%s_%s", table, column);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* inserting the new INSERT trigger TYPE */
	  sprintf (trigger, "CREATE TRIGGER gti_%s_%s BEFORE INSERT ON %s ",
		   table, column, table);
	  strcat (trigger, "FOR EACH ROW BEGIN ");
	  sprintf (dummy,
		   "SELECT RAISE(ROLLBACK, '''%s.%s'' violates Geometry constraint [geom-type not allowed]') ",
		   table, column);
	  strcat (trigger, dummy);
	  strcat (trigger, "WHERE (SELECT type FROM geometry_columns ");
	  sprintf (dummy,
		   "WHERE f_table_name = '%s' AND f_geometry_column = '%s' ",
		   table, column);
	  strcat (trigger, dummy);
	  sprintf (dummy,
		   "AND (type = GeometryType(NEW.%s) OR type = GeometryAliasType(NEW.%s))) IS NULL; ",
		   column, column);
	  strcat (trigger, dummy);
	  strcat (trigger, "END;");
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* deleting the old INSERT trigger SRID [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS gsi_%s_%s", table, column);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* inserting the new INSERT trigger SRID */
	  sprintf (trigger, "CREATE TRIGGER gsi_%s_%s BEFORE INSERT ON %s ",
		   table, column, table);
	  strcat (trigger, "FOR EACH ROW BEGIN ");
	  sprintf (dummy,
		   "SELECT RAISE(ROLLBACK, '''%s.%s'' violates Geometry constraint [SRID not allowed]') ",
		   table, column);
	  strcat (trigger, dummy);
	  strcat (trigger, "WHERE (SELECT srid FROM geometry_columns ");
	  sprintf (dummy,
		   "WHERE f_table_name = '%s' AND f_geometry_column = '%s' ",
		   table, column);
	  strcat (trigger, dummy);
	  sprintf (dummy, "AND srid = SRID(NEW.%s)) IS NULL; ", column, column);
	  strcat (trigger, dummy);
	  strcat (trigger, "END;");
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* deleting the old UPDATE trigger TYPE [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS gtu_%s_%s", table, column);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* inserting the new UPDATE trigger TYPE */
	  sprintf (trigger, "CREATE TRIGGER gtu_%s_%s BEFORE UPDATE ON %s ",
		   table, column, table);
	  strcat (trigger, "FOR EACH ROW BEGIN ");
	  sprintf (dummy,
		   "SELECT RAISE(ROLLBACK, '''%s.%s'' violates Geometry constraint [geom-type not allowed]') ",
		   table, column);
	  strcat (trigger, dummy);
	  strcat (trigger, "WHERE (SELECT type FROM geometry_columns ");
	  sprintf (dummy,
		   "WHERE f_table_name = '%s' AND f_geometry_column = '%s' ",
		   table, column);
	  strcat (trigger, dummy);
	  sprintf (dummy,
		   "AND (type = GeometryType(NEW.%s) OR type = GeometryAliasType(NEW.%s))) IS NULL; ",
		   column, column);
	  strcat (trigger, dummy);
	  strcat (trigger, "END;");
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* deleting the old UPDATE trigger SRID [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS gsu_%s_%s", table, column);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* inserting the new UPDATE trigger SRID */
	  sprintf (trigger, "CREATE TRIGGER gsu_%s_%s BEFORE UPDATE ON %s ",
		   table, column, table);
	  strcat (trigger, "FOR EACH ROW BEGIN ");
	  sprintf (dummy,
		   "SELECT RAISE(ROLLBACK, '''%s.%s'' violates Geometry constraint [SRID not allowed]') ",
		   table, column);
	  strcat (trigger, dummy);
	  strcat (trigger, "WHERE (SELECT srid FROM geometry_columns ");
	  sprintf (dummy,
		   "WHERE f_table_name = '%s' AND f_geometry_column = '%s' ",
		   table, column);
	  strcat (trigger, dummy);
	  sprintf (dummy, "AND srid = SRID(NEW.%s)) IS NULL; ", column, column);
	  strcat (trigger, dummy);
	  strcat (trigger, "END;");
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
/* inserting SpatialIndex infos into the linked list */
	  curr_idx = malloc (sizeof (struct spatial_index_str));
	  sprintf (trigger, "idx_%s_%s", table, column);
	  len = strlen (trigger);
	  curr_idx->IndexName = malloc (len + 1);
	  strcpy (curr_idx->IndexName, trigger);
	  len = strlen ((char *)column);
	  curr_idx->Column = malloc (len + 1);
	  strcpy (curr_idx->Column, (char *)column);
	  curr_idx->Valid = index;
	  curr_idx->Next = NULL;
	  if (!first_idx)
	      first_idx = curr_idx;
	  if (last_idx)
	      last_idx->Next = curr_idx;
	  last_idx = curr_idx;
	  /* deleting the old INSERT trigger SPATIAL_INDEX [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS gii_%s_%s", table, column);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  if (index)
	    {
		/* inserting the new INSERT trigger SRID */
		sprintf (trigger,
			 "CREATE TRIGGER gii_%s_%s AFTER INSERT ON %s ", table,
			 column, table);
		strcat (trigger, "FOR EACH ROW BEGIN ");
		sprintf (dummy,
			 "INSERT INTO idx_%s_%s (pkid, xmin, xmax, ymin, ymax) VALUES (NEW.ROWID, ",
			 table, column);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMinX(NEW.%s), ", column);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMaxX(NEW.%s), ", column);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMinY(NEW.%s), ", column);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMaxY(NEW.%s)); ", column);
		strcat (trigger, dummy);
		strcat (trigger, "END;");
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
	    }
	  sqlite3_free_table (results);
	  /* deleting the old UPDATE trigger SPATIAL_INDEX [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS giu_%s_%s", table, column);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  if (index)
	    {
		/* inserting the new UPDATE trigger SRID */
		sprintf (trigger,
			 "CREATE TRIGGER giu_%s_%s AFTER UPDATE ON %s ", table,
			 column, table);
		strcat (trigger, "FOR EACH ROW BEGIN ");
		sprintf (dummy, "UPDATE idx_%s_%s SET ", table, column);
		strcat (trigger, dummy);
		sprintf (dummy, "xmin = MbrMinX(NEW.%s), ", column);
		strcat (trigger, dummy);
		sprintf (dummy, "xmax = MbrMaxX(NEW.%s), ", column);
		strcat (trigger, dummy);
		sprintf (dummy, "ymin = MbrMinY(NEW.%s), ", column);
		strcat (trigger, dummy);
		sprintf (dummy, "ymax = MbrMaxY(NEW.%s) ", column);
		strcat (trigger, dummy);
		strcat (trigger, "WHERE pkid = NEW.ROWID; ");
		strcat (trigger, "END;");
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
	    }
	  /* deleting the old UPDATE trigger SPATIAL_INDEX [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS gid_%s_%s", table, column);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  if (index)
	    {
		/* inserting the new DELETE trigger SRID */
		sprintf (trigger,
			 "CREATE TRIGGER gid_%s_%s AFTER DELETE ON %s ", table,
			 column, table);
		strcat (trigger, "FOR EACH ROW BEGIN ");
		sprintf (dummy, "DELETE FROM idx_%s_%s WHERE pkid = NEW.ROWID;",
			 table, column);
		strcat (trigger, dummy);
		strcat (trigger, "END;");
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
	    }
      }
/* now we'll adjust any related SpatialIndex as required */
    curr_idx = first_idx;
    while (curr_idx)
      {
	  if (curr_idx->Valid)
	    {
		/* building RTree SpatialIndex */
		sprintf (trigger, "CREATE VIRTUAL TABLE %s USING rtree(",
			 curr_idx->IndexName);
		strcat (trigger, "pkid, xmin, xmax, ymin, ymax)");
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
		buildSpatialIndex (sqlite, table, curr_idx->Column);
	    }
	  curr_idx = curr_idx->Next;
      }
    goto index_cleanup;
  error:
    fprintf (stderr, "updateTableTriggers: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
  index_cleanup:
    curr_idx = first_idx;
    while (curr_idx)
      {
	  next_idx = curr_idx->Next;
	  if (curr_idx->IndexName)
	      free (curr_idx->IndexName);
	  if (curr_idx->Column)
	      free (curr_idx->Column);
	  free (curr_idx);
	  curr_idx = next_idx;
      }
}

static void
fnct_AddGeometryColumn (sqlite3_context * context, int argc,
			sqlite3_value ** argv)
{
/* SQL function:
/ AddGeometryColumn(table, column, srid, type , dimension )
/
/ creates a new COLUMN of given TYPE into TABLE; optionally can accept a SRID value as well
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    const unsigned char *type;
    int xtype;
    int srid = -1;
    int dimension = 2;
    char dummy[32];
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    if (sqlite3_value_type (argv[2]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 3 [SRID] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    srid = sqlite3_value_int (argv[2]);
    if (sqlite3_value_type (argv[3]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 4 [geometry_type] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    type = sqlite3_value_text (argv[3]);
    if (sqlite3_value_type (argv[4]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 5 [dimension] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    dimension = sqlite3_value_int (argv[4]);
    xtype = GAIA_UNKNOWN;
    if (strcasecmp ((char *) type, "POINT") == 0)
	xtype = GAIA_POINT;
    if (strcasecmp ((char *) type, "LINESTRING") == 0)
	xtype = GAIA_LINESTRING;
    if (strcasecmp ((char *) type, "POLYGON") == 0)
	xtype = GAIA_POLYGON;
    if (strcasecmp ((char *) type, "MULTIPOINT") == 0)
	xtype = GAIA_MULTIPOINT;
    if (strcasecmp ((char *) type, "MULTILINESTRING") == 0)
	xtype = GAIA_MULTILINESTRING;
    if (strcasecmp ((char *) type, "MULTIPOLYGON") == 0)
	xtype = GAIA_MULTIPOLYGON;
    if (strcasecmp ((char *) type, "POINT") == 0)
	xtype = GAIA_POINT;
    if (strcasecmp ((char *) type, "LINESTRING") == 0)
	xtype = GAIA_LINESTRING;
    if (strcasecmp ((char *) type, "GEOMETRYCOLLECTION") == 0)
	xtype = GAIA_GEOMETRYCOLLECTION;
    if (xtype == GAIA_UNKNOWN)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 3 [geometry_type] has an illegal value\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    if (dimension != 2)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 5 [dimension] current version only accepts dimension=2\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    strcpy (sql,
	    "INSERT INTO geometry_columns (f_table_name, f_geometry_column, type, ");
    strcat (sql, "coord_dimension, srid, spatial_index_enabled) VALUES (");
    strcat (sql, "'");
    strcat (sql, (char *) table);
    strcat (sql, "', '");
    strcat (sql, (char *) column);
    strcat (sql, "', '");
    switch (xtype)
      {
      case GAIA_POINT:
	  strcat (sql, "POINT");
	  break;
      case GAIA_LINESTRING:
	  strcat (sql, "LINESTRING");
	  break;
      case GAIA_POLYGON:
	  strcat (sql, "POLYGON");
	  break;
      case GAIA_MULTIPOINT:
	  strcat (sql, "MULTIPOINT");
	  break;
      case GAIA_MULTILINESTRING:
	  strcat (sql, "MULTILINESTRING");
	  break;
      case GAIA_MULTIPOLYGON:
	  strcat (sql, "MULTIPOLYGON");
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  strcat (sql, "GEOMETRYCOLLECTION");
	  break;
      };
    strcat (sql, "', 2, ");
    if (srid <= 0)
	strcat (sql, "-1");
    else
      {
	  sprintf (dummy, "%d", srid);
	  strcat (sql, dummy);
      }
    strcat (sql, ", 0)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    strcpy (sql, "ALTER TABLE ");
    strcat (sql, (char *) table);
    strcat (sql, " ADD COLUMN ");
    strcat (sql, (char *) column);
    strcat (sql, " ");
    switch (xtype)
      {
      case GAIA_POINT:
	  strcat (sql, "POINT");
	  break;
      case GAIA_LINESTRING:
	  strcat (sql, "LINESTRING");
	  break;
      case GAIA_POLYGON:
	  strcat (sql, "POLYGON");
	  break;
      case GAIA_MULTIPOINT:
	  strcat (sql, "MULTIPOINT");
	  break;
      case GAIA_MULTILINESTRING:
	  strcat (sql, "MULTILINESTRING");
	  break;
      case GAIA_MULTIPOLYGON:
	  strcat (sql, "MULTIPOLYGON");
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  strcat (sql, "GEOMETRYCOLLECTION");
	  break;
      };
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    updateGeometryTriggers (sqlite, table, column);
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "AddGeometryColumn() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_RecoverGeometryColumn (sqlite3_context * context, int argc,
			    sqlite3_value ** argv)
{
/* SQL function:
/ RecoverGeometryColumn(table, column, srid, type , dimension )
/
/ checks if an existing TABLE.COLUMN satisfies the required geometric features
/ if yes adds it to SpatialMetaData and enabling triggers
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    const unsigned char *type;
    int xtype;
    int srid = -1;
    int dimension = 2;
    char dummy[32];
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    if (sqlite3_value_type (argv[2]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 3 [SRID] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    srid = sqlite3_value_int (argv[2]);
    if (sqlite3_value_type (argv[3]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 4 [geometry_type] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    type = sqlite3_value_text (argv[3]);
    if (sqlite3_value_type (argv[4]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 5 [dimension] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    dimension = sqlite3_value_int (argv[4]);
    xtype = GAIA_UNKNOWN;
    if (strcasecmp ((char *) type, "POINT") == 0)
	xtype = GAIA_POINT;
    if (strcasecmp ((char *) type, "LINESTRING") == 0)
	xtype = GAIA_LINESTRING;
    if (strcasecmp ((char *) type, "POLYGON") == 0)
	xtype = GAIA_POLYGON;
    if (strcasecmp ((char *) type, "MULTIPOINT") == 0)
	xtype = GAIA_MULTIPOINT;
    if (strcasecmp ((char *) type, "MULTILINESTRING") == 0)
	xtype = GAIA_MULTILINESTRING;
    if (strcasecmp ((char *) type, "MULTIPOLYGON") == 0)
	xtype = GAIA_MULTIPOLYGON;
    if (strcasecmp ((char *) type, "POINT") == 0)
	xtype = GAIA_POINT;
    if (strcasecmp ((char *) type, "LINESTRING") == 0)
	xtype = GAIA_LINESTRING;
    if (strcasecmp ((char *) type, "GEOMETRYCOLLECTION") == 0)
	xtype = GAIA_GEOMETRYCOLLECTION;
    if (xtype == GAIA_UNKNOWN)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 3 [geometry_type] has an illegal value\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    if (dimension != 2)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 5 [dimension] current version only accepts dimension=2\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    if (!recoverGeomColumn (sqlite, table, column, xtype, srid))
      {
	  fprintf (stderr, "RecoverGeometryColumn(): validation failed\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    strcpy (sql,
	    "INSERT INTO geometry_columns (f_table_name, f_geometry_column, type, ");
    strcat (sql, "coord_dimension, srid, spatial_index_enabled) VALUES (");
    strcat (sql, "'");
    strcat (sql, (char *) table);
    strcat (sql, "', '");
    strcat (sql, (char *) column);
    strcat (sql, "', '");
    switch (xtype)
      {
      case GAIA_POINT:
	  strcat (sql, "POINT");
	  break;
      case GAIA_LINESTRING:
	  strcat (sql, "LINESTRING");
	  break;
      case GAIA_POLYGON:
	  strcat (sql, "POLYGON");
	  break;
      case GAIA_MULTIPOINT:
	  strcat (sql, "MULTIPOINT");
	  break;
      case GAIA_MULTILINESTRING:
	  strcat (sql, "MULTILINESTRING");
	  break;
      case GAIA_MULTIPOLYGON:
	  strcat (sql, "MULTIPOLYGON");
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  strcat (sql, "GEOMETRYCOLLECTION");
	  break;
      };
    strcat (sql, "', 2, ");
    if (srid <= 0)
	strcat (sql, "-1");
    else
      {
	  sprintf (dummy, "%d", srid);
	  strcat (sql, dummy);
      }
    strcat (sql, ", 0)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    updateGeometryTriggers (sqlite, table, column);
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "RecoverGeometryColumn() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_DiscardGeometryColumn (sqlite3_context * context, int argc,
			    sqlite3_value ** argv)
{
/* SQL function:
/ DiscardGeometryColumn(table, column)
/
/ removes TABLE.COLUMN from the Spatial MetaData [thus disablig triggers too]
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "DiscardGeometryColumn() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "DiscardGeometryColumn() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    sprintf (sql,
	     "DELETE FROM geometry_columns WHERE f_table_name = '%s' AND f_geometry_column = '%s'",
	     (char *) table, (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* removing triggers too */
    sprintf (sql, "DROP TRIGGER IF EXISTS gti_%s_%s", (char *) table,
	     (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql, "DROP TRIGGER IF EXISTS gsu_%s_%s", (char *) table,
	     (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql, "DROP TRIGGER IF EXISTS gsi_%s_%s", (char *) table,
	     (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql, "DROP TRIGGER IF EXISTS gtu_%s_%s", (char *) table,
	     (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql, "DROP TRIGGER IF EXISTS gii_%s_%s", (char *) table,
	     (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql, "DROP TRIGGER IF EXISTS giu_%s_%s", (char *) table,
	     (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql, "DROP TRIGGER IF EXISTS gid_%s_%s", (char *) table,
	     (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "DiscardGeometryColumn() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_CreateSpatialIndex (sqlite3_context * context, int argc,
			 sqlite3_value ** argv)
{
/* SQL function:
/ CreateSpatialIndex(table, column )
/
/ creates a SpatialIndex based on Column and Table
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "CreateSpatialIndex() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "CreateSpatialIndex() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    strcpy (sql,
	    "UPDATE geometry_columns SET spatial_index_enabled = 1 WHERE f_table_name = '");
    strcat (sql, (char *) table);
    strcat (sql, "' AND f_geometry_column = '");
    strcat (sql, (char *) column);
    strcat (sql, "' AND spatial_index_enabled = 0");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    if (sqlite3_changes (sqlite) == 0)
      {
	  fprintf (stderr,
		   "CreateSpatialIndex() error: either '%s.%s' isn't a Geometry column or a SpatialIndex is already defined\n",
		   table, column);
	  return;
      }
    updateGeometryTriggers (sqlite, table, column);
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "CreateSpatialIndex() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_DisableSpatialIndex (sqlite3_context * context, int argc,
			  sqlite3_value ** argv)
{
/* SQL function:
/ DisableSpatialIndex(table, column )
/
/ disables a SpatialIndex based on Column and Table
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "DisableSpatialIndex() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "DisableSpatialIndex() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    strcpy (sql,
	    "UPDATE geometry_columns SET spatial_index_enabled = 0 WHERE f_table_name = '");
    strcat (sql, (char *) table);
    strcat (sql, "' AND f_geometry_column = '");
    strcat (sql, (char *) column);
    strcat (sql, "' AND spatial_index_enabled <> 0");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    if (sqlite3_changes (sqlite) == 0)
      {
	  fprintf (stderr,
		   "DisableSpatialIndex() error: either '%s.%s' isn't a Geometry column or no SpatialIndex is defined\n",
		   table, column);
	  return;
      }
    updateGeometryTriggers (sqlite, table, column);
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "DisableSpatialIndex() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static gaiaPointPtr
simplePoint (gaiaGeomCollPtr geo)
{
/* helper function
/ if this GEOMETRY contains only one POINT, and no other elementary geometry
/ the POINT address will be returned
/ otherwise NULL will be returned
*/
    int cnt = 0;
    gaiaPointPtr point;
    gaiaPointPtr this_point = NULL;
    if (!geo)
	return NULL;
    if (geo->FirstLinestring || geo->FirstPolygon)
	return NULL;
    point = geo->FirstPoint;
    while (point)
      {
	  /* counting how many POINTs are there */
	  cnt++;
	  this_point = point;
	  point = point->Next;
      }
    if (cnt == 1 && this_point)
	return this_point;
    return NULL;
}

static gaiaLinestringPtr
simpleLinestring (gaiaGeomCollPtr geo)
{
/* helper function
/ if this GEOMETRY contains only one LINESTRING, and no other elementary geometry
/ the LINESTRING address will be returned
/ otherwise NULL will be returned
*/
    int cnt = 0;
    gaiaLinestringPtr line;
    gaiaLinestringPtr this_line = NULL;
    if (!geo)
	return NULL;
    if (geo->FirstPoint || geo->FirstPolygon)
	return NULL;
    line = geo->FirstLinestring;
    while (line)
      {
	  /* counting how many LINESTRINGs are there */
	  cnt++;
	  this_line = line;
	  line = line->Next;
      }
    if (cnt == 1 && this_line)
	return this_line;
    return NULL;
}

static gaiaPolygonPtr
simplePolygon (gaiaGeomCollPtr geo)
{
/* helper function
/ if this GEOMETRY contains only one POLYGON, and no other elementary geometry
/ the POLYGON address will be returned
/ otherwise NULL will be returned
*/
    int cnt = 0;
    gaiaPolygonPtr polyg;
    gaiaPolygonPtr this_polyg = NULL;
    if (!geo)
	return NULL;
    if (geo->FirstPoint || geo->FirstLinestring)
	return NULL;
    polyg = geo->FirstPolygon;
    while (polyg)
      {
	  /* counting how many POLYGONs are there */
	  cnt++;
	  this_polyg = polyg;
	  polyg = polyg->Next;
      }
    if (cnt == 1 && this_polyg)
	return this_polyg;
    return NULL;
}

static void
fnct_AsText (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ AsText(BLOB encoded geometry)
/
/ returns the corresponding WKT encoded value
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaOutWkt (geo, &p_result);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	    {
		len = strlen (p_result);
		sqlite3_result_text (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_AsBinary (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ AsBinary(BLOB encoded geometry)
/
/ returns the corresponding WKB encoded value
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaToWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
geom_from_text1 (sqlite3_context * context, int argc, sqlite3_value ** argv,
		 short type)
{
/* SQL function:
/ GeomFromText(WKT encoded geometry)
/
/ returns the current geometry by parsing WKT encoded string 
/ or NULL if any error is encountered
/
/ if *type* is a negative value can accept any GEOMETRY CLASS
/ otherwise only requests conforming with required CLASS are valid
*/
    int len;
    unsigned char *p_result = NULL;
    const unsigned char *text;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_null (context);
	  return;
      }
    text = sqlite3_value_text (argv[0]);
    geo = gaiaParseWkt (text, type);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    sqlite3_result_blob (context, p_result, len, free);
}

static void
geom_from_text2 (sqlite3_context * context, int argc, sqlite3_value ** argv,
		 short type)
{
/* SQL function:
/ GeomFromText(WKT encoded geometry, SRID)
/
/ returns the current geometry by parsing WKT encoded string 
/ or NULL if any error is encountered
/
/ if *type* is a negative value can accept any GEOMETRY CLASS
/ otherwise only requests conforming with required CLASS are valid
*/
    int len;
    unsigned char *p_result = NULL;
    const unsigned char *text;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    text = sqlite3_value_text (argv[0]);
    geo = gaiaParseWkt (text, type);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = sqlite3_value_int (argv[1]);
    gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    sqlite3_result_blob (context, p_result, len, free);
}

static int
check_wkb (const unsigned char *wkb, int size, short type)
{
/* checking type coherency for WKB encoded GEOMETRY */
    int little_endian;
    int wkb_type;
    int endian_arch = gaiaEndianArch ();
    if (size < 5)
	return 0;		/* too short to be a WKB */
    if (*(wkb + 0) == 0x01)
	little_endian = GAIA_LITTLE_ENDIAN;
    else if (*(wkb + 0) == 0x00)
	little_endian = GAIA_BIG_ENDIAN;
    else
	return 0;		/* illegal byte ordering; neither BIG-ENDIAN nor LITTLE-ENDIAN */
    wkb_type = gaiaImport32 (wkb + 1, little_endian, endian_arch);
    if (wkb_type == GAIA_POINT || wkb_type == GAIA_LINESTRING
	|| wkb_type == GAIA_POLYGON || wkb_type == GAIA_MULTIPOINT
	|| wkb_type == GAIA_MULTILINESTRING || wkb_type == GAIA_MULTIPOLYGON
	|| wkb_type == GAIA_GEOMETRYCOLLECTION)
	;
    else
	return 0;		/* illegal GEOMETRY CLASS */
    if (type < 0)
	;			/* no restrinction about GEOMETRY CLASS TYPE */
    else
      {
	  if (wkb_type != type)
	      return 0;		/* invalid CLASS TYPE for request */
      }
    return 1;
}

static void
geom_from_wkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv,
		short type)
{
/* SQL function:
/ GeomFromWKB(WKB encoded geometry)
/
/ returns the current geometry by parsing WKB encoded blob 
/ or NULL if any error is encountered
/
/ if *type* is a negative value can accept any GEOMETRY CLASS
/ otherwise only requests conforming with required CLASS are valid
*/
    int len;
    int n_bytes;
    unsigned char *p_result = NULL;
    const unsigned char *wkb;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    wkb = sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!check_wkb (wkb, n_bytes, type))
	return;
    geo = gaiaFromWkb (wkb, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    sqlite3_result_blob (context, p_result, len, free);
}

static void
geom_from_wkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv,
		short type)
{
/* SQL function:
/ GeomFromWKB(WKB encoded geometry, SRID)
/
/ returns the current geometry by parsing WKB encoded string 
/ or NULL if any error is encountered
/
/ if *type* is a negative value can accept any GEOMETRY CLASS
/ otherwise only requests conforming with required CLASS are valid
*/
    int len;
    int n_bytes;
    unsigned char *p_result = NULL;
    const unsigned char *wkb;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    wkb = sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!check_wkb (wkb, n_bytes, type))
	return;
    geo = gaiaFromWkb (wkb, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = sqlite3_value_int (argv[1]);
    gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    sqlite3_result_blob (context, p_result, len, free);
}

/*
/ the following functions simply readdress the request to geom_from_text?()
/ setting the appropriate GEOMETRY CLASS TYPE
*/

static void
fnct_GeomFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) -1);
}

static void
fnct_GeomFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) -1);
}

static void
fnct_GeomCollFromText1 (sqlite3_context * context, int argc,
			sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_GEOMETRYCOLLECTION);
}

static void
fnct_GeomCollFromText2 (sqlite3_context * context, int argc,
			sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_GEOMETRYCOLLECTION);
}

static void
fnct_LineFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_LINESTRING);
}

static void
fnct_LineFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_LINESTRING);
}

static void
fnct_PointFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_POINT);
}

static void
fnct_PointFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_POINT);
}

static void
fnct_PolyFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_POLYGON);
}

static void
fnct_PolyFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_POLYGON);
}

static void
fnct_MLineFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_MULTILINESTRING);
}

static void
fnct_MLineFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_MULTILINESTRING);
}

static void
fnct_MPointFromText1 (sqlite3_context * context, int argc,
		      sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_MULTIPOINT);
}

static void
fnct_MPointFromText2 (sqlite3_context * context, int argc,
		      sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_MULTIPOINT);
}

static void
fnct_MPolyFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_MULTIPOLYGON);
}

static void
fnct_MPolyFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_MULTIPOLYGON);
}

/*
/ the following functions simply readdress the request to geom_from_wkb?()
/ setting the appropriate GEOMETRY CLASS TYPE
*/

static void
fnct_GeomFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) -1);
}

static void
fnct_GeomFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) -1);
}

static void
fnct_GeomCollFromWkb1 (sqlite3_context * context, int argc,
		       sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_GEOMETRYCOLLECTION);
}

static void
fnct_GeomCollFromWkb2 (sqlite3_context * context, int argc,
		       sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_GEOMETRYCOLLECTION);
}

static void
fnct_LineFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_LINESTRING);
}

static void
fnct_LineFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_LINESTRING);
}

static void
fnct_PointFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_POINT);
}

static void
fnct_PointFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_POINT);
}

static void
fnct_PolyFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_POLYGON);
}

static void
fnct_PolyFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_POLYGON);
}

static void
fnct_MLineFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_MULTILINESTRING);
}

static void
fnct_MLineFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_MULTILINESTRING);
}

static void
fnct_MPointFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_MULTIPOINT);
}

static void
fnct_MPointFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_MULTIPOINT);
}

static void
fnct_MPolyFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_MULTIPOLYGON);
}

static void
fnct_MPolyFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_MULTIPOLYGON);
}

static void
fnct_Dimension (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Dimension(BLOB encoded geometry)
/
/ returns:
/ 0 if geometry is a POINT or MULTIPOINT
/ 1 if geometry is a LINESTRING or MULTILINESTRING
/ 2 if geometry is a POLYGON or MULTIPOLYGON
/ 0, 1, 2, for GEOMETRYCOLLECTIONS according to geometries contained inside
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int dim;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  dim = gaiaDimension (geo);
	  sqlite3_result_int (context, dim);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_GeometryType (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ GeometryType(BLOB encoded geometry)
/
/ returns the class for current geometry:
/ 'POINT' or 'MULTIPOINT'
/ 'LINESTRING' or 'MULTILINESTRING'
/ 'POLYGON' or 'MULTIPOLYGON'
/ 'GEOMETRYCOLLECTION' 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaGeometryType (geo, &p_result);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	    {
		len = strlen (p_result);
		sqlite3_result_text (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_GeometryAliasType (sqlite3_context * context, int argc,
			sqlite3_value ** argv)
{
/* SQL function:
/ GeometryAliasType(BLOB encoded geometry)
/
/ returns the alias-class for current geometry:
/ 'MULTIPOINT'
/ 'MULTILINESTRING'
/ 'MULTIPOLYGON'
/ 'GEOMETRYCOLLECTION' 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaGeometryAliasType (geo, &p_result);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	    {
		len = strlen (p_result);
		sqlite3_result_text (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_SRID (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Srid(BLOB encoded geometry)
/
/ returns the SRID
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
	sqlite3_result_int (context, geo->Srid);
    gaiaFreeGeomColl (geo);
}

static void
fnct_SetSRID (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ SetSrid(BLOBencoded geometry, srid)
/
/ returns a new geometry that is the original one received, but with the new SRID [no coordinates translation is applied]
/ or NULL if any error is encountered
*/
    int endian_arch = gaiaEndianArch ();
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    int srid;
    unsigned char *p_result = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
	srid = sqlite3_value_int (argv[1]);
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  geo->Srid = srid;
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &n_bytes);
	  sqlite3_result_blob (context, p_result, n_bytes, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_IsEmpty (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ IsEmpty(BLOB encoded geometry)
/
/ returns:
/ 1 if this geometry contains no elementary geometries
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_int (context, 1);
    else
	sqlite3_result_int (context, gaiaIsEmpty (geo));
    gaiaFreeGeomColl (geo);
}

static void
fnct_Envelope (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Envelope(BLOB encoded geometry)
/
/ returns the MBR for current geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr bbox;
    gaiaPolygonPtr polyg;
    gaiaRingPtr rect;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  if (gaiaIsEmpty (geo))
	      sqlite3_result_null (context);
	  else
	    {
		gaiaMbrGeometry (geo);
		bbox = gaiaAllocGeomColl ();
		polyg = gaiaAddPolygonToGeomColl (bbox, 5, 0);
		rect = polyg->Exterior;
		gaiaSetPoint (rect->Coords, 0, geo->MinX, geo->MinY);	/* vertex # 1 */
		gaiaSetPoint (rect->Coords, 1, geo->MaxX, geo->MinY);	/* vertex # 2 */
		gaiaSetPoint (rect->Coords, 2, geo->MaxX, geo->MaxY);	/* vertex # 3 */
		gaiaSetPoint (rect->Coords, 3, geo->MinX, geo->MaxY);	/* vertex # 4 */
		gaiaSetPoint (rect->Coords, 4, geo->MinX, geo->MinY);	/* vertex # 5 [same as vertex # 1 to close the polygon] */
		gaiaToSpatiaLiteBlobWkb (bbox, &p_result, &len);
		gaiaFreeGeomColl (bbox);
		sqlite3_result_blob (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_BuildMbr (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ BuildMBR(double X1, double Y1, double X2, double Y2)
/
/ builds an MBR from two points (identifying a rectangle's diagonal) 
/ or NULL if any error is encountered
*/
    int len;
    unsigned char *p_result = NULL;
    double x1;
    double y1;
    double x2;
    double y2;
    int int_value;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x1 = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x1 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	y1 = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  y1 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
	x2 = sqlite3_value_double (argv[2]);
    else if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[2]);
	  x2 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[3]) == SQLITE_FLOAT)
	y2 = sqlite3_value_double (argv[3]);
    else if (sqlite3_value_type (argv[3]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[3]);
	  y2 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaBuildMbr (x1, y1, x2, y2, &p_result, &len);
    if (!p_result)
	sqlite3_result_null (context);
    else
	sqlite3_result_blob (context, p_result, len, free);
}

static void
fnct_BuildCircleMbr (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ BuildCircleMBR(double X, double Y, doubleradius)
/
/ builds an MBR from two points (identifying a rectangle's diagonal) 
/ or NULL if any error is encountered
*/
    int len;
    unsigned char *p_result = NULL;
    double x;
    double y;
    double radius;
    int int_value;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	y = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  y = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
	radius = sqlite3_value_double (argv[2]);
    else if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[2]);
	  radius = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaBuildCircleMbr (x, y, radius, &p_result, &len);
    if (!p_result)
	sqlite3_result_null (context);
    else
	sqlite3_result_blob (context, p_result, len, free);
}

static void
fnct_MbrMinX (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ MbrMinX(BLOB encoded GEMETRY)
/
/ returns the MinX coordinate for current geometry's MBR 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    double coord;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!gaiaGetMbrMinX (p_blob, n_bytes, &coord))
	sqlite3_result_null (context);
    else
	sqlite3_result_double (context, coord);
}

static void
fnct_MbrMaxX (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ MbrMaxX(BLOB encoded GEMETRY)
/
/ returns the MaxX coordinate for current geometry's MBR 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    double coord;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!gaiaGetMbrMaxX (p_blob, n_bytes, &coord))
	sqlite3_result_null (context);
    else
	sqlite3_result_double (context, coord);
}

static void
fnct_MbrMinY (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ MbrMinY(BLOB encoded GEMETRY)
/
/ returns the MinY coordinate for current geometry's MBR 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    double coord;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!gaiaGetMbrMinY (p_blob, n_bytes, &coord))
	sqlite3_result_null (context);
    else
	sqlite3_result_double (context, coord);
}

static void
fnct_MbrMaxY (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ MbrMaxY(BLOB encoded GEMETRY)
/
/ returns the MaxY coordinate for current geometry's MBR 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    double coord;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!gaiaGetMbrMaxY (p_blob, n_bytes, &coord))
	sqlite3_result_null (context);
    else
	sqlite3_result_double (context, coord);
}

static void
fnct_X (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ X(BLOB encoded POINT)
/
/ returns the X coordinate for current POINT geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaPointPtr point;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  point = simplePoint (geo);
	  if (!point)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, point->X);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Y (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Y(BLOB encoded POINT)
/
/ returns the Y coordinate for current POINT geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaPointPtr point;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  point = simplePoint (geo);
	  if (!point)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, point->Y);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_NumPoints (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ NumPoints(BLOB encoded LINESTRING)
/
/ returns the numer of vertices for current LINESTRING geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  line = simpleLinestring (geo);
	  if (!line)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_int (context, line->Points);
      }
    gaiaFreeGeomColl (geo);
}

static void
point_n (sqlite3_context * context, int argc, sqlite3_value ** argv,
	 int request)
{
/* SQL functions:
/ StartPoint(BLOB encoded LINESTRING geometry)
/ EndPoint(BLOB encoded LINESTRING geometry)
/ PointN(BLOB encoded LINESTRING geometry, integer point_no)
/
/ returns the Nth POINT for current LINESTRING geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int vertex;
    int len;
    int n_lines = 0;
    double x;
    double y;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (request == GAIA_POINTN)
      {
	  /* PointN() requires point index to be defined as an SQL function argument */
	  if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
	    {
		sqlite3_result_null (context);
		return;
	    }
	  vertex = sqlite3_value_int (argv[1]);
      }
    else if (request == GAIA_END_POINT)
	vertex = -1;		/* EndPoint() specifies a negative point index */
    else
	vertex = 1;		/* StartPoint() */
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  line = simpleLinestring (geo);
	  if (!line)
	      sqlite3_result_null (context);
	  else
	    {
		if (vertex < 0)
		    vertex = line->Points - 1;
		else
		    vertex -= 1;	/* decreasing the point index by 1, because PointN counts starting at index 1 */
		if (vertex >= 0 && vertex < line->Points)
		  {
		      gaiaGetPoint (line->Coords, vertex, &x, &y);
		      result = gaiaAllocGeomColl ();
		      gaiaAddPointToGeomColl (result, x, y);
		  }
		else
		    result = NULL;
		if (!result)
		    sqlite3_result_null (context);
		else
		  {
		      gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		      gaiaFreeGeomColl (result);
		      sqlite3_result_blob (context, p_result, len, free);
		  }
	    }
      }
    gaiaFreeGeomColl (geo);
}

/*
/ the following functions simply readdress the request to point_n()
/ setting the appropriate request mode
*/

static void
fnct_StartPoint (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    point_n (context, argc, argv, GAIA_START_POINT);
}

static void
fnct_EndPoint (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    point_n (context, argc, argv, GAIA_END_POINT);
}

static void
fnct_PointN (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    point_n (context, argc, argv, GAIA_POINTN);
}

static void
fnct_ExteriorRing (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL functions:
/ ExteriorRing(BLOB encoded POLYGON geometry)
/
/ returns the EXTERIOR RING for current POLYGON geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int iv;
    double x;
    double y;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    gaiaPolygonPtr polyg;
    gaiaRingPtr ring;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  polyg = simplePolygon (geo);
	  if (!polyg)
	      sqlite3_result_null (context);
	  else
	    {
		ring = polyg->Exterior;
		result = gaiaAllocGeomColl ();
		line = gaiaAddLinestringToGeomColl (result, ring->Points);
		for (iv = 0; iv < line->Points; iv++)
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		      gaiaSetPoint (line->Coords, iv, x, y);
		  }
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		gaiaFreeGeomColl (result);
		sqlite3_result_blob (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_NumInteriorRings (sqlite3_context * context, int argc,
		       sqlite3_value ** argv)
{
/* SQL function:
/ NumInteriorRings(BLOB encoded POLYGON)
/
/ returns the number of INTERIOR RINGS for current POLYGON geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaPolygonPtr polyg;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  polyg = simplePolygon (geo);
	  if (!polyg)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_int (context, polyg->NumInteriors);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_InteriorRingN (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL functions:
/ InteriorRingN(BLOB encoded POLYGON geometry)
/
/ returns the Nth INTERIOR RING for current POLYGON geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int border;
    int iv;
    double x;
    double y;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    gaiaPolygonPtr polyg;
    gaiaRingPtr ring;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    border = sqlite3_value_int (argv[1]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  polyg = simplePolygon (geo);
	  if (!polyg)
	      sqlite3_result_null (context);
	  else
	    {
		if (border >= 1 && border <= polyg->NumInteriors)
		  {
		      ring = polyg->Interiors + (border - 1);
		      result = gaiaAllocGeomColl ();
		      line = gaiaAddLinestringToGeomColl (result, ring->Points);
		      for (iv = 0; iv < line->Points; iv++)
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			    gaiaSetPoint (line->Coords, iv, x, y);
			}
		      gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		      gaiaFreeGeomColl (result);
		      sqlite3_result_blob (context, p_result, len, free);
		  }
		else
		    sqlite3_result_null (context);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_NumGeometries (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ NumGeometries(BLOB encoded GEOMETRYCOLLECTION)
/
/ returns the number of elementary geometries for current geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int cnt = 0;
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  point = geo->FirstPoint;
	  while (point)
	    {
		/* counts how many points are there */
		cnt++;
		point = point->Next;
	    }
	  line = geo->FirstLinestring;
	  while (line)
	    {
		/* counts how many linestrings are there */
		cnt++;
		line = line->Next;
	    }
	  polyg = geo->FirstPolygon;
	  while (polyg)
	    {
		/* counts how many polygons are there */
		cnt++;
		polyg = polyg->Next;
	    }
	  sqlite3_result_int (context, cnt);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_GeometryN (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ GeometryN(BLOB encoded GEOMETRYCOLLECTION geometry)
/
/ returns the Nth geometry for current GEOMETRYCOLLECTION or MULTIxxxx geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int entity;
    int len;
    int cnt = 0;
    int iv;
    int ib;
    double x;
    double y;
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaLinestringPtr line2;
    gaiaPolygonPtr polyg;
    gaiaPolygonPtr polyg2;
    gaiaRingPtr ring_in;
    gaiaRingPtr ring_out;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    entity = sqlite3_value_int (argv[1]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  point = geo->FirstPoint;
	  while (point)
	    {
		/* counts how many points are there */
		cnt++;
		if (cnt == entity)
		  {
		      /* ok, required elementary geometry is this POINT */
		      result = gaiaAllocGeomColl ();
		      gaiaAddPointToGeomColl (result, point->X, point->Y);
		      goto skip;
		  }
		point = point->Next;
	    }
	  line = geo->FirstLinestring;
	  while (line)
	    {
		/* counts how many linestrings are there */
		cnt++;
		if (cnt == entity)
		  {
		      /* ok, required elementary geometry is this LINESTRING */
		      result = gaiaAllocGeomColl ();
		      line2 =
			  gaiaAddLinestringToGeomColl (result, line->Points);
		      for (iv = 0; iv < line2->Points; iv++)
			{
			    gaiaGetPoint (line->Coords, iv, &x, &y);
			    gaiaSetPoint (line2->Coords, iv, x, y);
			}
		      goto skip;
		  }
		line = line->Next;
	    }
	  polyg = geo->FirstPolygon;
	  while (polyg)
	    {
		/* counts how many polygons are there */
		cnt++;
		if (cnt == entity)
		  {
		      /* ok, required elementary geometry is this POLYGON */
		      result = gaiaAllocGeomColl ();
		      ring_in = polyg->Exterior;
		      polyg2 =
			  gaiaAddPolygonToGeomColl (result, ring_in->Points,
						    polyg->NumInteriors);
		      ring_out = polyg2->Exterior;
		      for (iv = 0; iv < ring_out->Points; iv++)
			{
			    /* copying the exterior ring POINTs */
			    gaiaGetPoint (ring_in->Coords, iv, &x, &y);
			    gaiaSetPoint (ring_out->Coords, iv, x, y);
			}
		      for (ib = 0; ib < polyg2->NumInteriors; ib++)
			{
			    /* processing the interior rings */
			    ring_in = polyg->Interiors + ib;
			    ring_out =
				gaiaAddInteriorRing (polyg2, ib,
						     ring_in->Points);
			    for (iv = 0; iv < ring_out->Points; iv++)
			      {
				  gaiaGetPoint (ring_in->Coords, iv, &x, &y);
				  gaiaSetPoint (ring_out->Coords, iv, x, y);
			      }
			}
		      goto skip;
		  }
		polyg = polyg->Next;
	    }
	skip:
	  if (result)
	    {
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		gaiaFreeGeomColl (result);
		sqlite3_result_blob (context, p_result, len, free);
	    }
	  else
	      sqlite3_result_null (context);
      }
    gaiaFreeGeomColl (geo);
}

static void
mbrs_eval (sqlite3_context * context, int argc, sqlite3_value ** argv,
	   int request)
{
/* SQL function:
/ MBRsomething(BLOB encoded GEOMETRY-1, BLOB encoded GEOMETRY-2)
/
/ returns:
/ 1 if the required spatial relationship between the two MBRs is TRUE
/ 0 otherwise
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int ret;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobMbr (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobMbr (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  ret = 0;
	  gaiaMbrGeometry (geo1);
	  gaiaMbrGeometry (geo2);
	  switch (request)
	    {
	    case GAIA_MBR_CONTAINS:
		ret = gaiaMbrsContains (geo1, geo2);
		break;
	    case GAIA_MBR_DISJOINT:
		ret = gaiaMbrsDisjoint (geo1, geo2);
		break;
	    case GAIA_MBR_EQUAL:
		ret = gaiaMbrsEqual (geo1, geo2);
		break;
	    case GAIA_MBR_INTERSECTS:
		ret = gaiaMbrsIntersects (geo1, geo2);
		break;
	    case GAIA_MBR_OVERLAPS:
		ret = gaiaMbrsOverlaps (geo1, geo2);
		break;
	    case GAIA_MBR_TOUCHES:
		ret = gaiaMbrsTouches (geo1, geo2);
		break;
	    case GAIA_MBR_WITHIN:
		ret = gaiaMbrsWithin (geo1, geo2);
		break;
	    }
	  if (ret < 0)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

/*
/ the following functions simply readdress the mbr_eval()
/ setting the appropriate request mode
*/

static void
fnct_MbrContains (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_CONTAINS);
}

static void
fnct_MbrDisjoint (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_DISJOINT);
}

static void
fnct_MbrEqual (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_EQUAL);
}

static void
fnct_MbrIntersects (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_INTERSECTS);
}

static void
fnct_MbrOverlaps (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_OVERLAPS);
}

static void
fnct_MbrTouches (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_TOUCHES);
}

static void
fnct_MbrWithin (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_WITHIN);
}

static void
fnct_ShiftCoords (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ ShiftCoords(BLOBencoded geometry, shiftX, shiftY)
/
/ returns a new geometry that is the original one received, but with shifted coordinates
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    double shift_x;
    double shift_y;
    int int_value;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	shift_x = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  shift_x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
	shift_y = sqlite3_value_double (argv[2]);
    else if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[2]);
	  shift_y = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaShiftCoords (geo, shift_x, shift_y);
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_ScaleCoords (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ ScaleCoords(BLOBencoded geometry, scale_factor_x [, scale_factor_y])
/
/ returns a new geometry that is the original one received, but with scaled coordinates
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    double scale_x;
    double scale_y;
    int int_value;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	scale_x = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  scale_x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (argc == 2)
	scale_y = scale_x;	/* this one is an isotropic scaling request */
    else
      {
	  /* an anisotropic scaling is requested */
	  if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
	      scale_y = sqlite3_value_double (argv[2]);
	  else if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
	    {
		int_value = sqlite3_value_int (argv[2]);
		scale_y = int_value;
	    }
	  else
	    {
		sqlite3_result_null (context);
		return;
	    }
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaScaleCoords (geo, scale_x, scale_y);
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_RotateCoords (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ RotateCoords(BLOBencoded geometry, angle)
/
/ returns a new geometry that is the original one received, but with rotated coordinates
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    double angle;
    int int_value;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	angle = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  angle = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaRotateCoords (geo, angle);
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_ReflectCoords (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ ReflectCoords(BLOBencoded geometry, x_axis,  y_axis)
/
/ returns a new geometry that is the original one received, but with mirrored coordinates
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    int x_axis;
    int y_axis;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
	x_axis = sqlite3_value_int (argv[1]);
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
	y_axis = sqlite3_value_int (argv[2]);
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaReflectCoords (geo, x_axis, y_axis);
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_SwapCoords (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ SwapCoords(BLOBencoded geometry)
/
/ returns a new geometry that is the original one received, but with swapped x- and y-coordinate
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaSwapCoords (geo);
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
proj_params (sqlite3 * sqlite, int srid, char *proj_params)
{
/* retrives the PROJ params from SPATIAL_SYS_REF table, if possible */
    char sql[256];
    char **results;
    int rows;
    int columns;
    int i;
    char *errMsg = NULL;
    int ret = SQLITE_OK;
    *proj_params = '\0';
    sprintf (sql, "SELECT proj4text FROM spatial_ref_sys WHERE srid = %d",
	     srid);
    ret =	sqlite3_get_table (sqlite, sql, &results, &rows, &columns, &errMsg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "unknown SRID: %d\t<%s>\n", srid, errMsg);
	  sqlite3_free (errMsg);
	  return;
      }
    for (i = 1; i <= rows; i++)
	strcpy (proj_params, results[(i * columns)]);
    if (*proj_params == '\0')
	fprintf (stderr, "unknown SRID: %d\n", srid);
    sqlite3_free_table (results);
}

#if OMIT_PROJ == 0		/* including PROJ.4 */

static void
fnct_Transform (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Transform(BLOBencoded geometry, srid)
/
/ returns a new geometry that is the original one received, but with the new SRID [no coordinates translation is applied]
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    int srid_from;
    int srid_to;
    char proj_from[2048];
    char proj_to[2048];
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
	srid_to = sqlite3_value_int (argv[1]);
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  *proj_from = '\0';
	  *proj_to = '\0';
	  srid_from = geo->Srid;
	  proj_params (sqlite, srid_from, proj_from);
	  proj_params (sqlite, srid_to, proj_to);
	  if (*proj_to == '\0' || *proj_from == '\0')
	    {
		gaiaFreeGeomColl (geo);
		sqlite3_result_null (context);
		return;
	    }
	  result = gaiaTransform (geo, proj_from, proj_to);
	  if (!result)
	      sqlite3_result_null (context);
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		result->Srid = srid_to;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo);
}

#endif /* end including PROJ.4 */

#if OMIT_GEOS == 0		/* including GEOS */

static void
fnct_Boundary (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Boundary(BLOB encoded geometry)
/
/ returns the combinatioral boundary for current geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr boundary;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  if (gaiaIsEmpty (geo))
	      sqlite3_result_null (context);
	  else
	    {
		boundary = gaiaBoundary (geo);
		if (!boundary)
		    sqlite3_result_null (context);
		else
		  {
		      gaiaToSpatiaLiteBlobWkb (boundary, &p_result, &len);
		      gaiaFreeGeomColl (boundary);
		      sqlite3_result_blob (context, p_result, len, free);
		  }
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_IsClosed (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ IsClosed(BLOB encoded LINESTRING or MULTILINESTRING geometry)
/
/ returns:
/ 1 if this LINESTRING is closed [or if this is a MULTILINESTRING and every LINESTRINGs are closed] 
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_int (context, -1);
    else
      {
	  line = simpleLinestring (geo);
	  if (!line < 0)
	      sqlite3_result_int (context, -1);
	  else
	      sqlite3_result_int (context, gaiaIsClosed (line));
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_IsSimple (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ IsSimple(BLOB encoded GEOMETRY)
/
/ returns:
/ 1 if this GEOMETRY is simple
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int ret;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaIsSimple (geo);
	  if (ret < 0)
	      sqlite3_result_int (context, -1);
	  else
	      sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_IsRing (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ IsRing(BLOB encoded LINESTRING geometry)
/
/ returns:
/ 1 if this LINESTRING is a valid RING
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int ret;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_int (context, -1);
    else
      {
	  line = simpleLinestring (geo);
	  if (!line < 0)
	      sqlite3_result_int (context, -1);
	  else
	    {
		ret = gaiaIsRing (line);
		sqlite3_result_int (context, ret);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_IsValid (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ IsValid(BLOB encoded GEOMETRY)
/
/ returns:
/ 1 if this GEOMETRY is a valid one
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int ret;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaIsValid (geo);
	  if (ret < 0)
	      sqlite3_result_int (context, -1);
	  else
	      sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Length (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Length(BLOB encoded GEOMETRYCOLLECTION)
/
/ returns  the total length for current geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int ib;
    double length = 0.0;
    int ret;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  ret = gaiaGeomCollLength (geo, &length);
	  if (!ret)
	      sqlite3_result_null (context);
	  sqlite3_result_double (context, length);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Area (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Area(BLOB encoded GEOMETRYCOLLECTION)
/
/ returns the total area for current geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int ib;
    double area = 0.0;
    int ret;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  ret = gaiaGeomCollArea (geo, &area);
	  if (!ret)
	      sqlite3_result_null (context);
	  sqlite3_result_double (context, area);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Centroid (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Centroid(BLOBencoded POLYGON or MULTIPOLYGON geometry)
/
/ returns a POINT representing the centroid for current POLYGON / MULTIPOLYGON geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    int ret;
    double x;
    double y;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  if (gaiaIsEmpty (geo))
	      sqlite3_result_null (context);
	  else
	    {
		ret = gaiaGeomCollCentroid (geo, &x, &y);
		if (!ret)
		    sqlite3_result_null (context);
		else
		  {
		      result = gaiaAllocGeomColl ();
		      gaiaAddPointToGeomColl (result, x, y);
		      gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		      gaiaFreeGeomColl (result);
		      sqlite3_result_blob (context, p_result, len, free);
		  }
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_PointOnSurface (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ PointOnSurface(BLOBencoded POLYGON or MULTIPOLYGON geometry)
/
/ returns a POINT guaranteed to lie on the Surface
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    double x;
    double y;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  if (!gaiaGetPointOnSurface (geo, &x, &y))
	      sqlite3_result_null (context);
	  else
	    {
		result = gaiaAllocGeomColl ();
		gaiaAddPointToGeomColl (result, x, y);
		result->Srid = geo->Srid;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		gaiaFreeGeomColl (result);
		sqlite3_result_blob (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Simplify (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Simplify(BLOBencoded geometry, tolerance)
/
/ returns a new geometry that is a caricature of the original one received, but simplified using the Douglas-Peuker algorihtm
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    int int_value;
    double tolerance;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	tolerance = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  tolerance = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeomCollSimplify (geo, tolerance);
	  if (!result)
	      sqlite3_result_null (context);
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_SimplifyPreserveTopology (sqlite3_context * context, int argc,
			       sqlite3_value ** argv)
{
/* SQL function:
/ SimplifyPreserveTopology(BLOBencoded geometry, tolerance)
/
/ returns a new geometry that is a caricature of the original one received, but simplified using the Douglas-Peuker algorihtm [preserving topology]
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    int int_value;
    double tolerance;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	tolerance = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  tolerance = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeomCollSimplifyPreserveTopology (geo, tolerance);
	  if (!result)
	      sqlite3_result_null (context);
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_ConvexHull (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ ConvexHull(BLOBencoded geometry)
/
/ returns a new geometry representing the CONVEX HULL for current geometry
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaConvexHull (geo);
	  if (!result)
	      sqlite3_result_null (context);
	  else
	    {
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Buffer (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Buffer(BLOBencoded geometry, radius)
/
/ returns a new geometry representing the BUFFER for current geometry
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    double radius;
    int int_value;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	radius = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  radius = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeomCollBuffer (geo, radius, 30);
	  if (!result)
	      sqlite3_result_null (context);
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		result->Srid = geo->Srid;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Intersection (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Intersection(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns a new geometry representing the INTERSECTION of both geometries
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    gaiaGeomCollPtr result;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeometryIntersection (geo1, geo2);
	  if (!result)
	      sqlite3_result_null (context);
	  else if (gaiaIsEmpty (result))
	    {
		gaiaFreeGeomColl (result);
		sqlite3_result_null (context);
	    }
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Union (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Union(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns a new geometry representing the UNION of both geometries
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    gaiaGeomCollPtr result;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeometryUnion (geo1, geo2);
	  if (!result)
	      sqlite3_result_null (context);
	  else if (gaiaIsEmpty (result))
	    {
		gaiaFreeGeomColl (result);
		sqlite3_result_null (context);
	    }
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Difference (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Difference(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns a new geometry representing the DIFFERENCE of both geometries
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    gaiaGeomCollPtr result;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeometryDifference (geo1, geo2);
	  if (!result)
	      sqlite3_result_null (context);
	  else if (gaiaIsEmpty (result))
	    {
		gaiaFreeGeomColl (result);
		sqlite3_result_null (context);
	    }
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_SymDifference (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ SymDifference(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns a new geometry representing the SYMMETRIC DIFFERENCE of both geometries
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    gaiaGeomCollPtr result;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeometrySymDifference (geo1, geo2);
	  if (!result)
	      sqlite3_result_null (context);
	  else if (gaiaIsEmpty (result))
	    {
		gaiaFreeGeomColl (result);
		sqlite3_result_null (context);
	    }
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Equals (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Equals(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries are "spatially equal"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollEquals (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Intersects (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Intersects(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries do "spatially intersects"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollIntersects (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Disjoint (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Disjoint(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries are "spatially disjoint"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollDisjoint (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Overlaps (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Overlaps(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries do "spatially overlaps"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollOverlaps (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Crosses (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Crosses(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries do "spatially crosses"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollCrosses (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Touches (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Touches(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries do "spatially touches"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollTouches (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Within (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Within(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if GEOM-1 is completely contained within GEOM-2
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollWithin (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Contains (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Contains(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if GEOM-1 completely contains GEOM-2
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollContains (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Relate (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Relate(BLOBencoded geom1, BLOBencoded geom2, string pattern)
/
/ returns:
/ 1 if GEOM-1 and GEOM-2 have a spatial relationship as specified by the patternMatrix 
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    const unsigned char *pattern;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[2]) != SQLITE_TEXT)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    pattern = sqlite3_value_text (argv[2]);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollRelate (geo1, geo2, (char *) pattern);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Distance (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Distance(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns the distance between GEOM-1 and GEOM-2
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    double dist;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  ret = gaiaGeomCollDistance (geo1, geo2, &dist);
	  if (!ret)
	      sqlite3_result_null (context);
	  sqlite3_result_double (context, dist);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
geos_error (const char *fmt, ...)
{
/* reporting some GEOS warning/error */
    va_list ap;
    fprintf (stderr, "GEOS: ");
    va_start (ap, fmt);
    vfprintf (stdout, fmt, ap);
    va_end (ap);
    fprintf (stdout, "\n");
}

#endif /* end including GEOS */

static void
init_static_spatialite (sqlite3 * db, char **pzErrMsg,
			const sqlite3_api_routines * pApi)
{
    SQLITE_EXTENSION_INIT2 (pApi);
    sqlite3_create_function (db, "InitSpatialMetaData", 0, SQLITE_ANY, 0,
			     fnct_InitSpatialMetaData, 0, 0);
    sqlite3_create_function (db, "AddGeometryColumn", 5, SQLITE_ANY, 0,
			     fnct_AddGeometryColumn, 0, 0);
    sqlite3_create_function (db, "RecoverGeometryColumn", 5, SQLITE_ANY, 0,
			     fnct_RecoverGeometryColumn, 0, 0);
    sqlite3_create_function (db, "DiscardGeometryColumn", 2, SQLITE_ANY, 0,
			     fnct_DiscardGeometryColumn, 0, 0);
    sqlite3_create_function (db, "CreateSpatialIndex", 2, SQLITE_ANY, 0,
			     fnct_CreateSpatialIndex, 0, 0);
    sqlite3_create_function (db, "DisableSpatialIndex", 2, SQLITE_ANY, 0,
			     fnct_DisableSpatialIndex, 0, 0);
    sqlite3_create_function (db, "AsText", 1, SQLITE_ANY, 0, fnct_AsText, 0, 0);
    sqlite3_create_function (db, "AsBinary", 1, SQLITE_ANY, 0, fnct_AsBinary, 0,
			     0);
    sqlite3_create_function (db, "GeomFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomFromText1, 0, 0);
    sqlite3_create_function (db, "GeomFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomFromText2, 0, 0);
    sqlite3_create_function (db, "GeometryFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomFromText1, 0, 0);
    sqlite3_create_function (db, "GeometryFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomFromText2, 0, 0);
    sqlite3_create_function (db, "GeomCollFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromText1, 0, 0);
    sqlite3_create_function (db, "GeomCollFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromText2, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromText1, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromText2, 0, 0);
    sqlite3_create_function (db, "PointFromText", 1, SQLITE_ANY, 0,
			     fnct_PointFromText1, 0, 0);
    sqlite3_create_function (db, "PointFromText", 2, SQLITE_ANY, 0,
			     fnct_PointFromText2, 0, 0);
    sqlite3_create_function (db, "LineFromText", 1, SQLITE_ANY, 0,
			     fnct_LineFromText1, 0, 0);
    sqlite3_create_function (db, "LineFromText", 2, SQLITE_ANY, 0,
			     fnct_LineFromText2, 0, 0);
    sqlite3_create_function (db, "LineStringFromText", 1, SQLITE_ANY, 0,
			     fnct_LineFromText1, 0, 0);
    sqlite3_create_function (db, "LineStringFromText", 2, SQLITE_ANY, 0,
			     fnct_LineFromText2, 0, 0);
    sqlite3_create_function (db, "PolyFromText", 1, SQLITE_ANY, 0,
			     fnct_PolyFromText1, 0, 0);
    sqlite3_create_function (db, "PolyFromText", 2, SQLITE_ANY, 0,
			     fnct_PolyFromText2, 0, 0);
    sqlite3_create_function (db, "PolygonFromText", 1, SQLITE_ANY, 0,
			     fnct_PolyFromText1, 0, 0);
    sqlite3_create_function (db, "PolygonFromText", 2, SQLITE_ANY, 0,
			     fnct_PolyFromText2, 0, 0);
    sqlite3_create_function (db, "MPointFromText", 1, SQLITE_ANY, 0,
			     fnct_MPointFromText1, 0, 0);
    sqlite3_create_function (db, "MPointFromText", 2, SQLITE_ANY, 0,
			     fnct_MPointFromText2, 0, 0);
    sqlite3_create_function (db, "MultiPointFromText", 1, SQLITE_ANY, 0,
			     fnct_MPointFromText1, 0, 0);
    sqlite3_create_function (db, "MultiPointFromText", 2, SQLITE_ANY, 0,
			     fnct_MPointFromText2, 0, 0);
    sqlite3_create_function (db, "MLineFromText", 1, SQLITE_ANY, 0,
			     fnct_MLineFromText1, 0, 0);
    sqlite3_create_function (db, "MLineFromText", 2, SQLITE_ANY, 0,
			     fnct_MLineFromText2, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromText", 1, SQLITE_ANY, 0,
			     fnct_MLineFromText1, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromText", 2, SQLITE_ANY, 0,
			     fnct_MLineFromText2, 0, 0);
    sqlite3_create_function (db, "MPolyFromText", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromText1, 0, 0);
    sqlite3_create_function (db, "MPolyFromText", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromText2, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromText", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromText1, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromText", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromText2, 0, 0);
    sqlite3_create_function (db, "GeomFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeomFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeometryFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeometryFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeomCollFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeomCollFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb2, 0, 0);
    sqlite3_create_function (db, "PointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PointFromWkb1, 0, 0);
    sqlite3_create_function (db, "PointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PointFromWkb2, 0, 0);
    sqlite3_create_function (db, "LineFromWKB", 1, SQLITE_ANY, 0,
			     fnct_LineFromWkb1, 0, 0);
    sqlite3_create_function (db, "LineFromWKB", 2, SQLITE_ANY, 0,
			     fnct_LineFromWkb2, 0, 0);
    sqlite3_create_function (db, "LineStringFromWKB", 1, SQLITE_ANY, 0,
			     fnct_LineFromWkb1, 0, 0);
    sqlite3_create_function (db, "LineStringFromWKB", 2, SQLITE_ANY, 0,
			     fnct_LineFromWkb2, 0, 0);
    sqlite3_create_function (db, "PolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "PolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "PolygonFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "PolygonFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "MPointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPointFromWkb1, 0, 0);
    sqlite3_create_function (db, "MPointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPointFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiPointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPointFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiPointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPointFromWkb2, 0, 0);
    sqlite3_create_function (db, "MLineFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MLineFromWkb1, 0, 0);
    sqlite3_create_function (db, "MLineFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MLineFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MLineFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MLineFromWkb2, 0, 0);
    sqlite3_create_function (db, "MPolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "MPolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "Dimension", 1, SQLITE_ANY, 0, fnct_Dimension,
			     0, 0);
    sqlite3_create_function (db, "GeometryType", 1, SQLITE_ANY, 0,
			     fnct_GeometryType, 0, 0);
    sqlite3_create_function (db, "GeometryAliasType", 1, SQLITE_ANY, 0,
			     fnct_GeometryAliasType, 0, 0);
    sqlite3_create_function (db, "SRID", 1, SQLITE_ANY, 0, fnct_SRID, 0, 0);
    sqlite3_create_function (db, "SetSRID", 2, SQLITE_ANY, 0, fnct_SetSRID, 0,
			     0);
    sqlite3_create_function (db, "IsEmpty", 1, SQLITE_ANY, 0, fnct_IsEmpty, 0,
			     0);
    sqlite3_create_function (db, "Envelope", 1, SQLITE_ANY, 0, fnct_Envelope, 0,
			     0);
    sqlite3_create_function (db, "X", 1, SQLITE_ANY, 0, fnct_X, 0, 0);
    sqlite3_create_function (db, "Y", 1, SQLITE_ANY, 0, fnct_Y, 0, 0);
    sqlite3_create_function (db, "NumPoints", 1, SQLITE_ANY, 0, fnct_NumPoints,
			     0, 0);
    sqlite3_create_function (db, "StartPoint", 1, SQLITE_ANY, 0,
			     fnct_StartPoint, 0, 0);
    sqlite3_create_function (db, "EndPoint", 1, SQLITE_ANY, 0, fnct_EndPoint, 0,
			     0);
    sqlite3_create_function (db, "PointN", 2, SQLITE_ANY, 0, fnct_PointN, 0, 0);
    sqlite3_create_function (db, "ExteriorRing", 1, SQLITE_ANY, 0,
			     fnct_ExteriorRing, 0, 0);
    sqlite3_create_function (db, "NumInteriorRing", 1, SQLITE_ANY, 0,
			     fnct_NumInteriorRings, 0, 0);
    sqlite3_create_function (db, "NumInteriorRings", 1, SQLITE_ANY, 0,
			     fnct_NumInteriorRings, 0, 0);
    sqlite3_create_function (db, "InteriorRingN", 2, SQLITE_ANY, 0,
			     fnct_InteriorRingN, 0, 0);
    sqlite3_create_function (db, "NumGeometries", 1, SQLITE_ANY, 0,
			     fnct_NumGeometries, 0, 0);
    sqlite3_create_function (db, "GeometryN", 2, SQLITE_ANY, 0, fnct_GeometryN,
			     0, 0);
    sqlite3_create_function (db, "MBRContains", 2, SQLITE_ANY, 0,
			     fnct_MbrContains, 0, 0);
    sqlite3_create_function (db, "MbrDisjoint", 2, SQLITE_ANY, 0,
			     fnct_MbrDisjoint, 0, 0);
    sqlite3_create_function (db, "MBREqual", 2, SQLITE_ANY, 0, fnct_MbrEqual, 0,
			     0);
    sqlite3_create_function (db, "MbrIntersects", 2, SQLITE_ANY, 0,
			     fnct_MbrIntersects, 0, 0);
    sqlite3_create_function (db, "MBROverlaps", 2, SQLITE_ANY, 0,
			     fnct_MbrOverlaps, 0, 0);
    sqlite3_create_function (db, "MbrTouches", 2, SQLITE_ANY, 0,
			     fnct_MbrTouches, 0, 0);
    sqlite3_create_function (db, "MbrWithin", 2, SQLITE_ANY, 0, fnct_MbrWithin,
			     0, 0);
    sqlite3_create_function (db, "ShiftCoords", 3, SQLITE_ANY, 0,
			     fnct_ShiftCoords, 0, 0);
    sqlite3_create_function (db, "ShiftCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ShiftCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoords", 2, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoordinates", 2, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoords", 3, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "RotateCoords", 2, SQLITE_ANY, 0,
			     fnct_RotateCoords, 0, 0);
    sqlite3_create_function (db, "RotateCoordinates", 2, SQLITE_ANY, 0,
			     fnct_RotateCoords, 0, 0);
    sqlite3_create_function (db, "ReflectCoords", 3, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "ReflectCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "SwapCoords", 1, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "SwapCoordinates", 1, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "BuildMbr", 4, SQLITE_ANY, 0, fnct_BuildMbr, 0,
			     0);
    sqlite3_create_function (db, "BuildCircleMbr", 3, SQLITE_ANY, 0,
			     fnct_BuildCircleMbr, 0, 0);
    sqlite3_create_function (db, "MbrMinX", 1, SQLITE_ANY, 0, fnct_MbrMinX, 0,
			     0);
    sqlite3_create_function (db, "MbrMaxX", 1, SQLITE_ANY, 0, fnct_MbrMaxX, 0,
			     0);
    sqlite3_create_function (db, "MbrMinY", 1, SQLITE_ANY, 0, fnct_MbrMinY, 0,
			     0);
    sqlite3_create_function (db, "MbrMaxY", 1, SQLITE_ANY, 0, fnct_MbrMaxY, 0,
			     0);

#if OMIT_PROJ == 0		/* including PROJ.4 */

    sqlite3_create_function (db, "Transform", 2, SQLITE_ANY, 0, fnct_Transform,
			     0, 0);

#endif /* end including PROJ.4 */

#if OMIT_GEOS == 0		/* including GEOS */

    initGEOS (geos_error, geos_error);
    sqlite3_create_function (db, "Boundary", 1, SQLITE_ANY, 0, fnct_Boundary, 0,
			     0);
    sqlite3_create_function (db, "IsClosed", 1, SQLITE_ANY, 0, fnct_IsClosed, 0,
			     0);
    sqlite3_create_function (db, "IsSimple", 1, SQLITE_ANY, 0, fnct_IsSimple, 0,
			     0);
    sqlite3_create_function (db, "IsRing", 1, SQLITE_ANY, 0, fnct_IsRing, 0, 0);
    sqlite3_create_function (db, "IsValid", 1, SQLITE_ANY, 0, fnct_IsValid, 0,
			     0);
    sqlite3_create_function (db, "GLength", 1, SQLITE_ANY, 0, fnct_Length, 0,
			     0);
    sqlite3_create_function (db, "Area", 1, SQLITE_ANY, 0, fnct_Area, 0, 0);
    sqlite3_create_function (db, "Centroid", 1, SQLITE_ANY, 0, fnct_Centroid, 0,
			     0);
    sqlite3_create_function (db, "PointOnSurface", 1, SQLITE_ANY, 0,
			     fnct_PointOnSurface, 0, 0);
    sqlite3_create_function (db, "Simplify", 2, SQLITE_ANY, 0, fnct_Simplify, 0,
			     0);
    sqlite3_create_function (db, "SimplifyPreserveTopology", 2, SQLITE_ANY, 0,
			     fnct_SimplifyPreserveTopology, 0, 0);
    sqlite3_create_function (db, "ConvexHull", 1, SQLITE_ANY, 0,
			     fnct_ConvexHull, 0, 0);
    sqlite3_create_function (db, "Buffer", 2, SQLITE_ANY, 0, fnct_Buffer, 0, 0);
    sqlite3_create_function (db, "Intersection", 2, SQLITE_ANY, 0,
			     fnct_Intersection, 0, 0);
    sqlite3_create_function (db, "GUnion", 2, SQLITE_ANY, 0, fnct_Union, 0, 0);
    sqlite3_create_function (db, "Difference", 2, SQLITE_ANY, 0,
			     fnct_Difference, 0, 0);
    sqlite3_create_function (db, "SymDifference", 2, SQLITE_ANY, 0,
			     fnct_SymDifference, 0, 0);
    sqlite3_create_function (db, "Equals", 2, SQLITE_ANY, 0, fnct_Equals, 0, 0);
    sqlite3_create_function (db, "Intersects", 2, SQLITE_ANY, 0,
			     fnct_Intersects, 0, 0);
    sqlite3_create_function (db, "Disjoint", 2, SQLITE_ANY, 0, fnct_Disjoint, 0,
			     0);
    sqlite3_create_function (db, "Overlaps", 2, SQLITE_ANY, 0, fnct_Overlaps, 0,
			     0);
    sqlite3_create_function (db, "Crosses", 2, SQLITE_ANY, 0, fnct_Crosses, 0,
			     0);
    sqlite3_create_function (db, "Touches", 2, SQLITE_ANY, 0, fnct_Touches, 0,
			     0);
    sqlite3_create_function (db, "Within", 2, SQLITE_ANY, 0, fnct_Within, 0, 0);
    sqlite3_create_function (db, "Contains", 2, SQLITE_ANY, 0, fnct_Contains, 0,
			     0);
    sqlite3_create_function (db, "Relate", 3, SQLITE_ANY, 0, fnct_Relate, 0, 0);
    sqlite3_create_function (db, "Distance", 2, SQLITE_ANY, 0, fnct_Distance, 0,
			     0);

#endif /* end including GEOS */

/* initializing the VirtualShape  extension */
    virtualshape_extension_init (db, pApi);
/* initializing the RTree  extension[Spatial Index] */
    rtree_extension_init (db, pApi);
/* setting a timeout handler */
    sqlite3_busy_timeout (db, 5000);
}

void
spatialite_init ()
{
/* used when SQLite initializes SpatiaLite via statically linked lib */
    sqlite3_auto_extension ((void *) init_static_spatialite);
#if OMIT_PROJ == 0		/* PROJ.4 version */
    fprintf (stderr, "PROJ.4 %s\n", pj_get_release ());
    fflush (stderr);
#endif /* end including PROJ.4 */
#if OMIT_GEOS == 0		/* GEOS version */
    fprintf (stderr, "GEOS version %s\n", GEOSversion ());
    fflush (stderr);
#endif /* end GEOS version */
    fprintf (stderr, "\t*** Extension 'RTree' ready [Spatial Index support]\n");
    fprintf (stderr,
	     "\t*** Extension 'VirtualShape' ready [direct Shapefile access]\n");
    fprintf (stderr,
	     "\t*** Extension 'SpatiaLite' ready [Spatial SQL support]\n");
    fflush (stderr);
}

SPATIALITE_DECLARE int
sqlite3_extension_init (sqlite3 * db, char **pzErrMsg,
			const sqlite3_api_routines * pApi)
{
/* SQLite invokes this routine once when it dynamically loads the extension. */
    SQLITE_EXTENSION_INIT2 (pApi);
    sqlite3_create_function (db, "InitSpatialMetaData", 0, SQLITE_ANY, 0,
			     fnct_InitSpatialMetaData, 0, 0);
    sqlite3_create_function (db, "AddGeometryColumn", 5, SQLITE_ANY, 0,
			     fnct_AddGeometryColumn, 0, 0);
    sqlite3_create_function (db, "RecoverGeometryColumn", 5, SQLITE_ANY, 0,
			     fnct_RecoverGeometryColumn, 0, 0);
    sqlite3_create_function (db, "DiscardGeometryColumn", 2, SQLITE_ANY, 0,
			     fnct_DiscardGeometryColumn, 0, 0);
    sqlite3_create_function (db, "CreateSpatialIndex", 2, SQLITE_ANY, 0,
			     fnct_CreateSpatialIndex, 0, 0);
    sqlite3_create_function (db, "DisableSpatialIndex", 2, SQLITE_ANY, 0,
			     fnct_DisableSpatialIndex, 0, 0);
    sqlite3_create_function (db, "AsText", 1, SQLITE_ANY, 0, fnct_AsText, 0, 0);
    sqlite3_create_function (db, "AsBinary", 1, SQLITE_ANY, 0, fnct_AsBinary, 0,
			     0);
    sqlite3_create_function (db, "GeomFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomFromText1, 0, 0);
    sqlite3_create_function (db, "GeomFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomFromText2, 0, 0);
    sqlite3_create_function (db, "GeometryFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomFromText1, 0, 0);
    sqlite3_create_function (db, "GeometryFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomFromText2, 0, 0);
    sqlite3_create_function (db, "GeomCollFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromText1, 0, 0);
    sqlite3_create_function (db, "GeomCollFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromText2, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromText1, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromText2, 0, 0);
    sqlite3_create_function (db, "LineFromText", 1, SQLITE_ANY, 0,
			     fnct_LineFromText1, 0, 0);
    sqlite3_create_function (db, "LineFromText", 2, SQLITE_ANY, 0,
			     fnct_LineFromText2, 0, 0);
    sqlite3_create_function (db, "LineStringFromText", 1, SQLITE_ANY, 0,
			     fnct_LineFromText1, 0, 0);
    sqlite3_create_function (db, "LineStringFromText", 2, SQLITE_ANY, 0,
			     fnct_LineFromText2, 0, 0);
    sqlite3_create_function (db, "MLineFromText", 1, SQLITE_ANY, 0,
			     fnct_MLineFromText1, 0, 0);
    sqlite3_create_function (db, "MLineFromText", 2, SQLITE_ANY, 0,
			     fnct_MLineFromText2, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromText", 1, SQLITE_ANY, 0,
			     fnct_MLineFromText1, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromText", 2, SQLITE_ANY, 0,
			     fnct_MLineFromText2, 0, 0);
    sqlite3_create_function (db, "MPointFromText", 1, SQLITE_ANY, 0,
			     fnct_MPointFromText1, 0, 0);
    sqlite3_create_function (db, "MPointFromText", 2, SQLITE_ANY, 0,
			     fnct_MPointFromText2, 0, 0);
    sqlite3_create_function (db, "MultiPointFromText", 1, SQLITE_ANY, 0,
			     fnct_MPointFromText1, 0, 0);
    sqlite3_create_function (db, "MultiPointFromText", 2, SQLITE_ANY, 0,
			     fnct_MPointFromText2, 0, 0);
    sqlite3_create_function (db, "MPolyFromText", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromText1, 0, 0);
    sqlite3_create_function (db, "MPolyFromText", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromText2, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromText", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromText1, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromText", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromText2, 0, 0);
    sqlite3_create_function (db, "PointFromText", 1, SQLITE_ANY, 0,
			     fnct_PointFromText1, 0, 0);
    sqlite3_create_function (db, "PointFromText", 2, SQLITE_ANY, 0,
			     fnct_PointFromText2, 0, 0);
    sqlite3_create_function (db, "PolyFromText", 1, SQLITE_ANY, 0,
			     fnct_PolyFromText1, 0, 0);
    sqlite3_create_function (db, "PolyFromText", 2, SQLITE_ANY, 0,
			     fnct_PolyFromText2, 0, 0);
    sqlite3_create_function (db, "PolygonFromText", 1, SQLITE_ANY, 0,
			     fnct_PolyFromText1, 0, 0);
    sqlite3_create_function (db, "PolygomFromText", 2, SQLITE_ANY, 0,
			     fnct_PolyFromText2, 0, 0);
    sqlite3_create_function (db, "GeomFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeomFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeometryFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeometryFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeomCollFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeomCollFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb2, 0, 0);
    sqlite3_create_function (db, "LineFromWKB", 1, SQLITE_ANY, 0,
			     fnct_LineFromWkb1, 0, 0);
    sqlite3_create_function (db, "LineFromWKB", 2, SQLITE_ANY, 0,
			     fnct_LineFromWkb2, 0, 0);
    sqlite3_create_function (db, "LineStringFromWKB", 1, SQLITE_ANY, 0,
			     fnct_LineFromWkb1, 0, 0);
    sqlite3_create_function (db, "LineStringFromWKB", 2, SQLITE_ANY, 0,
			     fnct_LineFromWkb2, 0, 0);
    sqlite3_create_function (db, "MLineFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MLineFromWkb1, 0, 0);
    sqlite3_create_function (db, "MLineFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MLineFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MLineFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MLineFromWkb2, 0, 0);
    sqlite3_create_function (db, "MPointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPointFromWkb1, 0, 0);
    sqlite3_create_function (db, "MPointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPointFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiPointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPointFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiPointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPointFromWkb2, 0, 0);
    sqlite3_create_function (db, "MPolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "MPolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiPolygomFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "PointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PointFromWkb1, 0, 0);
    sqlite3_create_function (db, "PointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PointFromWkb2, 0, 0);
    sqlite3_create_function (db, "PolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "PolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "PolygonFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "PolygonFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "Dimension", 1, SQLITE_ANY, 0, fnct_Dimension,
			     0, 0);
    sqlite3_create_function (db, "GeometryType", 1, SQLITE_ANY, 0,
			     fnct_GeometryType, 0, 0);
    sqlite3_create_function (db, "GeometryAliasType", 1, SQLITE_ANY, 0,
			     fnct_GeometryAliasType, 0, 0);
    sqlite3_create_function (db, "SRID", 1, SQLITE_ANY, 0, fnct_SRID, 0, 0);
    sqlite3_create_function (db, "SetSrid", 2, SQLITE_ANY, 0, fnct_SetSRID, 0,
			     0);
    sqlite3_create_function (db, "IsEmpty", 1, SQLITE_ANY, 0, fnct_IsEmpty, 0,
			     0);
    sqlite3_create_function (db, "Envelope", 1, SQLITE_ANY, 0, fnct_Envelope, 0,
			     0);
    sqlite3_create_function (db, "X", 1, SQLITE_ANY, 0, fnct_X, 0, 0);
    sqlite3_create_function (db, "Y", 1, SQLITE_ANY, 0, fnct_Y, 0, 0);
    sqlite3_create_function (db, "NumPoints", 1, SQLITE_ANY, 0, fnct_NumPoints,
			     0, 0);
    sqlite3_create_function (db, "StartPoint", 1, SQLITE_ANY, 0,
			     fnct_StartPoint, 0, 0);
    sqlite3_create_function (db, "EndPoint", 1, SQLITE_ANY, 0, fnct_EndPoint, 0,
			     0);
    sqlite3_create_function (db, "PointN", 2, SQLITE_ANY, 0, fnct_PointN, 0, 0);
    sqlite3_create_function (db, "ExteriorRing", 1, SQLITE_ANY, 0,
			     fnct_ExteriorRing, 0, 0);
    sqlite3_create_function (db, "NumInteriorRings", 1, SQLITE_ANY, 0,
			     fnct_NumInteriorRings, 0, 0);
    sqlite3_create_function (db, "NumInteriorRing", 1, SQLITE_ANY, 0,
			     fnct_NumInteriorRings, 0, 0);
    sqlite3_create_function (db, "InteriorRingN", 2, SQLITE_ANY, 0,
			     fnct_InteriorRingN, 0, 0);
    sqlite3_create_function (db, "NumGeometries", 1, SQLITE_ANY, 0,
			     fnct_NumGeometries, 0, 0);
    sqlite3_create_function (db, "GeometryN", 2, SQLITE_ANY, 0, fnct_GeometryN,
			     0, 0);
    sqlite3_create_function (db, "MBRContains", 2, SQLITE_ANY, 0,
			     fnct_MbrContains, 0, 0);
    sqlite3_create_function (db, "MBRDisjoint", 2, SQLITE_ANY, 0,
			     fnct_MbrDisjoint, 0, 0);
    sqlite3_create_function (db, "MBREqual", 2, SQLITE_ANY, 0, fnct_MbrEqual, 0,
			     0);
    sqlite3_create_function (db, "MBRIntersects", 2, SQLITE_ANY, 0,
			     fnct_MbrIntersects, 0, 0);
    sqlite3_create_function (db, "MBROverlaps", 2, SQLITE_ANY, 0,
			     fnct_MbrOverlaps, 0, 0);
    sqlite3_create_function (db, "MBRTouches", 2, SQLITE_ANY, 0,
			     fnct_MbrTouches, 0, 0);
    sqlite3_create_function (db, "MBRWithin", 2, SQLITE_ANY, 0, fnct_MbrWithin,
			     0, 0);
    sqlite3_create_function (db, "ShiftCoords", 3, SQLITE_ANY, 0,
			     fnct_ShiftCoords, 0, 0);
    sqlite3_create_function (db, "ShiftCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ShiftCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoords", 2, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoords", 3, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoordinates", 2, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "RotateCoords", 2, SQLITE_ANY, 0,
			     fnct_RotateCoords, 0, 0);
    sqlite3_create_function (db, "ReflectCoords", 3, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "RotateCoordinates", 2, SQLITE_ANY, 0,
			     fnct_RotateCoords, 0, 0);
    sqlite3_create_function (db, "ReflectCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "SwapCoordinates", 1, SQLITE_ANY, 0,
			     fnct_SwapCoords, 0, 0);
    sqlite3_create_function (db, "BuildMbr", 4, SQLITE_ANY, 0, fnct_BuildMbr, 0,
			     0);
    sqlite3_create_function (db, "BuildCircleMbr", 3, SQLITE_ANY, 0,
			     fnct_BuildCircleMbr, 0, 0);
    sqlite3_create_function (db, "MbrMinX", 1, SQLITE_ANY, 0, fnct_MbrMinX, 0,
			     0);
    sqlite3_create_function (db, "MbrMaxX", 1, SQLITE_ANY, 0, fnct_MbrMaxX, 0,
			     0);
    sqlite3_create_function (db, "MbrMinY", 1, SQLITE_ANY, 0, fnct_MbrMinY, 0,
			     0);
    sqlite3_create_function (db, "MbrMaxY", 1, SQLITE_ANY, 0, fnct_MbrMaxY, 0,
			     0);

#if OMIT_PROJ == 0		/* including PROJ.4 */

    sqlite3_create_function (db, "Transform", 2, SQLITE_ANY, 0, fnct_Transform,
			     0, 0);

#endif /* end including PROJ.4 */

#if OMIT_GEOS == 0		/* including GEOS */

    initGEOS (geos_error, geos_error);
    sqlite3_create_function (db, "Equals", 2, SQLITE_ANY, 0, fnct_Equals, 0, 0);
    sqlite3_create_function (db, "Intersects", 2, SQLITE_ANY, 0,
			     fnct_Intersects, 0, 0);
    sqlite3_create_function (db, "Disjoint", 2, SQLITE_ANY, 0, fnct_Disjoint, 0,
			     0);
    sqlite3_create_function (db, "Overlaps", 2, SQLITE_ANY, 0, fnct_Overlaps, 0,
			     0);
    sqlite3_create_function (db, "Crosses", 2, SQLITE_ANY, 0, fnct_Crosses, 0,
			     0);
    sqlite3_create_function (db, "Touches", 2, SQLITE_ANY, 0, fnct_Touches, 0,
			     0);
    sqlite3_create_function (db, "Within", 2, SQLITE_ANY, 0, fnct_Within, 0, 0);
    sqlite3_create_function (db, "Contains", 2, SQLITE_ANY, 0, fnct_Contains, 0,
			     0);
    sqlite3_create_function (db, "Relate", 3, SQLITE_ANY, 0, fnct_Relate, 0, 0);
    sqlite3_create_function (db, "Distance", 2, SQLITE_ANY, 0, fnct_Distance, 0,
			     0);
    sqlite3_create_function (db, "Intersection", 2, SQLITE_ANY, 0,
			     fnct_Intersection, 0, 0);
    sqlite3_create_function (db, "Difference", 2, SQLITE_ANY, 0,
			     fnct_Difference, 0, 0);
    sqlite3_create_function (db, "GUnion", 2, SQLITE_ANY, 0, fnct_Union, 0, 0);
    sqlite3_create_function (db, "SymDifference", 2, SQLITE_ANY, 0,
			     fnct_SymDifference, 0, 0);
    sqlite3_create_function (db, "Boundary", 1, SQLITE_ANY, 0, fnct_Boundary, 0,
			     0);
    sqlite3_create_function (db, "GLength", 1, SQLITE_ANY, 0, fnct_Length, 0,
			     0);
    sqlite3_create_function (db, "Area", 1, SQLITE_ANY, 0, fnct_Area, 0, 0);
    sqlite3_create_function (db, "Centroid", 1, SQLITE_ANY, 0, fnct_Centroid, 0,
			     0);
    sqlite3_create_function (db, "PointOnSurface", 1, SQLITE_ANY, 0,
			     fnct_PointOnSurface, 0, 0);
    sqlite3_create_function (db, "Simplify", 2, SQLITE_ANY, 0, fnct_Simplify, 0,
			     0);
    sqlite3_create_function (db, "SimplifyPreserveTopology", 2, SQLITE_ANY, 0,
			     fnct_SimplifyPreserveTopology, 0, 0);
    sqlite3_create_function (db, "ConvexHull", 1, SQLITE_ANY, 0,
			     fnct_ConvexHull, 0, 0);
    sqlite3_create_function (db, "Buffer", 2, SQLITE_ANY, 0, fnct_Buffer, 0, 0);
    sqlite3_create_function (db, "IsClosed", 1, SQLITE_ANY, 0, fnct_IsClosed, 0,
			     0);
    sqlite3_create_function (db, "IsSimple", 1, SQLITE_ANY, 0, fnct_IsSimple, 0,
			     0);
    sqlite3_create_function (db, "IsRing", 1, SQLITE_ANY, 0, fnct_IsRing, 0, 0);
    sqlite3_create_function (db, "IsValid", 1, SQLITE_ANY, 0, fnct_IsValid, 0,
			     0);

#endif /* end including GEOS */

/* initializing the VirtualShape  extension */
    virtualshape_extension_init (db, pApi);
/* initializing the RTree  extension[Spatial Index] */
    rtree_extension_init (db, pApi);
/* setting a timeout handler */
    sqlite3_busy_timeout (db, 5000);

#if OMIT_PROJ == 0		/* PROJ.4 version */
    fprintf (stderr, "PROJ.4 %s\n", pj_get_release ());
    fflush (stderr);
#endif /* end including PROJ.4 */
#if OMIT_GEOS == 0		/* GEOS version */
    fprintf (stderr, "GEOS version %s\n", GEOSversion ());
    fflush (stderr);
#endif /* end GEOS version */
    fprintf (stderr, "\t*** Extension 'RTree' ready [Spatial Index support]\n");
    fprintf (stderr,
	     "\t*** Extension 'VirtualShape' ready [direct Shapefile access]\n");
    fprintf (stderr,
	     "\t*** SQLite's extension 'SpatiaLite' is enabled ******\n");
    fflush (stderr);
    return 0;
}
