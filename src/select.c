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
#define NAME         "select"
#define BRIEF        "Filter columns from table"
#define SYNOPSIS     "select [options] col[,col[,col] ...] from table [where <condition>]"
#define DESCRIPTION  "Execute a select statement in order to filter records/columns from tables. Require valid connection"

/* Public variable */
osh_command_t cmd_select = { NAME, BRIEF, SYNOPSIS, DESCRIPTION, osh_select };


/* GNU short options */
enum
{
  /* Startup */
  OPT_HELP   = 'h',
  OPT_QUIET  = 'q',

  /* ResultSet size */
  OPT_RSSIZE = 'n',

  /* Output formats */
  OPT_MX     = 'm',
  OPT_TREE   = 't',
  OPT_CURSES = 'c'
};


/* GNU long options */
static struct option lopts [] =
{
  /* Startup */
  { "help",   no_argument,       NULL, OPT_HELP   },
  { "quiet",  no_argument,       NULL, OPT_QUIET  },

  /* ResultSet size */
  { "size",   required_argument, NULL, OPT_RSSIZE },

  /* Output formats */
  { "table",  no_argument,       NULL, OPT_MX     },
  { "curses", no_argument,       NULL, OPT_TREE   },
  { "tree",   no_argument,       NULL, OPT_CURSES },

  { NULL,     0,                 NULL, 0          }
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
  usage_item (options, n, OPT_HELP,   "show this help message and exit");
  usage_item (options, n, OPT_QUIET,  "run quietly");
  printf ("\n");

  /* ResultSet size */
  usage_item (options, n, OPT_RSSIZE, "# of records to display (0 means all)");
  printf ("\n");

  /* Output formats */
  usage_item (options, n, OPT_MX,     "display in a formatted table");
  usage_item (options, n, OPT_TREE,   "display in a tree");
  usage_item (options, n, OPT_CURSES, "display in a window");
}


/* Query Database and get records in a matrix */
static void print_mx (unsigned rssize, OCI_Resultset * rs, unsigned n)
{
  if (rssize)
    {
      /* Fill the ResulSet in a matrix */
      mx_t * mx = rstomx (rssize, rs, n, 1);

      /* Print the data */
      mxprint (mx);

      /* Memory cleanup */
      mxfree (mx);
    }
}


/* Query Database and get records in a tree */
static void print_tree (unsigned rssize, OCI_Resultset * rs, unsigned n)
{
  GNode * root = rstotree (rssize, rs, n);

  if (root)
    {
      /* Print the data */
      g_node_hprint_rosetta (root);

      /* Memory cleanup */
      g_node_no_more (root);
      g_node_destroy (root);
    }
}


/* The [select] command */
int osh_select (int argc, char * argv [])
{
  char * progname = basename (argv [0]);
  char * sopts    = optlegitimate (lopts);

  /* Variables that are set according to the specified options */
  bool quiet      = false;
  unsigned wsize  = 0;             /* how many records to display */
  unsigned fmt    = OPT_MX;

  osh_connection_t * conn;
  unsigned i;
  OCI_Resultset * rs;
  unsigned rssize;
  char * query;
  rtime_t t1;
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
	case OPT_HELP:   usage (progname, lopts); return 0;
	case OPT_QUIET:  quiet = true;            break;

	  /* ResultSet size */
	case OPT_RSSIZE: wsize = atoi (optarg);   break;

	  /* Output formats */
	case OPT_MX:     fmt = option;            break;
	case OPT_TREE:   fmt = option;            break;
	case OPT_CURSES: fmt = option;            break;
	}
    }

  /* Check # of connections */
  if (! len_connections ())
    {
      if (! quiet)
	printf ("%s: no connection.\n", progname);
      return 0;
    }

  /* Check for arguments */
  if (argc - optind < 3)   /* select [options] <cols> from <table> */
    {
      if (! quiet)
	{
	  printf ("%s: Too few arguments [%u].\n", progname, argc);
	  while (argc != optind)
	    printf ("%s\n", argv [optind ++]);
	}
      return 1;
    }

  /* Select records over current connection */
  conn = get_current_connection ();

  /* Built the query from remaining non-option arguments */
  i = 1;
  while (optind < argc)
    argv [i ++] = argv [optind ++];
  argv [i] = NULL;
  query = argsjoin (argv);

  /* Retrieve a scrollable ResultSet */
  if (! quiet)
    printf ("%s: querying for [%s] ... ", progname, query);

  /* Do the job */
  t1 = nswall ();
  rs = ocilib_scrollable_resultset (conn, query);
  if (! rs)
    {
      printf ("failed - [%s]\n", osh_connection_error (conn));
      return 1;
    }

  /* Evaluate the size of the ResultSet */
  rssize = rs_size (rs);

  if (! quiet)
    printf ("Ok! #%u records found in %s\n", rssize, ns2a (nswall () - t1));

  /* Render the ResulSet in one of available format */
  if (rssize)
    {
      switch (fmt)
	{
	case OPT_MX:     print_mx (rssize, rs, wsize);                                               break;
	case OPT_TREE:   print_tree (rssize, rs, wsize);                                             break;
	case OPT_CURSES: print_curses (wsize ? wsize : rssize, rs, wsize, OSH_PACKAGE, OSH_VERSION); break;
	}
    }
  else if (! quiet)
    printf ("%s: no data to display\n", progname);

  safefree (query);

  /* Bye bye! */
  return 0;
}
