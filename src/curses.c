/*
 * osh - A shell for Oracle
 *
 * R. Carbone (rocco@tecsiel.it)
 * 2Q 2019
 *
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 */


/* System headers */
#include <ncurses.h>

/* Project headers */
#include "osh.h"

/*
 * 4 lines reserved to header (Version && Uptime / Size 
 *
 *  1. osh version 0.1.0 - Connected to Database Server: CHT_ACTIVITI@OSH            Local Time: 11:37:27am
 *  2. Query: select id,ip_address,hostname from cht_target
 *  3. ResultSet size: 89 - Window size: 55 - Current: 0
 *  4. user input line
 */
#define HEADER_LINES  4


/* Where to place an input row */
#define INPUT_ROW     3     /* zero-based */


/* Reserved keys */
#define KEY_ESC       '\033' /* Escape                       */
#define KEY_QUIT      'q'    /* Quit Key                     */


static unsigned current_row = 0;   /* current row counter */


static unsigned first_offset (void);


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


static unsigned eval_pages (unsigned rssize, unsigned items_per_page)
{
  unsigned pages = items_per_page ? rssize / items_per_page : 1;

  return rssize && rssize % items_per_page ? ++ pages : pages;
}


static unsigned current_page (unsigned offset, unsigned items_per_page)
{
  return ! items_per_page ? 1 : offset / items_per_page + 1;
}


static bool is_first_page (unsigned offset)
{
  return offset == first_offset ();
}


static bool is_last_page (unsigned offset, unsigned rssize, unsigned items_per_page)
{
  return current_page (offset, items_per_page) == eval_pages (rssize, items_per_page);
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* [offset] is one-based (always in the range [1 - xxx]) */

static unsigned first_offset (void)
{
  return 1;
}


static unsigned last_offset (unsigned rssize, unsigned items_per_page)
{
  if (rssize == items_per_page)
    return first_offset ();
  return (eval_pages (rssize, items_per_page) - 1) * items_per_page + 1;
}


static unsigned prev_offset (unsigned offset, unsigned items_per_page)
{
  return is_first_page (offset) ? first_offset () : offset - items_per_page;
}


static unsigned next_offset (unsigned offset, unsigned rssize, unsigned items_per_page)
{
  return is_last_page (offset, rssize, items_per_page) ? offset : offset + items_per_page;
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* [cursor] is one-based (always in the range [1 - items_per_page]) */

static unsigned first_cursor (void)
{
  return 1;
}


static unsigned last_cursor (unsigned rssize, unsigned items_per_page)
{
  return rssize - last_offset (rssize, items_per_page) + 1;
}


static unsigned next_cursor (unsigned cursor, unsigned items_per_page)
{
  return cursor == items_per_page ? first_cursor () : cursor + 1;
}


static unsigned prev_cursor (unsigned cursor, unsigned items_per_page)
{
  return cursor == first_cursor () ? items_per_page : cursor - 1;
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static char * sprintf_now (void)
{
  static char textline [MAXLINE];

  time_t now = time (NULL);
  struct tm * tm = localtime (& now);

  sprintf (textline,
	   "%2d:%02d:%02d%s",
	   tm -> tm_hour == 12 ? 12 : tm -> tm_hour % 12,
	   tm -> tm_min,
	   tm -> tm_sec,
	   tm -> tm_hour >= 13 ? "pm" : "am");

  return textline;
}


/*
 * Define left part of the initial banner
 *
 * something like this including Uptime && Status:
 * osh version 0.1.0 -  Connected to Database Server USER@OSH
 */
static char * sprintf_left_banner (char * progname, char * version, char * tnsname, char * user)
{
  static char textline [MAXLINE];

  /* Uptime && Status */
  sprintf (textline, "%s version %s - Connected to Database Server: %s@%s", progname, version, user, tnsname);
  return textline;
}


/*
 * Define right part of the initial banner
 *
 * something like this including Current date:
 * Local Time:  9:25:57am
 */
static char * sprintf_right_banner (void)
{
  static char textline [MAXLINE];

  sprintf (textline, "Local Time: %s", sprintf_now ());
  return textline;
}


/*
 * Left justify 'left' and right justify 'right', clipping all to fit window size.
 *
 * Rules:
 *   size < rlen : print nothing
 *   size < rlen + llen + 1: print right
 *   size = rlen + llen + 1: print left, right
 *   size < rlen + llen + 4: print header, ..., right
 *   size < rlen + llen +    wcommand_columns: print left,
 *                           truncated, ..., right
 *   size > "": print left, blanks, right
 *
 * this is slightly different from how it used to be
 */
static char * l_and_r (char * left, char * right, int size)
{
  static char buffer [MAXLINE];

  int llen = left ? strlen (left) : 0;
  int rlen = right ? strlen (right) : 0;

  if (size < rlen)
    return NULL;

  memset (buffer, ' ', sizeof (buffer));             /* blank buffer */

  if (llen && llen + rlen + 1 <= size)
    memcpy (buffer, left, llen);                     /* Left */

  if (rlen)
    memcpy (buffer + size - rlen + 1, right, rlen);  /* Right */

  buffer [size + 1] = '\0';

  return buffer;
}


/* Build a summary line for Query */
static char * sprintf_summary_query (char * query)
{
  static char textline [MAXLINE];

  sprintf (textline, "Query: %s", query);
  return textline;
}


/* Build a summary line for records */
static char * sprintf_summary_records (unsigned rssize, unsigned items_per_page, unsigned offset, unsigned cursor, unsigned rows, unsigned cols)
{
  static char textline [MAXLINE];
  char fmt [MAXLINE];

  unsigned pages = eval_pages (rssize, items_per_page);
  unsigned page  = current_page (offset, items_per_page);
  unsigned lasto = last_offset (rssize, items_per_page);
  unsigned lastc = last_cursor (rssize, items_per_page);

  sprintf (fmt, "Page: %%%dd of %%%dd - ResultSet: %%%dd - Items per page: %%%dd - Offset: %%%dd - Cursor: %%%dd - Rows: %%%dd - Cols: %%%dd - Last offset: %%%dd - Last cursor: %%%dd",
	   digits (rssize), digits (items_per_page), digits (offset), digits (cursor), digits (page), digits (pages), digits (rows), digits (cols),
 digits (lasto), digits (lastc));
  sprintf (textline, fmt, page, pages, rssize, items_per_page, offset, cursor, rows, cols, lasto, lastc);
  return textline;
}


/* Define and display the information in the heading */
static void display_heading (char * progname, char * version, char * tnsname, char * user,             /* 1-st row */
			     char * query,                                                             /* 2-nd row */
			     unsigned rssize, unsigned pagesize, unsigned offset, unsigned cursor,     /* 3-rd row */
			     unsigned rows, unsigned cols)
{
  /* Start at first available row */
  current_row = 0;

  /* 1-st row - Software && Current Time */
  mvaddstr (current_row ++, 0, l_and_r (sprintf_left_banner (progname, version, tnsname, user), sprintf_right_banner (), cols - 1));

  /* 2-nd row - Query */
  attron (A_BOLD);
  mvaddstr (current_row ++, 0, sprintf_summary_query (query));
  attroff (A_BOLD);

  /* 3-rd row - Sizes */
  mvaddstr (current_row ++, 0, sprintf_summary_records (rssize, pagesize - 1, offset, cursor, rows, cols));

  current_row ++;
}


/* Show all lines in [argv] until there is space left on the window */
static char ** display_lines (char * argv [], unsigned pagesize, unsigned cursor)
{
  char ** lines = argv;
  unsigned count = 0;

  while (argv && * argv && pagesize --)
    {
      /* First item always in Bold (column names) */
      if (count == 0)
	attron (A_BOLD);
      else if (count == cursor)
	attron (A_REVERSE);

      /* Print current line content */
      mvaddstr (current_row ++, 0, * argv ++);

      count ++;

      if (count == 1)
	attroff (A_BOLD);
      else if (count == cursor + 1)
	attroff (A_REVERSE);
    }
  return lines;
}


/* Retrieve max [pagesize] records and display them ([cursor] in bold) */
static void print_current_page (char * progname, char * version, unsigned rssize, OCI_Resultset * rs,
				unsigned pagesize, unsigned offset, unsigned cursor, unsigned rows, unsigned cols)
{
  unsigned items_per_page = rows - HEADER_LINES - 1;

  clear ();

  /* Show heading (Uptime && Database information) */
  display_heading (progname, version,
		   (char *) OCI_GetDatabase (OCI_StatementGetConnection (OCI_ResultsetGetStatement (rs))),
		   (char *) OCI_GetUserName (OCI_StatementGetConnection (OCI_ResultsetGetStatement (rs))),
		   (char *) OCI_GetSql (OCI_ResultsetGetStatement (rs)),
		   rssize, pagesize, offset, cursor, rows, cols);

  /* Limits # of items in the last page */
  if (is_last_page (offset, rssize, items_per_page))
    items_per_page = rssize - offset + 1;

  /* Retrieve max [pagesize] records and display them ([cursor] in bold) */
  argsclear (display_lines (rstoargv (rssize, rs, items_per_page, offset), pagesize, cursor));

  /* Move cursor back to line/message area */
  move (INPUT_ROW, 0);
  refresh ();
}


/* Display a ResulSet in a window under curses control */
static void do_key (char * progname, char * version, unsigned rssize, OCI_Resultset * rs, unsigned rows, unsigned cols, unsigned pagesize);
void print_curses (unsigned rssize, OCI_Resultset * rs, unsigned wsize, char * progname, char * version)
{
  if (rssize)
    {
      unsigned pagesize;
      unsigned rows;
      unsigned cols;

      initialize_curses ();
      keypad (stdscr, TRUE);
      getmaxyx (stdscr, rows, cols);                                  /* find the boundaries of the screeen */

      /* Evaluate the max # of rows per page (add 1 in order to include column names) */
      pagesize = ! wsize ? RMIN (rssize + 1, rows - HEADER_LINES) : RMIN (rssize + 1, RMIN (wsize + 1, rows - HEADER_LINES));

      /* Display a ResulSet in a window under curses control */
      do_key (progname, version, rssize, rs, rows, cols, pagesize);

      terminate_curses ();
    }
}


/* Process keyboard input during the main rendering loop */
static void do_key (char * progname, char * version, unsigned rssize, OCI_Resultset * rs, unsigned rows, unsigned cols, unsigned pagesize)
{
  bool done = false;
  unsigned offset = first_offset ();    /* one-based - always in the range [1 - rssize]   */
  unsigned cursor = first_cursor ();    /* one-based - always in the range [1 - pagesize] */

  unsigned items_per_page = pagesize - 1;

  /* outer loop until the user quits */
  while (! done)
    {
      bool valid = false;      /* valid user keys */
      int key;

      /* Here is the meat - Display page paging [argv] in max [pagesize] lines per page */
      print_current_page (progname, version, rssize, rs, pagesize, offset, cursor, rows, cols);

      /* inner loop to handle user input */
      while (! valid)
	{
	  /* get a key and update both [offset] and [cursor] to handle boundaries */
	  switch (key = getch ())
	    {
	      /* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

	      /* first page */
	    case KEY_HOME:
	      valid = true;

	      offset = first_offset ();
	      cursor = first_cursor ();
	      break;

	      /* last page */
	    case KEY_END:
	      valid = true;

	      offset = last_offset (rssize, items_per_page);
	      cursor = last_cursor (rssize, items_per_page);
	      break;

	      /* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

	      /* prev page */
	    case KEY_PPAGE:
	      valid = true;

	      offset = prev_offset (offset, items_per_page);
	      break;

	      /* next page */
	    case KEY_NPAGE:
	      valid = true;

	      offset = next_offset (offset, rssize, items_per_page);

	      if (is_last_page (offset, rssize, items_per_page))
		if (cursor > last_cursor (rssize, items_per_page))
		  cursor = last_cursor (rssize, items_per_page);
	      break;

	      /* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

	      /* prev line */
	    case KEY_UP:
	      valid = true;

	      if (is_first_page (offset) && cursor == first_cursor ())
		;
	      else
		{
		  cursor = prev_cursor (cursor, items_per_page);
		  if (cursor == items_per_page)
		    offset = prev_offset (offset, items_per_page);
		}
	      break;

	      /* next line */
	    case KEY_DOWN:
	      valid = true;

	      if (is_last_page (offset, rssize, items_per_page))
		{
		  if (cursor == items_per_page)
		    ;
		  else
		    cursor = next_cursor (cursor, items_per_page);
		}
	      else
		{
		  cursor = next_cursor (cursor, items_per_page);
		  if (cursor == first_cursor ())
		    offset = next_offset (offset, rssize, items_per_page);
		}
	      break;

	      /* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

	    case KEY_QUIT:                      /* adios */
	    case KEY_ESC:                       /* adios */
	      valid = true;                     /* exit from user input loop */
	      done = true;                      /* exit from main loop */
	      break;                            /* Bye bye cruel world ! */

	    default:                            /* ignore all others */
	      break;
	    }
	}
    }

  /* Move to last line */
  move (rows - 1, 0);
  clrtoeol ();
  refresh ();
}
