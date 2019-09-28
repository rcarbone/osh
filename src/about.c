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

/* Oracle OCI */
#include "oci.h"

/* C Wrapper for Oracle OCI */
#include "ocilib.h"

/* tcsh */
#include "config.h"
#include "patchlevel.h"


/* Identifiers */
#define NAME         "about"
#define BRIEF        "About the shell"
#define SYNOPSIS     "about [options]"
#define DESCRIPTION  "No description yet"

/* Public variable */
osh_command_t cmd_about = { NAME, BRIEF, SYNOPSIS, DESCRIPTION, osh_about };


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


/* The [about] command */
int osh_about (int argc, char * argv [])
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

      printf ("%s, ver. %s built for %s-%s on %s %s\n", OSH_PACKAGE, OSH_VERSION, u . sysname, u . machine, __DATE__, __TIME__);
      printf ("Copyright(c) 2019 %s\n", OSH_AUTHOR);
      printf ("License: %s\n", OSH_LICENSE_ID);
      printf ("\n");
      printf ("%s is provided AS IS and comes with ABSOLUTELY NO WARRANTY.\n", OSH_PACKAGE);
      printf ("This is free software, and you are welcome to redistribute it under the terms of the license.\n");
      printf ("\n");
      printf ("Based on:\n");
      printf ("  tcsh v. %d.%d.%d  - C shell with file name completion and command line editing - %s\n", REV, VERS, PATCHLEVEL, "Christos Zoulas <christos@NetBSD.org>");
      printf ("  OCILIB v. %u.%u.%u - C Driver for Oracle - %s\n", OCILIB_MAJOR_VERSION, OCILIB_MINOR_VERSION, OCILIB_MINOR_VERSION, "Vincent Rogier <vince.rogier@ocilib.net>");
      printf ("  OCI v. %u.%u     - Oracle Instant Client and Call Interface - %s\n", OCI_MAJOR_VERSION, OCI_MINOR_VERSION, "Copyright (c) 1995, 2019 Oracle");
      printf ("  rlibc v. 1.0.3  - C library of useful functions - %s\n", "R. Carbone <rocco@tecsiel.it>");
    }

  /* Bye bye! */
  return 0;
}
