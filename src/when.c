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
#define NAME         "when"
#define BRIEF        "Tells how long the shell has been up and short info about some important user-tables"
#define SYNOPSIS     "when [options]"
#define DESCRIPTION  "No description yet"

/* Public variable */
osh_command_t cmd_when = { NAME, BRIEF, SYNOPSIS, DESCRIPTION, osh_when };


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


/* The [when] command */
int osh_when (int argc, char * argv [])
{
  char * progname = basename (argv [0]);
  char * sopts    = optlegitimate (lopts);

  /* Variables that are set according to the specified options */
  bool quiet      = false;
  time_t now      = time (NULL);
  struct tm * tm  = localtime (& now);

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

  /* Initialize bootime if not already done */
  if (! osh_run . boottime . tv_sec)
    gettimeofday (& osh_run . boottime, NULL);             /* Set time the program booted */

  /*
   * a simple banner, something like:
   * 12:01:33am   up   0 days,  0:00:35,    1 connection
   */
  if (! quiet)
    printf ("%2d:%02d:%02d%s   up %3d days, %2d:%02d:%02d,    %d connection%s\n",
	    tm -> tm_hour == 12 ? 12 : tm -> tm_hour % 12,
	    tm -> tm_min,
	    tm -> tm_sec,
	    tm -> tm_hour >= 13 ? "pm" : "am",
	    _days_ (now, osh_run . boottime . tv_sec),
	    _hours_ (now, osh_run . boottime . tv_sec),
	    _mins_ (now, osh_run . boottime . tv_sec),
	    (int) (now - osh_run . boottime . tv_sec) % 60,
	    len_connections (), len_connections () > 1 ? "s" : "");

  /* Bye bye! */
  return 0;
}
