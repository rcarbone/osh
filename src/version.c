/*
 * osh - A shell for Oracle
 *
 * R. Carbone (rocco@tecsiel.it)
 * 2Q 2019
 *
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 */


/* System headers */
#include <sys/utsname.h>

/* Project headers */
#include "osh.h"


/* Identifiers */
#define NAME         "version"
#define BRIEF        "About the shell version"
#define SYNOPSIS     "version [options]"
#define DESCRIPTION  "No description yet"

/* Public variable */
osh_command_t cmd_version = { NAME, BRIEF, SYNOPSIS, DESCRIPTION, osh_version };


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

  printf ("Startup:\n");
  usage_item (options, n, OPT_HELP,  "show this help message and exit");
  usage_item (options, n, OPT_QUIET, "run quietly");
}


/* The [version] command */
int osh_version (int argc, char * argv [])
{
  char * progname = basename (argv [0]);
  char * sopts    = optlegitimate (lopts);

  /* Variables that are set according to the specified options */
  bool quiet      = false;

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

  if (! quiet)
    {
      struct utsname u;
      uname (& u);
      printf ("%s, ver. %s\n", OSH_PACKAGE, OSH_VERSION);
      printf ("build date:   %s %s\n", __DATE__, __TIME__);
      printf ("build system: %s %s (%s) %s %s\n", u . sysname, u . machine, rfqname (), u . version, u . release);
    }

  /* Bye bye! */
  return 0;
}
