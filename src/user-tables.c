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
#define NAME         "tables"
#define BRIEF        "List user tables"
#define SYNOPSIS     "tables [options]"
#define DESCRIPTION  "List all the tables in the database of the current connection"

/* Public variable */
osh_command_t cmd_tables = { NAME, BRIEF, SYNOPSIS, DESCRIPTION, osh_tables };


/* GNU short options */
enum
{
  /* Startup */
  OPT_HELP    = 'h',
  OPT_QUIET   = 'q',

  /* Output formats */
  OPT_LIST    = 'l',
  OPT_XLIST   = 'x',
  OPT_TABLE   = 't',
  OPT_TREE    = 'T',
  OPT_COLS    = 'c',

  /* Sort */
  OPT_UNSORT  = 'u',
  OPT_REVERSE = 'r',

  OPT_RELOAD  = 'f'
};


/* GNU long options */
static struct option lopts [] =
{
  /* Startup */
  { "help",    no_argument, NULL, OPT_HELP    },
  { "quiet",   no_argument, NULL, OPT_QUIET   },

  /* Output format */
  { "list",    no_argument, NULL, OPT_LIST    },
  { "cols",    no_argument, NULL, OPT_XLIST   },
  { "table",   no_argument, NULL, OPT_TABLE   },
  { "tree",    no_argument, NULL, OPT_TREE    },
  { "columns", no_argument, NULL, OPT_COLS    },

  /* Sort */
  { "unsort",  no_argument, NULL, OPT_UNSORT  },
  { "reverse", no_argument, NULL, OPT_REVERSE },

  { "reload",  no_argument, NULL, OPT_RELOAD  },

  { NULL,      0,           NULL, 0           }
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
  usage_item (options, n, OPT_HELP,    "show this help message and exit");
  usage_item (options, n, OPT_QUIET,   "run quietly");
  printf ("\n");

  printf ("Output formats:\n");
  usage_item (options, n, OPT_LIST,    "list names (default)");
  usage_item (options, n, OPT_XLIST,   "list names by columns");
  usage_item (options, n, OPT_TABLE,   "list names in a table with # of records per table");
  usage_item (options, n, OPT_TREE,    "list names in a tree");
  usage_item (options, n, OPT_COLS,    "show also column names in tree");
  printf ("\n");

  printf ("Sorting options are:\n");
  usage_item (options, n, OPT_UNSORT,  "do not sort (default)");
  usage_item (options, n, OPT_REVERSE, "reverse the result of sorting");
  printf ("\n");

  usage_item (options, n, OPT_RELOAD,  "force reload of # of records");
}


/* Print user tables as a matrix */
static void print_user_tables_mx (char * argv [], bool reverse, osh_connection_t * conn)
{
  /* Allocate a matrix to keep header and data */
  unsigned rows = arrlen (argv) + 1;
  unsigned cols = 3;
  mx_t * mx     = mxalloc (rows, cols);
  unsigned r    = 0;
  unsigned c    = 0;

  /* Table header */
  mxcpy (mx, "#",     r, c ++);
  mxcpy (mx, "Name",  r, c ++);
  mxcpy (mx, "Count", r, c ++);

  /* Insert the records in the matrix */
  for (r = 1; r < rows; r ++)
    for (c = 0; c < cols; c ++)
      {
	char * name = reverse ? argv [rows - r - 1] : argv [r - 1];
	switch (c)
	  {
	  case 0: mxcpy (mx, utoa (r), r, c); break;
	  case 1: mxcpy (mx, name,     r, c); break;
	  case 2: mxcpy (mx, utoa (ocilib_table_count (conn, name)), r, c); break;
	  }
      }

  /* Print the data */
  mxprint (mx);

  /* Memory cleanup */
  mxfree (mx);
}


/* Print user tables as a matrix */
static void print_user_tables_tree (char * argv [], bool reverse, osh_connection_t * conn, bool cols)
{
  /* Build a tree representing the database */
  GNode * root = osh_mktree (conn, false, cols);      /* cache was already reloaded with previous ocilib_user_table_names() */

  /* Pretty print the tree upright */
  if (root)
    {
      g_node_hprint_rosetta (root);

      /* Memory cleanup */
      g_node_no_more (root);
      g_node_destroy (root);
    }
}


/* Print user tables in one of the supported output format */
static void print_user_tables (unsigned format, char * argv [], unsigned width, bool reverse, osh_connection_t * conn, bool cols)
{
  switch (format)
    {
    case OPT_LIST:  args_print_rows (argv, width);                      break;
    case OPT_XLIST: args_print_cols (argv, width);                      break;
    case OPT_TABLE: print_user_tables_mx (argv, reverse, conn);         break;
    case OPT_TREE:  print_user_tables_tree (argv, reverse, conn, cols); break;
    }
}


/* The [tables] command */
int osh_tables (int argc, char * argv [])
{
  char * progname = basename (argv [0]);
  char * sopts    = optlegitimate (lopts);

  /* Variables that are set according to the specified options */
  bool quiet      = false;
  unsigned format = OPT_LIST;
  unsigned width  = osh_screen_cols ();
  bool reverse    = false;
  bool reload     = false;
  bool cols       = false;

  osh_connection_t * conn;
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
	case OPT_HELP:    usage (progname, lopts); return 0;
	case OPT_QUIET:   quiet   = true;          break;

	  /* Output formats */
	case OPT_LIST:    format  = OPT_LIST;      break;
	case OPT_XLIST:   format  = OPT_XLIST;     break;
	case OPT_TABLE:   format  = OPT_TABLE;     break;
	case OPT_TREE:    format  = OPT_TREE;      break;
        case OPT_COLS:    cols    = true;          break;

	  /* Sort */
        case OPT_UNSORT:                           break;
        case OPT_REVERSE: reverse = true;          break;

        case OPT_RELOAD:  reload  = true;          break;
	}
    }

  if (! quiet)
    {
      /* Check # of connections */
      if (len_connections ())
	{
	  /* Print user tables over current connection */
	  conn = get_current_connection ();

	  print_user_tables (format, ocilib_user_table_names (conn, reload), width, reverse, conn, cols);
	}
      else
	printf ("%s: no connection.\n", progname);
    }

  /* Bye bye! */
  return 0;
}
