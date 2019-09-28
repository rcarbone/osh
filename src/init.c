/*
 * osh - A shell for Oracle
 *
 * R. Carbone (rocco@tecsiel.it)
 * 2Q 2019
 *
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 */


/* System headers */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

/* tcsh header file */
#include "sh.h"

/* Project headers */
#include "osh.h"


/* Identifiers */
static char __version__ []  = OSH_VERSION;
static char __authors__ []  = OSH_AUTHOR;
static char __released__ [] = OSH_RELEASED;
static char __id__ []       = "A hack of the popular 'tcsh' with builtin extensions for Oracle client\n";


/* Global variable here */
osh_run_t osh_run;


/* You are welcome! */
static void helloworld (char * progname)
{
  static bool once = false;

  if (! once)
    {
      xprintf ("\n");
      xprintf ("-- %s %s (%s) -- %s\n", progname, __version__, __released__, __authors__);
      xprintf ("%s\n", __id__);

      once = true;
    }
}


/* Just few initialization steps */
void osh_init (char * progname, int quiet)
{
  osh_run . progname  = progname;
  gettimeofday (& osh_run . boottime, NULL);   /* Set time the shell booted */
  osh_run . prompt      = NULL;                /* user prompt               */
  osh_run . pcolor      = NULL;                /* default prompt ansi-color */
  osh_run . bell        = false;
  osh_run . initialized = false;

  if (! quiet)
    {
      /* Hello world! this is the shell speaking */
      helloworld (progname);

      xprintf ("Type 'help' for the list of builtin extensions implemented by this shell.\n\n");
    }

  /* Ignore writes to connections that have been closed at the other end */
  signal (SIGPIPE, SIG_IGN);

  /* Set unbuffered stdout */
  setvbuf (stdout, NULL, _IONBF, 0);

  /* Set the $osh variable */
  set_variable (OSH_PACKAGE, OSH_VERSION);

  /* Set the complete commands */
  set_completions ();

  /* Set the $prompt variable */
  osh_prompt (NULL);
}
