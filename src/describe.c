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


/* Identifiers */
#define NAME         "describe"
#define BRIEF        "List the column definitions for a table"
#define SYNOPSIS     "describe [options]"
#define DESCRIPTION  "Display columns of a table"

/* Public variable */
osh_command_t cmd_describe = { NAME, BRIEF, SYNOPSIS, DESCRIPTION, osh_describe };


/* GNU short options */
enum
{
  /* Startup */
  OPT_HELP   = 'h',
  OPT_QUIET  = 'q',

  OPT_RELOAD = 'f'
};


/* GNU long options */
static struct option lopts [] =
{
  /* Startup */
  { "help",   no_argument, NULL, OPT_HELP   },
  { "quiet",  no_argument, NULL, OPT_QUIET  },

  { "reload", no_argument, NULL, OPT_RELOAD },

  { NULL,     0,           NULL, 0          }
};


/* Display the syntax */
static void usage (char * progname, struct option * options)
{
  /* longest option name */
  unsigned n = optmax (options);

  printf ("%s, %s\n", progname, NAME);
  printf ("Usage: %s [options]\n", progname);
  printf ("\n");

  printf ("Startup:\n");
  usage_item (options, n, OPT_HELP,  "show this help message and exit");
  usage_item (options, n, OPT_QUIET, "run quietly");
  printf ("\n");

  usage_item (options, n, OPT_RELOAD,  "force reload");
}


/* The [describe] command */
int osh_describe (int argc, char * argv [])
{
  char * progname = basename (argv [0]);
  char * sopts    = optlegitimate (lopts);

  /* Variables that are set according to the specified options */
  bool quiet      = false;
  bool reload     = false;

  osh_connection_t * conn;
  char ** names;
  int option;

  /* Lookup for the command in the static table of registered extensions */
  if (! cmd_by_name (progname))
    {
      printf ("%s: Command [%s] not found.\n", progname, progname);
      return 1;
    }

  /* Parse command line options */
  optind = 0;
  optarg = NULL;
  argv [0] = progname;
  while ((option = getopt_long (argc, argv, sopts, lopts, NULL)) != -1)
    {
      switch (option)
	{
	default: if (! quiet) printf ("Try '%s --help' for more information.\n", progname); return 1;

	  /* Startup */
	case OPT_HELP:  usage (progname, lopts); return 0;
	case OPT_QUIET: quiet = true;            break;

        case OPT_RELOAD:  reload = true;         break;
	}
    }

  /* Check for mandatory arguments */
  if (argc == optind)
    {
      printf ("%s: Too few arguments [%u].\n", progname, argc);
      return 1;
    }

  /* Check # of connections */
  if (! len_connections ())
    {
      if (! quiet)
	printf ("%s: no connection.\n", progname);
      return 0;
    }

  /* Describe tables over current connection */
  conn = get_current_connection ();

  /* Do the job */
  names = ocilib_user_table_names (conn, reload);
  if (! names)
    {
      if (! quiet)
	printf ("%s: Failed to get user tables names\n", progname);
      return 1;
    }

  if (! quiet)
    printf ("%s: found %u user tables\n", progname, arrlen (names));

  /* Iterate over all command line arguments in order to describe each */
  while (argc != optind)
    {
      char * name = argv [optind ++];
      osh_column_t ** columns;

      if (argscasemember (names, name) == -1)
	{
	  if (! quiet)
	    printf ("%s: Unknown user table name [%s].\n", progname, argv [argc - 1]);
	  continue;
	}

      /* Retrieve [columns] of the given table */
      if (! quiet)
	printf ("%s: retrieving cols from user table '%s' ... ", progname, name);

      /* Convert to [name] to uppercase */
      rupper (name);

      columns = ocilib_columns (conn, name);
      if (! columns)
	{
	  if (! quiet)
	    printf ("no item found.\n");
	}
      else
	{
	  /* Allocate a matrix to keep header and data */
	  unsigned rows = arrlen (columns) + 1;
	  unsigned cols = 3;
	  mx_t * mx     = mxalloc (rows, cols);
	  unsigned r    = 0;
	  unsigned c    = 0;
	  bool reverse  = false;
	  osh_column_t * col;

	  /* Print columns */
	  if (! quiet)
	    printf ("found #%u items!\n", arrlen (columns));

	  /* Table header */
	  mxcpy (mx, "#",     r, c ++);
	  mxcpy (mx, "Name",  r, c ++);
	  mxcpy (mx, "Value", r, c ++);

	  /* Insert the records in the matrix */
	  for (r = 1; r < rows; r ++)
	    for (c = 0; c < cols; c ++)
	      {
		col = reverse ? columns [rows - r - 1] : columns [r - 1];
		switch (c)
		  {
		  case 0: mxcpy (mx, utoa (r),     r, c); break;
		  case 1: mxcpy (mx, col -> name,  r, c); break;
		  case 2: mxcpy (mx, col -> value, r, c); break;
		  }
	      }

	  /* Print the data */
	  mxprint (mx);

	  /* Memory cleanup */
	  mxfree (mx);
	}

      /* separator */
      if (argc != optind)
	printf ("\n");
    }

  /* Bye bye! */
  return 0;
}
