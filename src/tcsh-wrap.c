/*
 * osh - A shell for Oracle
 *
 * R. Carbone (rocco@tecsiel.it)
 * 2Q 2019
 *
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 */


/* Avoid warning: 'tcsh_xxx' defined but not used [-Wunused-function] */
#if defined(__GNUC__)
#pragma GCC diagnostic ignored   "-Wunused-function"
#else /* defined(__clang__) */
#pragma clang diagnostic ignored "-Wunused-function"
#endif


/* tcsh header file */
#include "sh.h"

extern int TermH;      /* number of real screen lines */
extern int TermV;      /* screen width                */


extern void unset (Char **, struct command *);
extern void docomplete (Char **, struct command *);

/* Project headers */
#include "osh.h"


/* Typedefs */
typedef int handler (int argc, char * argv []);


/* Handle completion */
static void handle_completion (char * argv [])
{
  Char ** v = blk2short (argv);

  if (! strcmp (argv [0], "set"))
    doset (v, NULL);
  else if (! strcmp (argv [0], "unset"))
    unset (v, NULL);
  else if (! strcmp (argv [0], "complete"))
    docomplete (v, NULL);
  else if (! strcmp (argv [0], "echo"))
    doecho (v, NULL);
}


/* Rows of the terminal */
unsigned osh_screen_rows (void)
{
  return TermH;
}


/* Cols of the terminal */
unsigned osh_screen_cols (void)
{
  return TermV;
}


/* Fill the [$name] variable to [value] */
void set_variable (char * name, char * value)
{
  char ** argv;

  argv = argsmore (NULL, "set");
  argv = argsmore (argv, name);
  argv = argsmore (argv, "=");
  argv = argsmore (argv, value);

  doset (blk2short (argv), NULL);

  argsclear (argv);
}


/* Unset the [$var] variable */
void unset_variable (char * var)
{
  char ** argv = argsmore (NULL, "unset");
  argv = argsmore (argv, var);

  handle_completion (argv);

  argsclear (argv);
}


/* Set the [$var] array */
static void set_tables (char * var)
{
  /* Lookup for the working connection */
  osh_connection_t * conn = get_current_connection ();
  char ** argv;
  char ** values;
  char ** n;

  if (! conn)
    return;

  /* Sort currently known unique table names */
  values = argssort (conn -> tabv);

  /* Set the $var variable */
  argv = argsmore (NULL, "set");
  argv = argsmore (argv, var);
  argv = argsmore (argv, "=");

  argv = argsmore (argv, "(");
  for (n = values; n && * n; n ++)
    argv = argsmore (argv, * n);
  argv = argsmore (argv, ")");

  handle_completion (argv);

  argsclear (argv);
}


/* Set complete in order to have the database table names completed on command [describe] */
void set_completions (void)
{
  extern void docomplete (Char **, struct command *);

  char ** argv  = argsmore (NULL, "describe");
  unsigned argc = argslen (argv);
  unsigned i;

  for (i = 0; i < argc; i ++)
    {
      char ** targv = argsmore (NULL, "complete");
      targv = argsmore (targv, argv [i]);
      targv = argsmore (targv, "p/\\*/$osh_tables/");

      /* add the completion directive to the list of completions */
      docomplete (blk2short (targv), NULL);

      argsclear (targv);
    }

  argsclear (argv);
}


/* How to call the [osh] extensions from tcsh */
static void tcsh_xxx (Char ** v, handler * func)
{
  Char ** vv = v;                                               /* interator in the 'v' array */
  char * name = "osh_tables";

  /* Insert command name as argv [0] */
  char ** argv = argsmore (NULL, short2str (* vv ++));

  while (* vv)
    argv = argsmore (argv, short2str (* vv ++));

  /* It's time to execute the function */
  if ((* func) (argslen (argv), argv))
    setcopy (STRstatus, Strsave (STR1), VAR_READWRITE);         /* set the $status variable */

  /* Set the [$osh_tables] variable for database names TAB-completion and globbing (only a subset of commands) */
  if (! strcmp (argv [0], "connect") || ! strcmp (argv [0], "tables") ||
      ! strcmp (argv [0], "describe") || ! strcmp (argv [0], "lsc") || ! strcmp (argv [0], "chc"))
    set_tables (name);
  else if (! strcmp (argv [0], "disconnect"))
    unset_variable (name);
}


/*
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Do not edit anything below, configure creates it.
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

/* Definitions for builtin extensions to the shell will be automatically inserted here by the configure script */
