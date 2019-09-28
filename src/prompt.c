/*
 * osh - A shell for Oracle
 *
 * R. Carbone (rocco@tecsiel.it)
 * 2Q 2019
 *
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 */


/* tcsh header file(s) */
#include "sh.h"

/* Project headers */
#include "osh.h"


/* Change the prompt */
void osh_prompt (char * text)
{
  char * prompt = calloc (1, text ? strlen (progname) + strlen (text) + 200 : strlen (progname) + 200);
  Char Prompt [512];

  unsigned i;
  unsigned len;

  if (text)
    sprintf (prompt, "%%S%s@%s %%!>%%s ", progname, text);
  else
    sprintf (prompt, "%%S%s %%!>%%s ", progname);

  len = strlen (prompt);
  for (i = 0; i <= len; i ++)
    Prompt [i] = prompt [i];

  free (prompt);

  /* Set the prompt in the shell */
  setcopy (STRprompt, Strsave (Prompt), VAR_READWRITE);
}
