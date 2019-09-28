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


/* All the commands grouped together in a static unsorted table */
static osh_command_t * commands [] =
{
  & cmd_help,
  & cmd_about,
  & cmd_version,
  & cmd_license,
  & cmd_when,

  & cmd_connect,
  & cmd_disconnect,
  & cmd_lsc,
  & cmd_chc,

  & cmd_select,

  & cmd_tables,
  & cmd_describe,

  & cmd_ping,

  NULL
};


/* Helpers */
unsigned cmd_size (void)
{
  return (sizeof (commands) / sizeof (commands [0])) - 1;  /* Exclude NULL terminator */
}


/* Return all command names */
char ** cmd_names (void)
{
  char ** names = NULL;
  unsigned i;
  for (i = 0; i < cmd_size (); i ++)
    names = argsmore (names, commands [i] -> name);
  return names;
}


/* Lookup for a command by name */
osh_command_t * cmd_by_name (char * name)
{
  unsigned i;
  if (name)
    for (i = 0; i < cmd_size (); i ++)
      if (commands [i] -> name && ! strcmp (name, commands [i] -> name))
	return commands [i];
  return NULL;
}


/* Lookup for a command by index */
osh_command_t * cmd_lookup (unsigned i)
{
  return i < cmd_size() ? commands [i] : NULL;
}


/* Lookup for a command name by index */
char * cmd_by_index (unsigned i)
{
  return i < cmd_size () ? commands [i] -> name : NULL;
}


unsigned maxname (void)
{
  static unsigned val = 0;
  unsigned i;

  if (val)
    return val;

  for (i = 0; i < cmd_size (); i ++)
    if (commands [i] -> name)
      val = RMAX (val, strlen (commands [i] -> name));
  return val;
}
