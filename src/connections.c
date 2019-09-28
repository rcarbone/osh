/*
 * osh - A shell for Oracle
 *
 * R. Carbone (rocco@tecsiel.it)
 * 2Q 2019
 *
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 */


/* Project headers */
#include "osh.h"


/* The vector of connections */
static osh_connection_t ** all_connections = NULL;


/* Sort by TNS name */
static int sort_by_tnsname (const void * a, const void * b)
{
  return strcmp (osh_connection_name (* (osh_connection_t **) b), osh_connection_name (* (osh_connection_t **) a));
}


/* Reverse sort by TNS name */
static int rev_sort_by_tnsname (const void * a, const void * b)
{
  return sort_by_tnsname (b, a);
}


/* Sort by user */
static int sort_by_user (const void * a, const void * b)
{
  return strcmp (osh_connection_user (* (osh_connection_t **) b), osh_connection_user (* (osh_connection_t **) a));
}


/* Reverse sort by user */
static int rev_sort_by_user (const void * a, const void * b)
{
  return sort_by_user (b, a);
}


/* Sort by uptime */
static int sort_by_uptime (const void * a, const void * b)
{
  return osh_connection_uptime (* (osh_connection_t **) b) - osh_connection_uptime (* (osh_connection_t **) a);
}


/* Reverse sort by uptime */
static int rev_sort_by_uptime (const void * a, const void * b)
{
  return sort_by_uptime (b, a);
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


/* === Helpers === */

char * osh_connection_name (osh_connection_t * conn)
{
  return conn && conn -> handle ? (char *) OCI_GetDatabase (conn -> handle) : NULL;
}


char * osh_connection_user (osh_connection_t * conn)
{
  return conn && conn -> handle ? (char *) OCI_GetUserName (conn -> handle) : NULL;
}


char * osh_connection_pass (osh_connection_t * conn)
{
  return conn && conn -> handle ? (char *) OCI_GetPassword (conn -> handle) : NULL;
}


OCI_Connection * osh_connection_handle (osh_connection_t * conn)
{
  return conn ? conn -> handle : NULL;
}


rtime_t osh_connection_uptime (osh_connection_t * conn)
{
  return conn ? conn -> when : 0;
}


unsigned osh_connection_version (osh_connection_t * conn)
{
  return conn && conn -> handle ? OCI_GetVersionConnection (conn -> handle) : 0;
}


char * osh_connection_banner (osh_connection_t * conn)
{
  return conn && conn -> handle ? (char *) OCI_GetVersionServer (conn -> handle) : NULL;
}


char * osh_connection_error (osh_connection_t * conn)
{
  return conn ? conn -> error : NULL;
}


void osh_set_error (osh_connection_t * conn, char * fmt, ...)
{
  if (conn && fmt)
    {
      /* Format to a local buffer */
      char error [40960] = "";

      va_list va_ap;

      if (conn -> error)
	free (conn -> error);

      va_start (va_ap, fmt);
      vsprintf (error, fmt, va_ap);
      va_end (va_ap);

      conn -> error = strdup (error);
    }
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


/* Lookup for working connection in the table of connections */
osh_connection_t * get_current_connection (void)
{
  osh_connection_t ** conns = all_connections;
  if (conns)
    while (* conns)
      {
	if (osh_connection_get_working (* conns))
	  return * conns;
	else
	  conns ++;
      }
  return NULL;
}


/* Lookup for the n-th item in the table of connections */
osh_connection_t * get_connection (unsigned n)
{
  return all_connections && n > 0 && n < len_connections () + 1 ? all_connections [n - 1] : NULL;
}


/* Set [conn] as the current connection */
void set_current_connection (osh_connection_t * conn)
{
  osh_connection_t * c = get_current_connection ();
  if (c)
    osh_connection_set_working (c, false);
  osh_connection_set_working (conn, true);
}


/* Reset the current connection after [conn] has been closed */
void reset_current_connection (osh_connection_t * conn)
{
  /* Most recent becomes the current */
  osh_connection_t ** all = all_connections;
  osh_connection_t * c    = NULL;
  rtime_t newer           = 0;

  if (all)
    while (* all)
      {
	if (* all != conn && osh_connection_uptime (* all) > newer)
	  {
	    newer = osh_connection_uptime (* all);
	    c = * all;
	  }
	all ++;
      }

  if (c)
    osh_connection_set_working (c, true);
}


bool osh_connection_get_working (osh_connection_t * conn)
{
  return conn ? conn -> working : false;
}


void osh_connection_set_working (osh_connection_t * conn, bool status)
{
  if (conn)
    conn -> working = status;
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


unsigned len_connections (void)
{
  return arrlen (all_connections);
}


osh_connection_t ** get_connections (void)
{
  return all_connections;
}


void add_connection (osh_connection_t * conn)
{
  all_connections = arrmore (all_connections, conn, osh_connection_t);
}


void del_connection (osh_connection_t * conn)
{
  all_connections = arrless (all_connections, conn, osh_connection_t, osh_connection_done);
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


osh_connection_t ** conn_sort_by_tnsname (void)
{
  return all_connections = arrsort (all_connections, sort_by_tnsname, osh_connection_t);
}


osh_connection_t ** conn_sort_by_user (void)
{
  return all_connections = arrsort (all_connections, sort_by_user, osh_connection_t);
}


osh_connection_t ** conn_sort_by_uptime (void)
{
  return all_connections = arrsort (all_connections, sort_by_uptime, osh_connection_t);
}


osh_connection_t ** conn_rev_sort_by_tnsname (void)
{
  return all_connections = arrsort (all_connections, rev_sort_by_tnsname, osh_connection_t);
}


osh_connection_t ** conn_rev_sort_by_user (void)
{
  return all_connections = arrsort (all_connections, rev_sort_by_user, osh_connection_t);
}


osh_connection_t ** conn_rev_sort_by_uptime (void)
{
  return all_connections = arrsort (all_connections, rev_sort_by_uptime, osh_connection_t);
}
