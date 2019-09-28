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


/* constants/types/variables/functions required to link */

#define Char char

struct command;

char * progname = OSH_PACKAGE;

Char STRprompt [256];
Char STRstatus [256];
Char STR1 [256];
int TermH;
int TermV;

/* Dummy functions required to link */
int xprintf (const char * s, ...)                    { return 0; }
char * s_strsave (char * s)                          { return s; }
void setcopy (const char * a, const char * b, int n) {           }

char ** blk2short (char ** a)                        { return a; }
const char  * short2str (const Char * a)             { return a; }

void doset (Char ** a, struct command * b)           { };
void unset (Char ** a, struct command * b)           { };
void docomplete (Char ** a, struct command * b)      { };
void doecho (Char ** a, struct command * b)          { };
