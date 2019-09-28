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
#include "ocilib.h"


/* Identifiers */
#define NAME         "oping"
#define BRIEF        "Ping an Oracle server over a connection"
#define SYNOPSIS     "oping [options]"
#define DESCRIPTION  "Makes a round trip call to the server to confirm that the connection and the server are active"

/* Public variable */
osh_command_t cmd_ping = { NAME, BRIEF, SYNOPSIS, DESCRIPTION, osh_oping };


static bool interrupted = false;


/* What to do on ^C */
static void on_ctrl_c (int signo)
{
  interrupted = true;
  fflush (stdout);
}


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


/* The [oping] command */
int osh_oping (int argc, char * argv [])
{
  char * progname = basename (argv [0]);
  char * sopts    = optlegitimate (lopts);

  /* Variables that are set according to the specified options */
  bool quiet      = false;

  unsigned seq    = 0;
  rtime_t started;
  rtime_t stopped;
  bool pinged;
  rtime_t min     = 0xffffffff;
  rtime_t avg     = 0;
  rtime_t max     = 0;

  /* signal handler */
  void (* on_prev) (int);

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
	case OPT_HELP:  usage (progname, lopts); return 0;
	case OPT_QUIET: quiet = true;            break;
	}
    }

  /* Check # of connections */
  if (! len_connections ())
    {
      if (! quiet)
	printf ("%s: no connection.\n", progname);
      return 0;
    }

  /* Ping server over current connection */
  conn = get_current_connection ();

  /* Enable ^C */
  on_prev = signal (SIGINT, on_ctrl_c);

  /* Main loop - iterate to run ping and evaluate min/max/avg */
  started = nswall ();
  while (! interrupted)
    {
      /* Ping the connection */
      rtime_t t1 = nswall ();
      rtime_t elapsed;

      pinged = OCI_Ping (osh_connection_handle (conn));
      elapsed = nswall () - t1;

      /* Evaluate min/max/avg time elapsed */
      min = RMIN (min, elapsed);
      max = RMAX (max, elapsed);
      avg += elapsed;

      if (! pinged)
	{
	  if (! quiet)
	    printf ("%s: Error! Cannot ping\n", progname);
	  break;
	}
      else
	{
	  if (! quiet)
	    printf ("reply from %s: seq=%u time=%s\n", OCI_GetDatabase (osh_connection_handle (conn)), ++ seq, ns2a (elapsed));
	}
    }
  stopped = nswall ();

  interrupted = false;

  avg /= seq;   /* normalize */

  printf ("\n");
  printf ("%8.8s  / %8.8s  / %8.8s    - Elapsed\n", "Min", "Max", "Avg");
  printf (" %8.3f  / %8.3f  / %8.3f    - %12.12s\n",
	  min / 1e6,
	  max / 1e6,
	  avg / 1e6,
	  ns2a (stopped - started));

  /* Re-enable ^C to its previous handler */
  signal (SIGINT, on_prev);

  /* Bye bye! */
  return 0;
}
