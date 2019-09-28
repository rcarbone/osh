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
#define NAME         "lsc"
#define BRIEF        "Display information about known connections"
#define SYNOPSIS     "lsc"
#define DESCRIPTION  "No description yet"

/* Mark for the working connection */
#define MARK   "******"


/* Public variable */
osh_command_t cmd_lsc = { NAME, BRIEF, SYNOPSIS, DESCRIPTION, osh_lsc };


/* GNU short options */
enum
{
  /* Startup */
  OPT_HELP         = 'h',
  OPT_QUIET        = 'q',

  /* Sorting */
  OPT_UNSORT       = 'u',
  OPT_REVERSE      = 'r',

  OPT_SORT_TNSNAME = 'N',
  OPT_SORT_USER    = 'U',
  OPT_SORT_UPTIME  = 'T',
};


/* GNU long options */
static struct option lopts [] =
{
  /* Startup */
  { "help",    no_argument, NULL, OPT_HELP         },
  { "quiet",   no_argument, NULL, OPT_QUIET        },

  /* Sorting */
  { "unsort",  no_argument, NULL, OPT_UNSORT       },
  { "reverse", no_argument, NULL, OPT_REVERSE      },

  { "tnsname", no_argument, NULL, OPT_SORT_TNSNAME },
  { "user",    no_argument, NULL, OPT_SORT_USER    },
  { "uptime",  no_argument, NULL, OPT_SORT_UPTIME  },

  { NULL,      0,           NULL, 0                }
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
  usage_item (options, n, OPT_HELP,         "show this help message and exit");
  usage_item (options, n, OPT_QUIET,        "run quietly");
  printf ("\n");

  printf ("Sorting options are:\n");
  usage_item (options, n, OPT_UNSORT,       "do not sort (default)");
  usage_item (options, n, OPT_REVERSE,      "reverse the result of sorting");

  usage_item (options, n, OPT_SORT_TNSNAME, "sort by TNS name");
  usage_item (options, n, OPT_SORT_USER,    "sort by user");
  usage_item (options, n, OPT_SORT_UPTIME,  "sort by uptime");
}


/* Print table of connections */
static void print_connections (osh_connection_t * argv [], bool reverse)
{
  /* Allocate a matrix to keep header and data */
  unsigned rows = arrlen (argv) + 1;
  unsigned cols = 8;
  mx_t * mx     = mxalloc (rows, cols);
  unsigned r    = 0;
  unsigned c    = 0;
  osh_connection_t * conn;

  /* Table header */
  mxcpy (mx, "#",        r, c ++);
  mxcpy (mx, "TNS Name", r, c ++);
  mxcpy (mx, "User",     r, c ++);
  mxcpy (mx, "Tables",   r, c ++);
  mxcpy (mx, "Records",  r, c ++);
  mxcpy (mx, "Uptime",   r, c ++);
  mxcpy (mx, "Version",  r, c ++);
  mxcpy (mx, "Working",  r, c ++);

  /* Insert the records in a matrix */
  for (r = 1; r < rows; r ++)
    for (c = 0; c < cols; c ++)
      {
	unsigned count;
	conn = reverse ? argv [rows - r - 1] : argv [r - 1];

	/* Force reload */
	if (conn -> updated)
	  count = arrlen (conn -> tabv);
	else
	  count = ocilib_user_table_count (conn, true);

	switch (c)
	  {
	  case 0: mxcpy (mx, utoa (r),                                         r, c); break;
	  case 1: mxcpy (mx, osh_connection_name (conn),                       r, c); break;
	  case 2: mxcpy (mx, osh_connection_user (conn),                       r, c); break;
	  case 3: mxcpy (mx, utoa (count),                                     r, c); break;
	  case 4: mxcpy (mx, "###",                                            r, c); break;
	  case 5: mxcpy (mx, relapsed (osh_connection_uptime (conn)),          r, c); break;
	  case 6: mxcpy (mx, utoa (osh_connection_version (conn)),             r, c); break;
	  case 7: mxcpy (mx, conn == get_current_connection () ? MARK : "   ", r, c); break;
	  }
      }

  /* Print the data */
  mxprint (mx);

  /* Memory cleanup */
  mxfree (mx);
}


/* The [lsc] command */
int osh_lsc (int argc, char * argv [])
{
  char * progname = basename (argv [0]);
  char * sopts    = optlegitimate (lopts);

  /* Variables that are set according to the specified options */
  bool quiet      = false;
  osh_connection_t ** conns = NULL;
  bool reverse    = false;

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
	case OPT_HELP:    usage (progname, lopts);              return 0;
	case OPT_QUIET:   quiet   = true;                       break;

        case OPT_UNSORT:       conns   = get_connections ();    break;
        case OPT_REVERSE:      reverse = true;                  break;

	case OPT_SORT_TNSNAME: conns = conn_sort_by_tnsname (); break;
	case OPT_SORT_USER:    conns = conn_sort_by_user ();    break;
	case OPT_SORT_UPTIME:  conns = conn_sort_by_uptime ();  break;
	}
    }

  /* Initialize bootime if not already done */
  if (! osh_run . boottime . tv_sec)
    gettimeofday (& osh_run . boottime, NULL);             /* Set time the program booted */

  /*
   * a simple banner, something like:
   * 7:07:18pm   up   0 days,  0:00:14,    1 connection
   */
  if (! quiet)
    {
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
      printf ("\n");

      /*
       * information for each connection, something like:
       *
       * TNS Name --  user,   28 tables,   1757 records - up 
       * 11:04:56am   up   0 days,  0:01:01,    1 connection
       *
       * # | TNS Name |   User   | Tables | Records |   Uptime   | Version | Connected |
       *
       * 1 | osh      | OSH_USER |     36 |    3528 |   5.0  sec |    1210 |     Y     |
       * 2 | testing  | OSH_USER |        |         |   4.4  sec |    1210 |     Y     |
       * 3 | stable   | OSH_USER |        |         |   3.8  sec |    1210 |     Y     |
       * 4 | unstable | OSH_USER |        |         |   3.0  sec |    1210 |     Y     |
       * 5 | osh2     | OSH_USER |        |         |   1.8  sec |    1210 |     Y     |
       */
      if (len_connections ())
	print_connections (conns ? conns : get_connections (), reverse);
      else
	printf ("%s: no connection.\n", progname);
    }

  /* Bye bye! */
  return 0;
}
