/*
 * osh - A shell for Oracle
 *
 * R. Carbone (rocco@tecsiel.it)
 * 2Q 2019
 *
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 */


#pragma once


/* System headers */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <libgen.h>
#include <getopt.h>
#include <signal.h>


/* Project headers */
#include "rlibc.h"
#include "ocilib.h"


/* Constants */

/* The name of the game */
#define OSH_PACKAGE      "osh"
#define OSH_VERSION      "0.1.0"
#define OSH_AUTHOR       "R. Carbone (rocco@tecsiel.it)"
#define OSH_RELEASED     __DATE__
#define OSH_LICENSE_ID   "BSD-2-Clause-FreeBSD"
#define OSH_LICENSE      "BSD 2-Clause FreeBSD License"
#define OSH_LICENSE_URL  "http://www.freebsd.org/copyright/freebsd-license.html"


/* Default Database values to access an Oracle Database istance in our development environment */
#define DEFAULT_NAME  "OSH"
#define DEFAULT_USER  "SCOTT"
#define DEFAULT_PASS  "TIGER"

/* Small buffer size */
#define MAXLINE 4096


/* Typedefs */


/* The structure contains information on the commands the application can understand */
typedef struct
{
  char * name;                                 /* builtin name      */
  char * brief;                                /* brief description */
  char * synopsis;                             /* usage synopsis    */
  char * description;                          /* long description  */
  int (* func) (int argc, char * argv []);

} osh_command_t;


/* A Connection */
typedef struct
{
  OCI_Connection * handle;  /* pointer to underlaying connection handler  */
  rtime_t when;             /* connection time at nsec resolution         */
  bool working;             /* flag to indicate if it is currently in use */

  char * error;             /* a buffer where to store underlaying errors */

  OCI_Resultset * scroll;   /* Scrollable Result Set                      */

  /* Cache */
  rtime_t updated;          /* last updated at nsec resolution            */
  char ** tabv;             /* user table names                           */

} osh_connection_t;


/* A Column */
typedef struct
{
  char * name;
  unsigned type;
  char * value;             /* eg. VARCHAR2 NUMBER CLOB BLOB */

} osh_column_t;


/* A Table */
typedef struct
{
  char * name;             /* unique table name              */

  rtime_t when;            /* last update at nsec resolution */
  unsigned count;          /* records count at [when]        */
  osh_column_t ** cols;    /* columns                        */

} osh_table_t;


/*
 * The structure to keep run-time parameters all in one,
 * defined in order to have a static, local and unique
 * container that collects all the global variables
 */
typedef struct
{
  char * progname;           /* the name of the game          */
  struct timeval boottime;   /* the time program started      */
  char * prompt;             /* user prompt                   */
  char * pcolor;             /* default ansi-color for prompt */
  bool   bell;               /* ring the bell after execution */

  bool   initialized;        /* has ocilib been initialized?  */

} osh_run_t;


/* Variables */

/* A global variable defined and initialized in file init.c */
extern osh_run_t osh_run;


/* === Helpers === */
extern osh_command_t cmd_help;
extern osh_command_t cmd_about;
extern osh_command_t cmd_version;
extern osh_command_t cmd_license;
extern osh_command_t cmd_when;

/* === Connections === */
extern osh_command_t cmd_connect;
extern osh_command_t cmd_disconnect;
extern osh_command_t cmd_lsc;
extern osh_command_t cmd_chc;

/* === User Tables === */
extern osh_command_t cmd_tables;
extern osh_command_t cmd_describe;
extern osh_command_t cmd_dtree;

/* === Viewers === */
extern osh_command_t cmd_select;

/* === Applications === */
extern osh_command_t cmd_ping;


/* Public functions in file tcsh-wrap.c */
unsigned osh_screen_rows (void);
unsigned osh_screen_cols (void);
void set_variable (char * name, char * value);
void unset_variable (char * var);
void set_completions (void);
void tcsh_builtins (int argc, char * argv []);

/* Public functions in file init.c */
void osh_init (char * progname, int quiet);

/* Public functions in file prompt.c */
void osh_prompt (char * text);

/* Public functions in file memory.c */
osh_connection_t * osh_connection_alloc (OCI_Connection * handle);
osh_connection_t * osh_connection_free (osh_connection_t * conn);
void osh_connection_done (void * conn);

osh_column_t * osh_column_alloc (char * name, unsigned type, char * value);
osh_column_t * osh_column_free (osh_column_t * col);
void osh_column_done (void * col);

/* === Containers === */

/* Public functions in file commands.c */
unsigned cmd_size (void);
char ** cmd_names (void);
osh_command_t * cmd_by_name (char * name);
osh_command_t * cmd_lookup (unsigned i);
char * cmd_by_index (unsigned i);
unsigned maxname (void);

/* Public functions in file connections.c */
char * osh_connection_name (osh_connection_t * conn);
char * osh_connection_user (osh_connection_t * conn);
char * osh_connection_pass (osh_connection_t * conn);
OCI_Connection * osh_connection_handle (osh_connection_t * conn);
rtime_t osh_connection_uptime (osh_connection_t * conn);
unsigned osh_connection_version (osh_connection_t * conn);
char * osh_connection_banner (osh_connection_t * conn);
char * osh_connection_error (osh_connection_t * conn);
void osh_set_error (osh_connection_t * conn, char * fmt, ...);

bool osh_connection_get_working (osh_connection_t * conn);
void osh_connection_set_working (osh_connection_t * conn, bool status);

osh_connection_t * get_current_connection (void);
void set_current_connection (osh_connection_t * conn);
void reset_current_connection (osh_connection_t * conn);
osh_connection_t * get_connection (unsigned id);
unsigned len_connections (void);
void add_connection (osh_connection_t * conn);
void del_connection (osh_connection_t * conn);

osh_connection_t ** get_connections (void);
osh_connection_t ** conn_sort_by_tnsname (void);
osh_connection_t ** conn_sort_by_user (void);
osh_connection_t ** conn_sort_by_uptime (void);
osh_connection_t ** conn_rev_sort_by_tnsname (void);
osh_connection_t ** conn_rev_sort_by_user (void);
osh_connection_t ** conn_rev_sort_by_uptime (void);

/* === Helpers === */

/* Public functions in file help.c */
int osh_exit (int argc, char * argv []);
int osh_quit (int argc, char * argv []);
int osh_help (int argc, char * argv []);

/* Public functions in file about.c */
int osh_about (int argc, char * argv []);

/* Public functions in file version.c */
int osh_version (int argc, char * argv []);

/* Public functions in file license.c */
int osh_license (int argc, char * argv []);

/* Public functions in file when.c */
int osh_when (int argc, char * argv []);

/* === Library === */

/* Public functions in file ocilib.c */

char * ocilib_date (OCI_Resultset * rs, unsigned c);
GNode * osh_mktree (osh_connection_t * conn, bool reload, bool expand);

unsigned ocilib_table_count (osh_connection_t * conn, char * table);
char ** ocilib_table_names (osh_connection_t * conn, char * table);
unsigned ocilib_column_count (osh_connection_t * conn, char * table);
char ** ocilib_column_names (osh_connection_t * conn, char * table);

bool ocilib_initialize (void);
void ocilib_cleanup (void);
osh_connection_t * ocilib_connect (char * name, char * user, char * pass);
OCI_Connection * ocilib_disconnect (OCI_Connection * handle);
bool ocilib_status (OCI_Connection * handle);

unsigned ocilib_column_count (osh_connection_t * conn, char * table);
char ** ocilib_column_names (osh_connection_t * conn, char * table);
osh_column_t ** ocilib_columns (osh_connection_t * conn, char * table);

unsigned rs_size (OCI_Resultset * rs);
OCI_Resultset * ocilib_resultset (osh_connection_t * conn, char * query);
OCI_Resultset * ocilib_scrollable_resultset (osh_connection_t * conn, char * query);

unsigned ocilib_user_table_count (osh_connection_t * conn, bool reload);
char ** ocilib_user_table_names (osh_connection_t * conn, bool reload);

char ** rstoargv (unsigned rssize, OCI_Resultset * rs, unsigned size, unsigned absolute);
mx_t * rstomx (unsigned rssize, OCI_Resultset * rs, unsigned n, unsigned from);
GNode * rstotree (unsigned rssize, OCI_Resultset * rs, unsigned n);

void print_record (OCI_Resultset * rs);

void print_curses (unsigned rssize, OCI_Resultset * rs, unsigned wsize, char * progname, char * version);


/* === Connections === */

/* Public functions in file connect.c */
int osh_connect (int argc, char * argv []);

/* Public functions in file disconnect.c */
int osh_disconnect (int argc, char * argv []);

/* Public functions in file list.c */
int osh_lsc (int argc, char * argv []);

/* Public functions in file change.c */
int osh_chc (int argc, char * argv []);

/* === User Tables === */

/* Public functions in file user-tables.c */
int osh_tables (int argc, char * argv []);

/* Public functions in file describe.c */
int osh_describe (int argc, char * argv []);

/* Public functions in file select.c */
int osh_select (int argc, char * argv []);

/* Public functions in file ping.c */
int osh_oping (int argc, char * argv []);


/*
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Do not edit anything below, configure creates it.
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

/* Definitions for builtin extensions to the shell will be automatically inserted here by the configure script */
