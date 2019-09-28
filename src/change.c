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
#define NAME         "chc"
#define BRIEF        "Change the current connection"
#define SYNOPSIS     "chc id"
#define DESCRIPTION  "No description yet"

/* Public variable */
osh_command_t cmd_chc = { NAME, BRIEF, SYNOPSIS, DESCRIPTION, osh_chc };


/* GNU short options */
enum
{
  /* Startup */
  OPT_HELP  = 'h',
  OPT_QUIET = 'q'
};


/* GNU long options */
static struct option lopts [] =
{
  /* Startup */
  { "help",  no_argument, NULL, OPT_HELP  },
  { "quiet", no_argument, NULL, OPT_QUIET },

  { NULL,    0,           NULL, 0         }
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
}


/* The [chc] command */
int osh_chc (int argc, char * argv [])
{
  char * progname = basename (argv [0]);
  char * sopts    = optlegitimate (lopts);

  /* Variables that are set according to the specified options */
  bool quiet      = false;

  osh_connection_t * conn;
  unsigned id;
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
	}
    }

  /* Check # of connections */
  if (len_connections ())
    {
      /* Check for optional/mandatory arguments */
      switch (argc - optind)
	{
	case 0:   /* no arguments - nothing to do */
	  break;

	case 1:
	  /* Check for a legal identifier and retrieve the connection handler */
	  id = atoi (argv [argc - 1]);
	  conn = get_connection (id);
	  if (! conn)
	    {
	      if (! quiet)
		printf ("%s: Unknown connection [%s].\n", progname, argv [argc - 1]);
	      return 1;
	    }

	  /* Keep track of given connection */
	  set_current_connection (conn);

	  printf ("%s: current now %u\n", progname, id);

	  /* Update user prompt to include the current connection */
	  osh_prompt (osh_connection_name (conn));

	  break;

	default:
	  printf ("%s: too many arguments (max #%u)\n", progname, argc - optind - 1);
	  while (argc != optind)
	    printf ("%s\n", argv [optind ++]);
	  return 1;
	}
    }
  else
    printf ("%s: no connection.\n", progname);

  /* Bye bye! */
  return 0;
}
