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


/* === Allocation === */
osh_connection_t * osh_connection_alloc (OCI_Connection * handle)
{
  osh_connection_t * conn = calloc (1, sizeof (* conn));

  conn -> handle    = handle;
  conn -> when      = nswall ();
  conn -> working   = false;
  conn -> error     = NULL;

  /* Cache for User Tables */
  conn -> updated   = 0;
  conn -> tabv      = NULL;

  return conn;
}


/* === Deallocation === */
osh_connection_t * osh_connection_free (osh_connection_t * conn)
{
  if (! conn)
    return NULL;

  if (conn -> scroll)
    OCI_StatementFree (OCI_ResultsetGetStatement (conn -> scroll));

  argsclear (conn -> tabv);
  safefree (conn -> error);

  if (conn -> handle)
    ocilib_disconnect (conn -> handle);

  free (conn);
  return NULL;
}


/* Another deallocation funtion to be used in arrless() */
void osh_connection_done (void * conn)
{
  osh_connection_free (conn);
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


/* === Allocation === */
osh_column_t * osh_column_alloc (char * name, unsigned type, char * value)
{
  osh_column_t * col = calloc (1, sizeof (* col));

  col -> name  = safedup (name);
  col -> type  = type;
  col -> value = safedup (value);
  return col;
}


/* === Deallocation === */
osh_column_t * osh_column_free (osh_column_t * col)
{
  if (! col)
    return NULL;

  safefree (col -> name);
  safefree (col -> value);
  free (col);
  return NULL;
}


/* === Another Deallocation === */
void osh_column_done (void * col)
{
  osh_column_free (col);
}
