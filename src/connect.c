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
#define NAME         "connect"
#define BRIEF        "Create a physical connection to a Database server"
#define SYNOPSIS     "connect [options]"
#define DESCRIPTION  "Connect to a database"

/* Public variable */
osh_command_t cmd_connect = { NAME, BRIEF, SYNOPSIS, DESCRIPTION, osh_connect };

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
  OPT_HELP    = 'h',
  OPT_QUIET   = 'q',

  /* Database */
  OPT_TNSNAME = 'n',    /* TNS Name */
  OPT_USER    = 'u',    /* User     */
  OPT_PASS    = 'p'     /* Password */
};


/* GNU long options */
static struct option lopts [] =
{
  /* Startup */
  { "help",  no_argument,       NULL, OPT_HELP    },
  { "quiet", no_argument,       NULL, OPT_QUIET   },

  /* Database */
  { "name",  required_argument, NULL, OPT_TNSNAME },
  { "user",  required_argument, NULL, OPT_USER    },
  { "pass",  required_argument, NULL, OPT_PASS    },

  { NULL,    0,                 NULL, 0           }
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

  printf ("Database:\n");
  usage_item (options, n, OPT_TNSNAME, "TNS name");
  usage_item (options, n, OPT_USER,    "user");
  usage_item (options, n, OPT_PASS,    "password");
}


/* The [connect] command */
int osh_connect (int argc, char * argv [])
{
  char * progname = basename (argv [0]);
  char * sopts    = optlegitimate (lopts);

  /* Variables that are set according to the specified options */
  bool quiet      = false;
  char * name     = DEFAULT_NAME;
  char * user     = DEFAULT_USER;
  char * pass     = DEFAULT_PASS;

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
	case OPT_HELP:    usage (progname, lopts); return 0;
	case OPT_QUIET:   quiet = true;            break;

	  /* Database */
	case OPT_TNSNAME: name = optarg;    break;
	case OPT_USER:    user = optarg;    break;
	case OPT_PASS:    pass = optarg;    break;
	}
    }

  /* Attempt to initialize the ocilib library (ignore errors due to multiple connect/disconnect) */
  if (! osh_run . initialized && ! ocilib_initialize ())
    {
      printf ("Cannot initialize library.\n");
      return 1;
    }

  /* Enable ^C */
  on_prev = signal (SIGINT, on_ctrl_c);

  /* Create a physical connection to an Oracle database server */
  if (! quiet)
    printf ("%s: connecting to database '%s:xxx@%s' ... ", progname, user, name);
  if (! (conn = ocilib_connect (name, user, pass)))
    {
      if (! quiet)
	{
	  if (! interrupted)
	    printf ("Failed!\n");
	  else
	    printf ("\n");
	}

      /* Clean up all resources allocated by the library */
      ocilib_cleanup ();

      return 1;
    }
  if (! quiet)
    printf ("Ok!\n");

  /* Add to the table of connections */
  add_connection (conn);

  /* Keep track of current connection */
  set_current_connection (conn);

  /* Update user prompt to include current connection */
  osh_prompt (osh_connection_name (conn));

  /* Re-enable ^C to its previous handler */
  signal (SIGINT, on_prev);

  /* Bye bye! */
  return 0;
}
