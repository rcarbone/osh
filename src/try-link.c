/*
 * osh - A shell for Oracle
 *
 * R. Carbone (rocco@tecsiel.it)
 * 2Q 2019
 *
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 */


/*
 * Does nothing, but tries to link static library.
 *
 * The functions have been written only to test
 * if a binary program can be generated at compile time.
 *
 * They will never be executed, so there is no need to check for failures.
 */


/* Project headers */
#include "osh.h"


/* Inline sources */
#include "missing.c"


/* Helpers */
static void osh_helpers_all (int argc, char * argv [])
{
  osh_init (NULL, 0);
  osh_prompt (NULL);

  osh_help (argc, argv);
  osh_about (argc, argv);
  osh_version (argc, argv);
  osh_license (argc, argv);

  osh_when (argc, argv);
}


static void osh_connections_all (int argc, char * argv [])
{
  osh_connect (argc, argv);
  osh_disconnect (argc, argv);
  osh_lsc (argc, argv);
  osh_chc (argc, argv);
}


static void osh_tables_all (int argc, char * argv [])
{
  osh_tables (argc, argv);
  osh_describe (argc, argv);
}


static void osh_viewers_all (int argc, char * argv [])
{
  osh_select (argc, argv);
}


static void osh_apps_all (int argc, char * argv [])
{
  osh_oping (argc, argv);
}


int main (int argc, char * argv [])
{
  printf ("This program does nothing, but it only tests if link works at compile time. Bye bye!\n");

  if (argc == 0)
    {
      /* Never executed, so no check is done about possible failures */
      osh_helpers_all (argc, argv);
      osh_connections_all (argc, argv);
      osh_tables_all (argc, argv);
      osh_viewers_all (argc, argv);
      osh_apps_all (argc, argv);
    }

  /* Bye bye! */
  return 0;
}
