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

#include "sqlite3.h"
#include <spatialite/gaiageo.h>

static gaiaDbfFieldPtr
getDbfField (gaiaDbfListPtr list, char *name)
{
/* find a DBF attribute by name */
    gaiaDbfFieldPtr fld = list->First;
    while (fld)
      {
	  if (strcasecmp (fld->Name, name) == 0)
	      return fld;
	  fld = fld->Next;
      }
    return NULL;
}

static int
is_reserved_sqlite_name (char *name)
{
/* checks if column-name is an SQLite reserved keyword */
    char *reserved[] = {
	"ALL",
	"ALTER",
	"AND",
	"AS",
	"AUTOINCREMENT",
	"BETWEEN",
	"BY",
	"CASE",
	"CHECK",
	"COLLATE",
	"COMMIT",
	"CONSTRAINT",
	"CREATE",
	"CROSS",
	"DEFAULT",
	"DEFERRABLE",
	"DELETE",
	"DISTINCT",
	"DROP",
	"ELSE",
	"ESCAPE",
	"EXCEPT",
	"FOREIGN",
	"FROM",
	"FULL",
	"GLOB",
	"GROUP",
	"HAVING",
	"IN",
	"INDEX",
	"INNER",
	"INSERT",
	"INTERSECT",
	"INTO",
	"IS",
	"ISNULL",
	"JOIN",
	"LEFT",
	"LIKE",
	"LIMIT",
	"NATURAL",
	"NOT",
	"NOTNULL",
	"NULL",
	"ON",
	"OR",
	"ORDER",
	"OUTER",
	"PRIMARY",
	"REFERENCES",
	"RIGHT",
	"ROLLBACK",
	"SELECT",
	"SET",
	"TABLE",
	"THEN",
	"TO",
	"TRANSACTION",
	"UNION",
	"UNIQUE",
	"UPDATE",
	"USING",
	"VALUES",
	"WHEN",
	"WHERE",
	NULL
    };
    char **pw = reserved;
    while (*pw != NULL)
      {
	  if (strcasecmp (name, *pw) == 0)
	      return 1;
	  pw++;
      }
    return 0;
}

static int
is_reserved_sql_name (char *name)
{
/* checks if column-name is an SQL reserved keyword */
    char *reserved[] = {
	"ABSOLUTE",
	"ACTION",
	"ADD",
	"AFTER",
	"ALL",
	"ALLOCATE",
	"ALTER",
	"AND",
	"ANY",
	"ARE",
	"ARRAY",
	"AS",
	"ASC",
	"ASENSITIVE",
	"ASSERTION",
	"ASYMMETRIC",
	"AT",
	"ATOMIC",
	"AUTHORIZATION",
	"AVG",
	"BEFORE",
	"BEGIN",
	"BETWEEN",
	"BIGINT",
	"BINARY",
	"BIT",
	"BIT_LENGTH",
	"BLOB",
	"BOOLEAN",
	"BOTH",
	"BREADTH",
	"BY",
	"CALL",
	"CALLED",
	"CASCADE",
	"CASCADED",
	"CASE",
	"CAST",
	"CATALOG",
	"CHAR",
	"CHARACTER",
	"CHARACTER_LENGTH",
	"CHAR_LENGTH",
	"CHECK",
	"CLOB",
	"CLOSE",
	"COALESCE",
	"COLLATE",
	"COLLATION",
	"COLUMN",
	"COMMIT",
	"CONDITION",
	"CONNECT",
	"CONNECTION",
	"CONSTRAINT",
	"CONSTRAINTS",
	"CONSTRUCTOR",
	"CONTAINS",
	"CONTINUE",
	"CONVERT",
	"CORRESPONDING",
	"COUNT",
	"CREATE",
	"CROSS",
	"CUBE",
	"CURRENT",
	"CURRENT_DATE",
	"CURRENT_DEFAULT_TRANSFORM_GROUP",
	"CURRENT_PATH",
	"CURRENT_ROLE",
	"CURRENT_TIME",
	"CURRENT_TIMESTAMP",
	"CURRENT_TRANSFORM_GROUP_FOR_TYPE",
	"CURRENT_USER",
	"CURSOR",
	"CYCLE",
	"DATA",
	"DATE",
	"DAY",
	"DEALLOCATE",
	"DEC",
	"DECIMAL",
	"DECLARE",
	"DEFAULT",
	"DEFERRABLE",
	"DEFERRED",
	"DELETE",
	"DEPTH",
	"DEREF",
	"DESC",
	"DESCRIBE",
	"DESCRIPTOR",
	"DETERMINISTIC",
	"DIAGNOSTICS",
	"DISCONNECT",
	"DISTINCT",
	"DO",
	"DOMAIN",
	"DOUBLE",
	"DROP",
	"DYNAMIC",
	"EACH",
	"ELEMENT",
	"ELSE",
	"ELSEIF",
	"END",
	"EQUALS",
	"ESCAPE",
	"EXCEPT",
	"EXCEPTION",
	"EXEC",
	"EXECUTE",
	"EXISTS",
	"EXIT",
	"external",
	"EXTRACT",
	"FALSE",
	"FETCH",
	"FILTER",
	"FIRST",
	"FLOAT",
	"FOR",
	"FOREIGN",
	"FOUND",
	"FREE",
	"FROM",
	"FULL",
	"FUNCTION",
	"GENERAL",
	"GET",
	"GLOBAL",
	"GO",
	"GOTO",
	"GRANT",
	"GROUP",
	"GROUPING",
	"HANDLER",
	"HAVING",
	"HOLD",
	"HOUR",
	"IDENTITY",
	"IF",
	"IMMEDIATE",
	"IN",
	"INDICATOR",
	"INITIALLY",
	"INNER",
	"INOUT",
	"INPUT",
	"INSENSITIVE",
	"INSERT",
	"INT",
	"INTEGER",
	"INTERSECT",
	"INTERVAL",
	"INTO",
	"IS",
	"ISOLATION",
	"ITERATE",
	"JOIN",
	"KEY",
	"LANGUAGE",
	"LARGE",
	"LAST",
	"LATERAL",
	"LEADING",
	"LEAVE",
	"LEFT",
	"LEVEL",
	"LIKE",
	"LOCAL",
	"LOCALTIME",
	"LOCALTIMESTAMP",
	"LOCATOR",
	"LOOP",
	"LOWER",
	"MAP",
	"MATCH",
	"MAX",
	"MEMBER",
	"MERGE",
	"METHOD",
	"MIN",
	"MINUTE",
	"MODIFIES",
	"MODULE",
	"MONTH",
	"MULTISET",
	"NAMES",
	"NATIONAL",
	"NATURAL",
	"NCHAR",
	"NCLOB",
	"NEW",
	"NEXT",
	"NO",
	"NONE",
	"NOT",
	"NULL",
	"NULLIF",
	"NUMERIC",
	"OBJECT",
	"OCTET_LENGTH",
	"OF",
	"OLD",
	"ON",
	"ONLY",
	"OPEN",
	"OPTION",
	"OR",
	"ORDER",
	"ORDINALITY",
	"OUT",
	"OUTER",
	"OUTPUT",
	"OVER",
	"OVERLAPS",
	"PAD",
	"PARAMETER",
	"PARTIAL",
	"PARTITION",
	"PATH",
	"POSITION",
	"PRECISION",
	"PREPARE",
	"PRESERVE",
	"PRIMARY",
	"PRIOR",
	"PRIVILEGES",
	"PROCEDURE",
	"PUBLIC",
	"RANGE",
	"READ",
	"READS",
	"REAL",
	"RECURSIVE",
	"REF",
	"REFERENCES",
	"REFERENCING",
	"RELATIVE",
	"RELEASE",
	"REPEAT",
	"RESIGNAL",
	"RESTRICT",
	"RESULT",
	"RETURN",
	"RETURNS",
	"REVOKE",
	"RIGHT",
	"ROLE",
	"ROLLBACK",
	"ROLLUP",
	"ROUTINE",
	"ROW",
	"ROWS",
	"SAVEPOINT",
	"SCHEMA",
	"SCOPE",
	"SCROLL",
	"SEARCH",
	"SECOND",
	"SECTION",
	"SELECT",
	"SENSITIVE",
	"SESSION",
	"SESSION_USER",
	"SET",
	"SETS",
	"SIGNAL",
	"SIMILAR",
	"SIZE",
	"SMALLINT",
	"SOME",
	"SPACE",
	"SPECIFIC",
	"SPECIFICTYPE",
	"SQL",
	"SQLCODE",
	"SQLERROR",
	"SQLEXCEPTION",
	"SQLSTATE",
	"SQLWARNING",
	"START",
	"STATE",
	"STATIC",
	"SUBMULTISET",
	"SUBSTRING",
	"SUM",
	"SYMMETRIC",
	"SYSTEM",
	"SYSTEM_USER",
	"TABLE",
	"TABLESAMPLE",
	"TEMPORARY",
	"THEN",
	"TIME",
	"TIMESTAMP",
	"TIMEZONE_HOUR",
	"TIMEZONE_MINUTE",
	"TO",
	"TRAILING",
	"TRANSACTION",
	"TRANSLATE",
	"TRANSLATION",
	"TREAT",
	"TRIGGER",
	"TRIM",
	"TRUE",
	"UNDER",
	"UNDO",
	"UNION",
	"UNIQUE",
	"UNKNOWN",
	"UNNEST",
	"UNTIL",
	"UPDATE",
	"UPPER",
	"USAGE",
	"USER",
	"USING",
	"VALUE",
	"VALUES",
	"VARCHAR",
	"VARYING",
	"VIEW",
	"WHEN",
	"WHENEVER",
	"WHERE",
	"WHILE",
	"WINDOW",
	"WITH",
	"WITHIN",
	"WITHOUT",
	"WORK",
	"WRITE",
	"YEAR",
	"ZONE",
	NULL
    };
    char **pw = reserved;
    while (*pw != NULL)
      {
	  if (strcasecmp (name, *pw) == 0)
	      return 1;
	  pw++;
      }
    return 0;
}

static void
clean_sql_string (char *value)
{
/*
/ returns a well formatted TEXT value for SQL
/ 1] strips trailing spaces
/ 2] masks any ' inside the string, appending another '
*/
    char new_value[1024];
    char *p;
    int len;
    int i;
    len = strlen (value);
    for (i = (len - 1); i >= 0; i--)
      {
	  /* stripping trailing spaces */
	  if (value[i] == ' ')
	      value[i] = '\0';
	  else
	      break;
      }
    p = new_value;
    for (i = 0; i < len; i++)
      {
	  if (value[i] == '\'')
	      *(p++) = '\'';
	  *(p++) = value[i];
      }
    *p = '\0';
    strcpy (value, new_value);
}

void
load_shapefile (sqlite3 * sqlite, char *shp_path, char *table, int srid,
		char *column)
{
    sqlite3_stmt *stmt;
    int ret;
    char *errMsg = NULL;
    char sql[65536];
    char sql2[65536];
    char dummy[65536];
    int already_exists = 0;
    int metadata = 0;
    int sqlError = 0;
    gaiaShapefilePtr shp = NULL;
    gaiaDbfFieldPtr dbf_field;
    gaiaDbfFieldPtr fld2;
    int cnt;
    int col_cnt;
    int seed;
    int len;
    int dup;
    int current_row;
    char **col_name = NULL;
    char *hexWKB = NULL;
    int szSQL;
    char *bufSQL = NULL;
    char *geom_type;
    char *geo_column = column;
    if (!geo_column)
	geo_column = "Geometry";
/* checking if TABLE already exists */
    sprintf (sql,
	     "SELECT name FROM sqlite_master WHERE type = 'table' AND name = '%s'",
	     table);
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "load shapefile error: <%s>\n",
		   sqlite3_errmsg (sqlite));
	  return;
      }
    while (1)
      {
	  /* scrolling the result set */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      already_exists = 1;
	  else
	    {
		fprintf (stderr, "load shapefile error: <%s>\n",
			 sqlite3_errmsg (sqlite));
		break;
	    }
      }
    sqlite3_finalize (stmt);
    if (already_exists)
      {
	  fprintf (stderr, "load shapefile error: table '%s' already exists\n",
		   table);
	  return;
      }
/* checking if MetaData GEOMETRY_COLUMNS exists */
    strcpy (sql,
	    "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'geometry_columns'");
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "load shapefile error: <%s>\n",
		   sqlite3_errmsg (sqlite));
	  return;
      }
    while (1)
      {
	  /* scrolling the result set */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      metadata = 1;
	  else
	    {
		fprintf (stderr, "load shapefile error: <%s>\n",
			 sqlite3_errmsg (sqlite));
		break;
	    }
      }
    sqlite3_finalize (stmt);
    shp = gaiaAllocShapefile ();
    gaiaOpenShpRead (shp, shp_path);
    if (!(shp->Valid))
      {
	  fprintf (stderr, "load shapefile error: cannot open shapefile '%s'\n",
		   shp_path);
	  gaiaFreeShapefile (shp);
	  return;
      }
/* checking for duplicate / illegal column names and antialising them */
    col_cnt = 0;
    dbf_field = shp->Dbf->First;
    while (dbf_field)
      {
	  /* counting DBF fields */
	  col_cnt++;
	  dbf_field = dbf_field->Next;
      }
    col_name = malloc (sizeof (char *) * col_cnt);
    cnt = 0;
    seed = 0;
    dbf_field = shp->Dbf->First;
    while (dbf_field)
      {
	  /* preparing column names */
	  if (is_reserved_sql_name (dbf_field->Name))
	    {
		sprintf (dummy, "%s_X%d", dbf_field->Name, seed++);
		len = strlen (dummy);
		*(col_name + cnt) = malloc (len + 1);
		strcpy (*(col_name + cnt), dummy);
		cnt++;
		dbf_field = dbf_field->Next;
		continue;
	    }
	  if (is_reserved_sqlite_name (dbf_field->Name))
	    {
		sprintf (dummy, "%s_X%d", dbf_field->Name, seed++);
		len = strlen (dummy);
		*(col_name + cnt) = malloc (len + 1);
		strcpy (*(col_name + cnt), dummy);
		cnt++;
		dbf_field = dbf_field->Next;
		continue;
	    }
	  dup = 0;
	  fld2 = shp->Dbf->First;
	  while (fld2)
	    {
		/* checking for duplicates */
		if (fld2 == dbf_field)
		    break;
		if (strcasecmp (dbf_field->Name, fld2->Name) == 0)
		  {
		      dup = 1;
		      break;
		  }
		fld2 = fld2->Next;
	    }
	  if (strcasecmp (dbf_field->Name, "PK_UID") == 0)
	      dup = 1;
	  if (strcasecmp (dbf_field->Name, geo_column) == 0)
	      dup = 1;
	  if (dup)
	    {
		sprintf (dummy, "%s_X%d", dbf_field->Name, seed++);
		len = strlen (dummy);
		*(col_name + cnt) = malloc (len + 1);
		strcpy (*(col_name + cnt), dummy);
		cnt++;
		dbf_field = dbf_field->Next;
		continue;
	    }
	  len = strlen (dbf_field->Name);
	  *(col_name + cnt) = malloc (len + 1);
	  strcpy (*(col_name + cnt), dbf_field->Name);
	  cnt++;
	  dbf_field = dbf_field->Next;
      }
    fprintf (stderr,
	     "========\nLoading shapefile at '%s' into SQLite table '%s'\n",
	     shp_path, table);
/* starting a transaction */
    fprintf (stderr, "\nBEGIN;\n");
    ret = sqlite3_exec (sqlite, "BEGIN", NULL, 0, &errMsg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "load shapefile error: %<%s>\n", errMsg);
	  sqlite3_free (errMsg);
	  sqlError = 1;
	  goto clean_up;
      }
/* creating the Table */
    sprintf (sql, "CREATE TABLE %s", table);
    strcat (sql, " (\n\tPK_UID INTEGER PRIMARY KEY AUTOINCREMENT");
    cnt = 0;
    dbf_field = shp->Dbf->First;
    while (dbf_field)
      {
	  strcat (sql, ",\n\t");
	  strcat (sql, *(col_name + cnt));
	  cnt++;
	  switch (dbf_field->Type)
	    {
	    case 'C':
		strcat (sql, " TEXT");
		break;
	    case 'N':
		if (dbf_field->Decimals)
		    strcat (sql, " DOUBLE");
		else
		  {
		      if (dbf_field->Length <= 9)
			  strcat (sql, " INTEGER");
		      else
			  strcat (sql, " DOUBLE");
		  }
		break;
	    case 'D':
		strcat (sql, " TEXT");
		break;
	    case 'L':
		strcat (sql, " INTEGER");
		break;
	    };
	  strcat (sql, " NOT NULL");
	  dbf_field = dbf_field->Next;
      }
    if (metadata)
	strcat (sql, ")");
    else
      {
	  strcat (sql, ",\n\t");
	  strcat (sql, geo_column);
	  strcat (sql, " BLOB NOT NULL)");
      }
    fprintf (stderr, "%s;\n", sql);
    ret = sqlite3_exec (sqlite, sql, NULL, 0, &errMsg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "load shapefile error: %<%s>\n", errMsg);
	  sqlite3_free (errMsg);
	  sqlError = 1;
	  goto clean_up;
      }
    if (metadata)
      {
	  /* creating Geometry column */
	  switch (shp->Shape)
	    {
	    case 1:
	    case 11:
	    case 21:
		geom_type = "POINT";
		break;
	    case 8:
		geom_type = "MULTIPOINT";
		break;
	    case 3:
	    case 13:
	    case 23:
		geom_type = "MULTILINESTRING";
		break;
	    case 5:
	    case 15:
	    case 25:
		geom_type = "MULTIPOLYGON";
		break;
	    };
	  sprintf (sql, "SELECT AddGeometryColumn('%s', '%s', %d, '%s', 2)",
		   table, geo_column, srid, geom_type);
	  fprintf (stderr, "%s;\n", sql);
	  ret = sqlite3_exec (sqlite, sql, NULL, 0, &errMsg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "load shapefile error: %<%s>\n", errMsg);
		sqlite3_free (errMsg);
		sqlError = 1;
		goto clean_up;
	    }
      }
    current_row = 0;
    while (1)
      {
	  /* inserting rows from shapefile */
	  ret = gaiaReadShpEntity (shp, current_row);
	  if (!ret)
	    {
		if (!(shp->LastError))	/* normal SHP EOF */
		    break;
		fprintf (stderr, "%s\n", shp->LastError);
		sqlError = 1;
		goto clean_up;
	    }
	  current_row++;
	  sprintf (sql, "INSERT INTO %s (\nPK_UID,", table);
	  cnt = 0;
	  dbf_field = shp->Dbf->First;
	  while (dbf_field)
	    {
		/* columns corresponding to some DBF attribute */
		strcat (sql, *(col_name + cnt));
		cnt++;
		strcat (sql, ",");
		dbf_field = dbf_field->Next;
	    }
	  strcat (sql, geo_column);	/* the GEOMETRY column */
	  strcat (sql, ")\nVALUES (");
	  sprintf (dummy, "%d,", current_row);
	  strcat (sql, dummy);
	  dbf_field = shp->Dbf->First;
	  while (dbf_field)
	    {
		/* column values */
		if (!(dbf_field->Value))
		    strcat (sql, "NULL");
		else
		  {
		      switch (dbf_field->Value->Type)
			{
			case GAIA_INT_VALUE:
			    sprintf (dummy, "%d", dbf_field->Value->IntValue);
			    strcat (sql, dummy);
			    break;
			case GAIA_DOUBLE_VALUE:
			    sprintf (dummy, "%1.6lf",
				     dbf_field->Value->DblValue);
			    strcat (sql, dummy);
			    break;
			case GAIA_TEXT_VALUE:
			    strcpy (dummy, dbf_field->Value->TxtValue);
			    clean_sql_string (dummy);
			    strcat (sql, "'");
			    strcat (sql, dummy);
			    strcat (sql, "'");
			    break;
			default:
			    strcat (sql, "NULL");
			    break;
			}
		  }
		strcat (sql, ",");
		dbf_field = dbf_field->Next;
	    }
	  hexWKB = gaiaToHexWkb (shp->Dbf->Geometry);
	  szSQL = strlen (sql) + strlen (hexWKB) + 1024;
	  bufSQL = malloc (szSQL);
	  strcpy (bufSQL, sql);
	  strcat (bufSQL, "\n");
	  strcat (bufSQL, "GeomFromWkb(X'");
	  strcat (bufSQL, hexWKB);
	  sprintf (dummy, "', %d))", srid);
	  strcat (bufSQL, dummy);
	  if (current_row == 1)
	    {
		/* showing INSERT example for first row */
		strcpy (sql2, sql);
		strcat (sql2, "\n");
		strcat (sql2, "GeomFromWkb(X'");
		if (strlen (hexWKB) < 50)
		    strcat (sql2, hexWKB);
		else
		  {
		      strncat (sql2, hexWKB, 46);
		      strcat (sql2, "...");
		  }
		strcat (sql2, dummy);
		strcat (sql2, ";\n...\n");
		fprintf (stderr, "%s", sql2);
	    }
	  ret = sqlite3_exec (sqlite, bufSQL, NULL, 0, &errMsg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "load shapefile error: %<%s>\n", errMsg);
		sqlite3_free (errMsg);
		sqlError = 1;
		goto clean_up;
	    }
	  free (hexWKB);
	  hexWKB = NULL;
	  free (bufSQL);
	  bufSQL = NULL;
      }
  clean_up:
    if (hexWKB)
	free (hexWKB);
    if (bufSQL)
	free (bufSQL);
    gaiaFreeShapefile (shp);
    if (col_name)
      {
	  /* releasing memory allocation for column names */
	  for (cnt = 0; cnt < col_cnt; cnt++)
	      free (*(col_name + cnt));
	  free (col_name);
      }
    if (sqlError)
      {
	  /* some error occurred - ROLLBACK */
	  fprintf (stderr, "ROLLBACK;\n");
	  ret = sqlite3_exec (sqlite, "ROLLBACK", NULL, 0, &errMsg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "load shapefile error: %<%s>\n", errMsg);
		sqlite3_free (errMsg);
	    }
      }
    else
      {
	  /* ok - confirming pending transaction - COMMIT */
	  fprintf (stderr, "COMMIT;\n");
	  ret = sqlite3_exec (sqlite, "COMMIT", NULL, 0, &errMsg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "load shapefile error: %<%s>\n", errMsg);
		sqlite3_free (errMsg);
		return;
	    }
	  sprintf (dummy, "ANALYZE %s", table);
	  fprintf (stderr, "%s;\n", dummy);
	  ret = sqlite3_exec (sqlite, dummy, NULL, 0, &errMsg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "load shapefile error: %<%s>\n", errMsg);
		sqlite3_free (errMsg);
		return;
	    }
	  fprintf (stderr, "VACUUM;\n");
	  ret = sqlite3_exec (sqlite, "VACUUM", NULL, 0, &errMsg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "load shapefile error: %<%s>\n", errMsg);
		sqlite3_free (errMsg);
		return;
	    }
	  fprintf (stderr,
		   "\nInserted %d rows into '%s' from SHAPEFILE\n========\n",
		   current_row, table);
      }
}

void
dump_shapefile (sqlite3 * sqlite, char *table, char *column, char *shp_path,
		char *geom_type)
{
/* SHAPEFILE dump */
    char sql[1024];
    char dummy[1024];
    int shape = -1;
    int len;
    int ret;
    char *errMsg = NULL;
    sqlite3_stmt *stmt;
    int row1 = 0;
    int n_cols = 0;
    int offset = 0;
    int i;
    int rows = 0;
    int type;
    int multiple_entities = 0;
    const unsigned char *char_value;
    const void *blob_value;
    gaiaShapefilePtr shp = NULL;
    gaiaDbfListPtr dbf_export_list = NULL;
    gaiaDbfListPtr dbf_list = NULL;
    gaiaDbfListPtr dbf_write;
    gaiaDbfFieldPtr dbf_field;
    gaiaGeomCollPtr geom;
    int *max_length = NULL;
    int *sql_type = NULL;
    if (geom_type)
      {
	  /* normalizing required geometry type */
	  if (strcasecmp ((char *) geom_type, "POINT") == 0)
	      shape = GAIA_POINT;
	  if (strcasecmp ((char *) geom_type, "LINESTRING") == 0)
	      shape = GAIA_LINESTRING;
	  if (strcasecmp ((char *) geom_type, "POLYGON") == 0)
	      shape = GAIA_POLYGON;
      }
    if (shape < 0)
      {
	  /* preparing SQL statement [no type was explicitly required, so we'll read GEOMETRY_COLUMNS */
	  char **results;
	  int rows;
	  int columns;
	  int i;
	  char metatype[256];
	  sprintf (sql,
		   "SELECT type FROM geometry_columns WHERE f_table_name = '%s' AND f_geometry_column = '%s'",
		   table, column);
	  ret =
	      sqlite3_get_table (sqlite, sql, &results, &rows, &columns,
				 &errMsg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "dump shapefile MetaData error: %<%s>\n",
			 errMsg);
		sqlite3_free (errMsg);
		return;
	    }
	  for (i = 1; i <= rows; i++)
	      strcpy (metatype, results[(i * columns)]);
	  sqlite3_free_table (results);
	  if (strcasecmp ((char *) metatype, "POINT") == 0
	      || strcasecmp ((char *) metatype, "MULTIPOINT") == 0)
	      shape = GAIA_POINT;
	  if (strcasecmp ((char *) metatype, "LINESTRING") == 0
	      || strcasecmp ((char *) metatype, "MULTILINESTRING") == 0)
	      shape = GAIA_LINESTRING;
	  if (strcasecmp ((char *) metatype, "POLYGON") == 0
	      || strcasecmp ((char *) metatype, "MULTIPOLYGON") == 0)
	      shape = GAIA_POLYGON;
      }
    if (shape < 0)
      {
	  fprintf (stderr,
		   "Unable to detect GeometryType for %s.%s ... sorry\n",
		   table, column);
	  return;
      }
    fprintf (stderr,
	     "========\nDumping SQLite table '%s' into shapefile at '%s'\n",
	     table, shp_path);
    /* preparing SQL statement */
    sprintf (sql, "SELECT * FROM %s WHERE GeometryType(%s) = ", table, column);
    if (shape == GAIA_LINESTRING)
      {
	  strcat (sql, "'LINESTRING' OR GeometryType(");
	  strcat (sql, (char *) column);
	  strcat (sql, ") = 'MULTILINESTRING'");
      }
    else if (shape == GAIA_POLYGON)
      {
	  strcat (sql, "'POLYGON' OR GeometryType(");
	  strcat (sql, (char *) column);
	  strcat (sql, ") = 'MULTIPOLYGON'");
      }
    else
      {
	  strcat (sql, "'POINT' OR GeometryType(");
	  strcat (sql, (char *) column);
	  strcat (sql, ") = 'MULTIPOINT'");
      }
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
	goto sql_error;
    while (1)
      {
	  /* Pass I - scrolling the result set to compute real DBF attributes' sizes and types */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a result set row */
		row1++;
		if (n_cols == 0)
		  {
		      /* this one is the first row, so we are going to prepare the DBF Fields list */
		      n_cols = sqlite3_column_count (stmt);
		      dbf_export_list = gaiaAllocDbfList ();
		      max_length = malloc (sizeof (int) * n_cols);
		      sql_type = malloc (sizeof (int) * n_cols);
		      for (i = 0; i < n_cols; i++)
			{
			    /* initializes the DBF export fields */
			    strcpy (dummy, sqlite3_column_name (stmt, i));
			    gaiaAddDbfField (dbf_export_list, dummy, '\0', 0, 0,
					     0);
			    max_length[i] = 0;
			    sql_type[i] = SQLITE_NULL;
			}
		  }
		for (i = 0; i < n_cols; i++)
		  {
		      /* update the DBF export fields analyzing fetched data */
		      type = sqlite3_column_type (stmt, i);
		      if (type == SQLITE_BLOB &&
			  strcasecmp ((char *) column,
				      (char *) sqlite3_column_name (stmt,
								    i)) == 0
			  && shape == GAIA_POINT)
			{
/* 
 we need to check if there is any MULTIPOINT,
because shapefile handles simple-points shapes and multi-points shapes
in a complete differet way
*/
			    blob_value = sqlite3_column_blob (stmt, i);
			    len = sqlite3_column_bytes (stmt, i);
			    geom = gaiaFromSpatiaLiteBlobWkb (blob_value, len);
			    if (geom)
			      {
				  if (geom->FirstPoint != geom->LastPoint)
				      multiple_entities = 1;
				  gaiaFreeGeomColl (geom);
			      }
			}
		      if (type == SQLITE_NULL || type == SQLITE_BLOB)
			  continue;
		      if (type == SQLITE_TEXT)
			{
			    char_value = sqlite3_column_text (stmt, i);
			    len = sqlite3_column_bytes (stmt, i);
			    sql_type[i] = SQLITE_TEXT;
			    if (len > max_length[i])
				max_length[i] = len;
			}
		      else if (type == SQLITE_FLOAT
			       && sql_type[i] != SQLITE_TEXT)
			  sql_type[i] = SQLITE_FLOAT;	/* promoting a numeric column to be DOUBLE */
		      else if (type == SQLITE_INTEGER &&
			       (sql_type[i] == SQLITE_NULL
				|| sql_type[i] == SQLITE_INTEGER))
			  sql_type[i] = SQLITE_INTEGER;	/* promoting a null column to be INTEGER */

		  }
	    }
	  else
	      goto sql_error;
      }
    if (!row1)
	goto empty_result_set;
    i = 0;
    offset = 0;
    dbf_list = gaiaAllocDbfList ();
    dbf_field = dbf_export_list->First;
    while (dbf_field)
      {
	  /* preparing the final DBF attribute list */
	  if (sql_type[i] == SQLITE_NULL)
	    {
		i++;
		dbf_field = dbf_field->Next;
		continue;
	    }
	  if (sql_type[i] == SQLITE_TEXT)
	    {
		gaiaAddDbfField (dbf_list, dbf_field->Name, 'C', offset,
				 max_length[i], 0);
		offset += max_length[i];
	    }
	  if (sql_type[i] == SQLITE_FLOAT)
	    {
		gaiaAddDbfField (dbf_list, dbf_field->Name, 'N', offset, 24, 6);
		offset += 24;
	    }
	  if (sql_type[i] == SQLITE_INTEGER)
	    {
		gaiaAddDbfField (dbf_list, dbf_field->Name, 'N', offset, 9, 0);
		offset += 9;
	    }
	  i++;
	  dbf_field = dbf_field->Next;
      }
    free (max_length);
    free (sql_type);
    gaiaFreeDbfList (dbf_export_list);
/* resetting SQLite query */
    fprintf (stderr, "\n%s;\n", sql);
    ret = sqlite3_reset (stmt);
    if (ret != SQLITE_OK)
	goto sql_error;
/* trying to open shapefile files */
    shp = gaiaAllocShapefile ();
    gaiaOpenShpWrite (shp, shp_path, shape, dbf_list);
    while (1)
      {
	  /* Pass II - scrolling the result set to dump data into shapefile */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		rows++;
		geom = NULL;
		dbf_write = gaiaCloneDbfEntity (dbf_list);
		for (i = 0; i < n_cols; i++)
		  {
		      if (strcasecmp
			  ((char *) column,
			   (char *) sqlite3_column_name (stmt, i)) == 0)
			{
			    /* this one is the internal BLOB encoded GEOMETRY to be exported */
			    blob_value = sqlite3_column_blob (stmt, i);
			    len = sqlite3_column_bytes (stmt, i);
			    dbf_write->Geometry =
				gaiaFromSpatiaLiteBlobWkb (blob_value, len);
			}
		      strcpy (dummy, sqlite3_column_name (stmt, i));
		      dbf_field = getDbfField (dbf_write, dummy);
		      if (!dbf_field)
			  continue;
		      switch (dbf_field->Type)
			{
			case 'N':
			    if (sqlite3_column_type (stmt, i) == SQLITE_INTEGER)
				gaiaSetIntValue (dbf_field,
						 sqlite3_column_int (stmt, i));
			    else if (sqlite3_column_type (stmt, i) ==
				     SQLITE_FLOAT)
				gaiaSetDoubleValue (dbf_field,
						    sqlite3_column_double (stmt,
									   i));
			    else
				gaiaSetNullValue (dbf_field);
			    break;
			case 'C':
			    if (sqlite3_column_type (stmt, i) == SQLITE_TEXT)
			      {
				  strcpy (dummy,
					  (char *) sqlite3_column_text (stmt,
									i));
				  gaiaSetStrValue (dbf_field, dummy);
			      }
			    else
				gaiaSetNullValue (dbf_field);
			    break;
			};
		  }
		if (!(dbf_write->Geometry))
		  {
		      gaiaFreeDbfList (dbf_write);
		      continue;
		  }
		if (!gaiaWriteShpEntity (shp, dbf_write))
		    fprintf (stderr, "shapefile write error\n");
		gaiaFreeDbfList (dbf_write);
	    }
	  else
	      goto sql_error;
      }
    sqlite3_finalize (stmt);
    gaiaFlushShpHeaders (shp);
    gaiaFreeShapefile (shp);
    fprintf (stderr, "\nExported %d rows into SHAPEFILE\n========\n", rows);
    return;
  sql_error:
/* some SQL error occurred */
    sqlite3_finalize (stmt);
    if (dbf_export_list);
    gaiaFreeDbfList (dbf_export_list);
    if (dbf_list);
    gaiaFreeDbfList (dbf_list);
    if (shp)
	gaiaFreeShapefile (shp);
    fprintf (stderr, "SELECT failed: %s", sqlite3_errmsg (sqlite));
    return;
  no_file:
/* shapefile can't be created/opened */
    if (dbf_export_list);
    gaiaFreeDbfList (dbf_export_list);
    if (dbf_list);
    gaiaFreeDbfList (dbf_list);
    if (shp)
	gaiaFreeShapefile (shp);
    fprintf (stderr, "ERROR: unable to open '%s' for writing", shp_path);
    return;
  empty_result_set:
/* the result set is empty - nothing to do */
    sqlite3_finalize (stmt);
    if (dbf_export_list);
    gaiaFreeDbfList (dbf_export_list);
    if (dbf_list);
    gaiaFreeDbfList (dbf_list);
    if (shp)
	gaiaFreeShapefile (shp);
    fprintf (stderr,
	     "The SQL SELECT returned an empty result set ... there is nothing to export ...");
}
