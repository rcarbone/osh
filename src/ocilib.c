/*
 * osh - A shell for Oracle
 *
 * R. Carbone (rocco@tecsiel.it)
 * 2Q 2019
 *
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 */


/* Project headers */
#define _GNU_SOURCE
#include "osh.h"


/* Constants */
#define SQL_LEN         10240

/* Reserved keys */
#define USER_TABLES    "user_tables"
#define USER_COLUMNS   "user_tab_columns"


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* A generic error handler routine */
static void keeperror (OCI_Error * e)
{
  /* A static buffer where to keep Oracle errors */
  static char error [4096] = "";

  sprintf (error, "%s", OCI_ErrorGetString (e));
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


static char ** get_cached_names (osh_connection_t * conn, bool reload)
{
  if (! conn)
    return NULL;

  if (! reload && conn -> tabv)
    return conn -> tabv;

  conn -> updated = nswall ();
  conn -> tabv    = ocilib_table_names (conn, USER_TABLES);

  return conn -> tabv;
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


char * ocilib_date (OCI_Resultset * rs, unsigned c)
{
#define DATEFMT "YYYY-MM-DD HH24:MI:SS"
  static char buf [128] = { 0x00 };

  char * dt = (char *) OCI_GetDate (rs, c);   /* Oracle DATE to keep date and time */
  if (dt && * dt)
    {
      OCI_Column * col  = OCI_GetColumn (rs, c);
      const char * name = OCI_ColumnGetName (col);
      OCI_Date * date   = OCI_GetDate2 (rs, OTEXT (name));
      OCI_DateToText (date, OTEXT (DATEFMT), sizeof (buf), buf);
      OCI_DateFree (date);
    }
  else
    * buf = 0x00;

  return buf;
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


/* Put a user table into a tree */
GNode * osh_tabletotree (osh_connection_t * conn, char * table, bool expand)
{
  char ** cols = ocilib_column_names (conn, table);
  char ** c    = cols;                        /* iterator over cols */
  GNode * root;

  if (! cols)
    return NULL;

  /* The root tree name */
  root = g_node_new (strdup (table));

  /* Iterate over all Columns */
  if (expand)
    while (c && * c)
      g_node_append_data (root, strdup ((* c ++)));

  /* Cleanup */
  argsclear (cols);

  return root;
}


/* Build a tree */
GNode * osh_mktree (osh_connection_t * conn, bool reload, bool expand)
{
  /* Get and cache names */
  char ** names = get_cached_names (conn, reload);
  GNode * root;

  if (! names)
    return NULL;

  /* The root tree name */
  root = g_node_new (strdup (osh_connection_name (conn)));

  /* Iterate over all tables */
  while (names && * names)
    {
      GNode * next = osh_tabletotree (conn, * names ++, expand);
      if (next)
	g_node_append (root, next);
    }

  return root;
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

unsigned rs_size (OCI_Resultset * rs)
{
  unsigned curr;
  unsigned size;

  /* Check for a scrollable ResultSet */
  if (! rs || OCI_GetFetchMode (OCI_ResultsetGetStatement (rs)) != OCI_SFM_SCROLLABLE)
    return 0;

  /* Get current offset */
  curr = OCI_GetCurrentRow (rs);

  /* Move to last item of the ResultSet in order to evaluate the size */
  OCI_FetchLast (rs);

  size = OCI_GetCurrentRow (rs);

  /* Move to old offset */
  OCI_FetchSeek (rs, OCI_SFD_ABSOLUTE, curr);

  return size;
}


/* Query the Database and return a ResultSet bound to a new Statement */
OCI_Resultset * ocilib_resultset (osh_connection_t * conn, char * query)
{
  OCI_Statement * st;

  /* Basic checks */
  if (! conn || ! conn -> handle || ! OCI_IsConnected (conn -> handle) || ! query)
    return NULL;

  /* Create a SQL statement */
  st = OCI_StatementCreate (conn -> handle);
  if (! st)
    {
      osh_set_error (conn, "%s:%d StatementCreate() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));
      return NULL;
    }

  /* Prepare and Execute a SQL statement */
  if (! OCI_ExecuteStmt (st, query))
    {
      osh_set_error (conn, "%s:%d ExecuteStmt() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return NULL;
    }

  return OCI_GetResultset (st);
}


/* Query the Database and return a scrollable ResultSet bound to a new Statement */
OCI_Resultset * ocilib_scrollable_resultset (osh_connection_t * conn, char * query)
{
  OCI_Statement * st;

  /* Basic checks */
  if (! conn || ! conn -> handle || ! OCI_IsConnected (conn -> handle) || ! query)
    return NULL;

  /* Create a SQL statement */
  st = OCI_StatementCreate (conn -> handle);
  if (! st)
    {
      osh_set_error (conn, "%s:%d StatementCreate() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));
      return NULL;
    }

  if (! OCI_SetFetchMode (st, OCI_SFM_SCROLLABLE))
    {
      osh_set_error (conn, "%s:%d SetFetchMode() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return NULL;
    }

  /* Prepare and Execute a SQL statement */
  if (! OCI_ExecuteStmt (st, query))
    {
      osh_set_error (conn, "%s:%d ExecuteStmt() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return NULL;
    }

  return OCI_GetResultset (st);
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


/* Query the Database and return # of table names for a given [table] */
unsigned ocilib_table_count (osh_connection_t * conn, char * table)
{
  unsigned n = 0;
  char query [SQL_LEN];
  OCI_Statement * st;
  OCI_Resultset * rs;

  /* Basic checks */
  if (! conn || ! conn -> handle || ! OCI_IsConnected (conn -> handle) || ! table)
    return 0;

  /* Create a SQL statement */
  st = OCI_StatementCreate (conn -> handle);
  if (! st)
    {
      osh_set_error (conn, "%s:%d StatementCreate() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));
      return 0;
    }

  /* Build the query */
  sprintf (query, "SELECT COUNT(*) FROM %s", table);

  /* Prepare and Execute a SQL statement */
  if (! OCI_ExecuteStmt (st, query))
    {
      osh_set_error (conn, "%s:%d ExecuteStmt() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return 0;
    }

  rs = OCI_GetResultset (st);
  if (! rs)
    {
      osh_set_error (conn, "%s:%d GetResultset() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return 0;
    }

  /* Retrieve the value from the Database */
  if (! OCI_FetchNext (rs))
    {
      osh_set_error (conn, "%s:%d FetchNext() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return 0;
    }

  /* Retrieve #count */
  n = OCI_GetInt (rs, 1);

  /* Free the statement and all resources associated to it */
  OCI_StatementFree (st);

  return n;
}


/* Query the Database and return all table names from a given [table] */
char ** ocilib_table_names (osh_connection_t * conn, char * table)
{
  char ** argv = NULL;
  char query [SQL_LEN];
  OCI_Statement * st;
  OCI_Resultset * rs;

  /* Basic checks */
  if (! conn || ! conn -> handle || ! OCI_IsConnected (conn -> handle) || ! table)
    return NULL;

  /* Create a SQL statement */
  st = OCI_StatementCreate (conn -> handle);
  if (! st)
    {
      osh_set_error (conn, "%s:%d StatementCreate() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));
      return NULL;
    }

  /* Build the query */
  sprintf (query, "SELECT table_name FROM %s", table);

  /* Prepare and Execute a SQL statement */
  if (! OCI_ExecuteStmt (st, query))
    {
      osh_set_error (conn, "%s:%d ExecuteStmt() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return NULL;
    }

  rs = OCI_GetResultset (st);
  if (! rs)
    {
      osh_set_error (conn, "%s:%d GetResultset() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return NULL;
    }

  /* Loop in the given result set to get values from the Database,
   * allocate a new variable and add it to the vector of table names */
  while (OCI_FetchNext (rs))
    argv = argsmore (argv, safedup ((char *) OCI_GetString (rs, 1)));

  /* Free the statement and all resources associated to it */
  OCI_StatementFree (st);

  return argv;
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


/* Query the Database and return # of columns names for a given [table] */
unsigned ocilib_column_count (osh_connection_t * conn, char * table)
{
  unsigned n = 0;
  char query [SQL_LEN];
  OCI_Statement * st;
  OCI_Resultset * rs;

  /* Basic checks */
  if (! conn || ! conn -> handle || ! OCI_IsConnected (conn -> handle) || ! table)
    return 0;

  /* Create a SQL statement */
  st = OCI_StatementCreate (conn -> handle);
  if (! st)
    {
      osh_set_error (conn, "%s:%d StatementCreate() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));
      return 0;
    }

  /* Build the query */
  sprintf (query, "SELECT COUNT(*) FROM %s where table_name = '%s'", USER_COLUMNS, table);

  /* Prepare and Execute a SQL statement */
  if (! OCI_ExecuteStmt (st, query))
    {
      osh_set_error (conn, "%s:%d ExecuteStmt() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return 0;
    }

  rs = OCI_GetResultset (st);
  if (! rs)
    {
      osh_set_error (conn, "%s:%d GetResultset() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return 0;
    }

  /* Retrieve the value from the Database */
  if (! OCI_FetchNext (rs))
    {
      osh_set_error (conn, "%s:%d FetchNext() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return 0;
    }

  /* Retrieve #count */
  n = OCI_GetInt (rs, 1);

  /* Free the statement and all resources associated to it */
  OCI_StatementFree (st);

  return n;
}


/* Query the Database and return all columns names from a given [table] */
char ** ocilib_column_names (osh_connection_t * conn, char * table)
{
  char ** argv = NULL;
  char query [SQL_LEN];
  OCI_Statement * st;
  OCI_Resultset * rs;

  /* Basic checks */
  if (! conn || ! conn -> handle || ! OCI_IsConnected (conn -> handle) || ! table)
    return NULL;

  /* Create a SQL statement */
  st = OCI_StatementCreate (conn -> handle);
  if (! st)
    {
      osh_set_error (conn, "%s:%d StatementCreate() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));
      return NULL;
    }

  /* Build the query */
  sprintf (query, "SELECT column_name FROM %s where table_name = '%s'", USER_COLUMNS, table);

  /* Prepare and Execute a SQL statement */
  if (! OCI_ExecuteStmt (st, query))
    {
      osh_set_error (conn, "%s:%d ExecuteStmt() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return NULL;
    }

  rs = OCI_GetResultset (st);
  if (! rs)
    {
      osh_set_error (conn, "%s:%d GetResultset() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return NULL;
    }

  /* Loop in the given result set to get values from the Database,
   * allocate a new variable and add it to the vector of column names */
  while (OCI_FetchNext (rs))
    argv = argsmore (argv, safedup ((char *) OCI_GetString (rs, 1)));

  /* Free the statement and all resources associated to it */
  OCI_StatementFree (st);

  return argv;
}


/* Query the Database and return all columns of a given user table */
osh_column_t ** ocilib_columns (osh_connection_t * conn, char * table)
{
  osh_column_t ** argv = NULL;
  char query [SQL_LEN];
  OCI_Statement * st;
  OCI_Resultset * rs;

  /* Basic checks */
  if (! conn || ! conn -> handle || ! OCI_IsConnected (conn -> handle) || ! table)
    return NULL;

  /* Create a SQL statement */
  st = OCI_StatementCreate (conn -> handle);
  if (! st)
    {
      osh_set_error (conn, "%s:%d StatementCreate() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));
      return NULL;
    }

  /* Build the query */
  sprintf (query, "SELECT column_name, data_type FROM %s where table_name = '%s'", USER_COLUMNS, table);

  /* Prepare and Execute a SQL statement */
  if (! OCI_ExecuteStmt (st, query))
    {
      osh_set_error (conn, "%s:%d ExecuteStmt() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return NULL;
    }

  rs = OCI_GetResultset (st);
  if (! rs)
    {
      osh_set_error (conn, "%s:%d GetResultset() - [%s]", __FILE__, __LINE__, OCI_ErrorGetString (OCI_GetLastError ()));

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return NULL;
    }

  /* Loop in the given result set to get values from the Database,
   * allocate a new variable and add it to the table of results */
  while (OCI_FetchNext (rs))
    argv = arrmore (argv, osh_column_alloc ((char *) OCI_GetString (rs, 1), 3, (char *) OCI_GetString (rs, 2)), osh_column_t);

  /* Free the statement and all resources associated to it */
  OCI_StatementFree (st);

  return argv;
}


/* Query the Database and return of #count for select statements */
unsigned ocilib_count (osh_connection_t * conn, char * text)
{
  unsigned n = 0;
  char query [SQL_LEN];
  char * s;
  OCI_Statement * st;
  OCI_Resultset * rs;

  /* Basic checks */
  if (! conn || ! conn -> handle || ! OCI_IsConnected (conn -> handle) || ! text)
    return 0;

  /* Check for a select statement in [text] */
  s = strcasestr (text, " from ");
  if (! s)
    return 0;

  /* Build the query */
  sprintf (query, "SELECT COUNT(*) %s", s);

  /* Create and execute a SQL statement and retrieve the resultset from the statement */
  st = OCI_StatementCreate (conn -> handle);
  OCI_ExecuteStmt (st, query);
  rs = OCI_GetResultset (st);
  if (! rs)
    {
      osh_set_error (conn, "%s:%d count [%s] - [%s]", __FILE__, __LINE__, query);

      /* Free the statement and all resources associated to it */
      OCI_StatementFree (st);
      return 0;
    }

  /* Fetch the next row of the resultset */
  OCI_FetchNext (rs);
  n = OCI_GetInt (rs, 1);

  /* Free the statement and all resources associated to it */
  OCI_StatementFree (st);

  return n;
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


/* Initialize the ocilib library */
bool ocilib_initialize (void)
{
  return ! osh_run . initialized ? (osh_run . initialized = OCI_Initialize (NULL, NULL, OCI_ENV_DEFAULT)) : false;
}


/* Clean up all resources allocated by the ocilib library */
void ocilib_cleanup (void)
{
  if (osh_run . initialized)
    OCI_Cleanup ();
  osh_run . initialized = false;
}


/* Attempt to connect to a Database server */
osh_connection_t * ocilib_connect (char * name, char * user, char * pass)
{
  OCI_Connection * handle;

  /* Set the global error handler */
  OCI_SetErrorHandler (keeperror);

  /* Create a physical connection to an Oracle Database server */
  if (! (handle = OCI_ConnectionCreate (name, user, pass, OCI_SESSION_DEFAULT)))
    return NULL;

  /* Disable the auto commit mode that allows commit changes after every executed SQL statement */
  if (! OCI_SetAutoCommit (handle, FALSE))
    {
      /* Close the established connection */
      OCI_ConnectionFree (handle);
      return NULL;
    }

  return osh_connection_alloc (handle);
}


/* Close the physical connection to a Database server */
OCI_Connection * ocilib_disconnect (OCI_Connection * handle)
{
  if (handle)
    {
      /* Don't commit current pending changes over the connection */
      if (OCI_IsConnected (handle))
	OCI_Rollback (handle);

      /* Close the physical connection to an Oracle Database server */
      OCI_ConnectionFree (handle);
    }
  return NULL;
}


/* Check the status of the connection */
bool ocilib_status (OCI_Connection * handle)
{
  return handle ? OCI_IsConnected (handle) : false;
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


/* Query the Database and return # of user tables */
unsigned ocilib_user_table_count (osh_connection_t * conn, bool reload)
{
  return arrlen (get_cached_names (conn, reload));
}


/* Query the Database and return all user table names */
char ** ocilib_user_table_names (osh_connection_t * conn, bool reload)
{
  return get_cached_names (conn, reload);
}


/* Extract in a matrix a subset of ResultSet [rs] of [size] items starting at [offset] */
mx_t * rstomx (unsigned rssize, OCI_Resultset * rs, unsigned size, unsigned offset)
{
  mx_t * mx;
  unsigned rows;
  unsigned cols;
  unsigned r;
  unsigned c;

  /* Check for a scrollable ResultSet */
  if (! rs || OCI_GetFetchMode (OCI_ResultsetGetStatement (rs)) != OCI_SFM_SCROLLABLE)
    return NULL;

  rows = rssize + 1;                   /* +1 to include header */
  cols = OCI_GetColumnCount (rs) + 1;  /* +1 to include serial */

  if (offset == 0)
    offset = 1;

  if (offset > rows)
    return NULL;

  /* Check for upper limit */
  if (size)
    rows = RMIN (size + 1, rows);

  /* Allocate a matrix */
  mx = mxalloc (rows, cols);

  /* Retrieve and keep selected columns to be shown in Table header */
  for (c = 0; c < cols; c ++)
    mxcpy (mx, ! c ? "#" : (char *) OCI_ColumnGetName (OCI_GetColumn (rs, c)), 0, c);     /* at row 0 */

  /* Loop in the given result set to get values from the Database add it to the table of results */
  r = 1;
  while (OCI_FetchSeek (rs, OCI_SFD_ABSOLUTE, offset) && r < rows)
    {
      mxcpy (mx, utoa (offset ++), r, c = 0);      /* #serial at column 0 */

      /* Insert the records in the matrix */
      for (c = 1; c < cols; c ++)
	{
	  OCI_Column * col = OCI_GetColumn (rs, c);
	  char * value = NULL;
	  switch (OCI_ColumnGetType (col))
	    {
	    case OCI_UNKNOWN:        value = "Unknown";                      break;
	    case OCI_CDT_NUMERIC:    value = utoa (OCI_GetInt (rs, c));      break;
	    case OCI_CDT_DATETIME:   value = ocilib_date (rs, c);            break;
	    case OCI_CDT_TEXT:       value = (char *) OCI_GetString (rs, c); break;
	    case OCI_CDT_LONG:       value = "long (unsupported)";           break;
	    case OCI_CDT_CURSOR:     value = "cursor (unsupported)";         break;
	    case OCI_CDT_LOB:        value = "lob (unsupported)";            break;
	    case OCI_CDT_FILE:       value = "file (unsupported)";           break;
	    case OCI_CDT_TIMESTAMP:  value = "timestamp (unsupported)";      break;
	    case OCI_CDT_INTERVAL:   value = "interval (unsupported)";       break;
	    case OCI_CDT_RAW:        value = "raw (unsupported)";            break;
	    case OCI_CDT_OBJECT:     value = "object (unsupported)";         break;
	    case OCI_CDT_COLLECTION: value = "collection (unsupported)";     break;
	    case OCI_CDT_REF:        value = "ref (unsupported)";            break;
	    case OCI_CDT_BOOLEAN:    value = "boolean (unsupported)";        break;
	    default:                 value = "default (unsupported)";        break;
	    }

	  /* Insert the value into the matrix at [r] [c] */
	  if (value)
	    mxcpy (mx, value, r, c);
	}
      r ++;    /* next row */
    }

  return mx;
}


/* Extract in a vector a subset of ResultSet [rs] of [size] items starting at [offset] */
char ** rstoargv (unsigned rssize, OCI_Resultset * rs, unsigned size, unsigned offset)
{
  char ** argv = NULL;

  /* First destination container is a matrix for better line rendering */
  mx_t * mx = rstomx (rssize, rs, size, offset);
  if (mx)
    {
      unsigned r;
      unsigned c;

      for (r = 0; r < mxrows (mx); r ++)
	{
	  char line [MAXLINE];
	  strcpy (line, "|");
	  for (c = 0; c < mxcols (mx); c ++)
	    {
	      unsigned n = mxcolmaxlen (mx, c);
	      char buf [MAXLINE];
	      sprintf (buf, " %-*.*s |", n, n, mxdata (mx) [r * mxcols (mx) + c]);
	      strcat (line, buf);
	    }
	  argv = argsmore (argv, strdup (line));
	}

      /* Memory cleanup */
      mxfree (mx);
    }

  return argv;
}


/* Query Database and get records in a tree */
GNode * rstotree (unsigned rssize, OCI_Resultset * rs, unsigned n)
{
  GNode * root;
  unsigned cols;
  unsigned r;
  unsigned c;

  /* Check for a scrollable ResultSet */
  if (! rssize || ! rs || OCI_GetFetchMode (OCI_ResultsetGetStatement (rs)) != OCI_SFM_SCROLLABLE)
    return NULL;

  /* The root tree name with the query performed */
  root = g_node_new (strdup ((char *) OCI_GetSql (OCI_ResultsetGetStatement (rs))));

  /* Loop in the given result set to get values from the Database add it to the table of results */
  r = 1;
  cols = OCI_GetColumnCount (rs);
  while (OCI_FetchSeek (rs, OCI_SFD_ABSOLUTE, r) && (! n || r <= n))
    {
      /* The child tree label */
      GNode * child;
      char label [1024];
      sprintf (label, "#%u", r ++);
      child = g_node_new (strdup (label));

      /* Insert the records in the tree */
      for (c = 1; c <= cols; c ++)
	{
	  OCI_Column * col = OCI_GetColumn (rs, c);
	  char * value = NULL;
	  char buf [1024];

	  switch (OCI_ColumnGetType (col))
	    {
	    case OCI_UNKNOWN:        value = "Unknown";                      break;
	    case OCI_CDT_NUMERIC:    value = utoa (OCI_GetInt (rs, c));      break;
	    case OCI_CDT_DATETIME:   value = ocilib_date (rs, c);            break;
	    case OCI_CDT_TEXT:       value = (char *) OCI_GetString (rs, c); break;
	    case OCI_CDT_LONG:       value = "long (unsupported)";           break;
	    case OCI_CDT_CURSOR:     value = "cursor (unsupported)";         break;
	    case OCI_CDT_LOB:        value = "lob (unsupported)";            break;
	    case OCI_CDT_FILE:       value = "file (unsupported)";           break;
	    case OCI_CDT_TIMESTAMP:  value = "timestamp (unsupported)";      break;
	    case OCI_CDT_INTERVAL:   value = "interval (unsupported)";       break;
	    case OCI_CDT_RAW:        value = "raw (unsupported)";            break;
	    case OCI_CDT_OBJECT:     value = "object (unsupported)";         break;
	    case OCI_CDT_COLLECTION: value = "collection (unsupported)";     break;
	    case OCI_CDT_REF:        value = "ref (unsupported)";            break;
	    case OCI_CDT_BOOLEAN:    value = "boolean (unsupported)";        break;
	    default:                 value = "default (unsupported)";        break;
	    }

	  /* Append the value into the tree */
	  sprintf (buf, "%s - %s", OCI_ColumnGetName (OCI_GetColumn (rs, c)), value);
	  g_node_append_data (child, strdup (buf));
	}
      g_node_append (root, child);
    }

  return root;
}


void print_record (OCI_Resultset * rs)
{
  /* Retrieve # of columns selected */
  unsigned cols = OCI_GetColumnCount (rs);
  unsigned i;
  for (i = 0; i < cols; i ++)
    {
      OCI_Column * col = OCI_GetColumn (rs, i + 1);
      switch (OCI_ColumnGetType (col))
	{
	case OCI_UNKNOWN:        printf ("Boh");                           break;
	case OCI_CDT_NUMERIC:    printf ("%d", OCI_GetInt (rs, i + 1));    break;
	case OCI_CDT_DATETIME:   printf ("DateTime");                      break;
	case OCI_CDT_TEXT:       printf ("%s", OCI_GetString (rs, i + 1)); break;
	case OCI_CDT_LONG:       printf ("Long");                          break;
	case OCI_CDT_CURSOR:     printf ("Cursor");                        break;
	case OCI_CDT_LOB:        printf ("Lob");                           break;
	case OCI_CDT_FILE:       printf ("File");                          break;
	case OCI_CDT_TIMESTAMP:  printf ("TimeStamp");                     break;
	case OCI_CDT_INTERVAL:   printf ("Interval");                      break;
	case OCI_CDT_RAW:        printf ("Raw");                           break;
	case OCI_CDT_OBJECT:     printf ("Object");                        break;
	case OCI_CDT_COLLECTION: printf ("Collection");                    break;
	case OCI_CDT_REF:        printf ("Ref");                           break;
	case OCI_CDT_BOOLEAN:    printf ("Boolean");	                   break;
	}
      printf (" | ");
      if (i == cols - 1)
	printf ("\n");
    }
}
