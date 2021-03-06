/*	$NetBSD: optfunc.c,v 1.1.1.3 1997/09/21 12:22:52 mrg Exp $	*/

/*
 * Copyright (c) 1984,1985,1989,1994,1995,1996  Mark Nudelman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice in the documentation and/or other materials provided with 
 *    the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN 
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/*
 * Handling functions for command line options.
 *
 * Most options are handled by the generic code in option.c.
 * But all string options, and a few non-string options, require
 * special handling specific to the particular option.
 * This special processing is done by the "handling functions" in this file.
 *
 * Each handling function is passed a "type" and, if it is a string
 * option, the string which should be "assigned" to the option.
 * The type may be one of:
 *	INIT	The option is being initialized from the command line.
 *	TOGGLE	The option is being changed from within the program.
 *	QUERY	The setting of the option is merely being queried.
 */

#include "less.h"
#include "option.h"

extern int nbufs;
extern int cbufs;
extern int pr_type;
extern int plusoption;
extern int swindow;
extern int sc_height;
extern int secure;
extern int dohelp;
extern char openquote;
extern char closequote;
extern char *prproto[];
extern char *eqproto;
extern char *hproto;
extern IFILE curr_ifile;
#if LOGFILE
extern char *namelogfile;
extern int force_logfile;
extern int logfile;
#endif
#if TAGS
public char *tagoption = NULL;
extern char *tags;
extern int jump_sline;
#endif
#if MSDOS_COMPILER
extern int nm_fg_color, nm_bg_color;
extern int bo_fg_color, bo_bg_color;
extern int ul_fg_color, ul_bg_color;
extern int so_fg_color, so_bg_color;
extern int bl_fg_color, bl_bg_color;
#endif


#if LOGFILE
/*
 * Handler for -o option.
 */
	public void
opt_o(type, s)
	int type;
	char *s;
{
	PARG parg;

	if (secure)
	{
		error("log file support is not available", NULL_PARG);
		return;
	}
	switch (type)
	{
	case INIT:
		namelogfile = s;
		break;
	case TOGGLE:
		if (ch_getflags() & CH_CANSEEK)
		{
			error("Input is not a pipe", NULL_PARG);
			return;
		}
		if (logfile >= 0)
		{
			error("Log file is already in use", NULL_PARG);
			return;
		}
		s = skipsp(s);
		namelogfile = lglob(s);
		use_logfile(namelogfile);
		sync_logfile();
		break;
	case QUERY:
		if (logfile < 0)
			error("No log file", NULL_PARG);
		else
		{
			parg.p_string = unquote_file(namelogfile);
			error("Log file \"%s\"", &parg);
			free(parg.p_string);
		}
		break;
	}
}

/*
 * Handler for -O option.
 */
	public void
opt__O(type, s)
	int type;
	char *s;
{
	force_logfile = TRUE;
	opt_o(type, s);
}
#endif

/*
 * Handlers for -l option.
 */
	public void
opt_l(type, s)
	int type;
	char *s;
{
	int err;
	int n;
	char *t;
	
	switch (type)
	{
	case INIT:
		t = s;
		n = getnum(&t, 'l', &err);
		if (err || n <= 0)
		{
			error("Line number is required after -l", NULL_PARG);
			return;
		}
		plusoption = TRUE;
		ungetsc(s);
		break;
	}
}

#if USERFILE
	public void
opt_k(type, s)
	int type;
	char *s;
{
	PARG parg;

	switch (type)
	{
	case INIT:
		if (lesskey(s))
		{
			parg.p_string = unquote_file(s);
			error("Cannot use lesskey file \"%s\"", &parg);
			free(parg.p_string);
		}
		break;
	}
}
#endif

#if TAGS
/*
 * Handler for -t option.
 */
	public void
opt_t(type, s)
	int type;
	char *s;
{
	IFILE save_ifile;
	POSITION pos;

	switch (type)
	{
	case INIT:
		tagoption = s;
		/* Do the rest in main() */
		break;
	case TOGGLE:
		if (secure)
		{
			error("tags support is not available", NULL_PARG);
			break;
		}
		findtag(skipsp(s));
		save_ifile = save_curr_ifile();
		if (edit_tagfile())
			break;
		if ((pos = tagsearch()) == NULL_POSITION)
		{
			reedit_ifile(save_ifile);
			break;
		}
		unsave_ifile(save_ifile);
		jump_loc(pos, jump_sline);
		break;
	}
}

/*
 * Handler for -T option.
 */
	public void
opt__T(type, s)
	int type;
	char *s;
{
	PARG parg;

	switch (type)
	{
	case INIT:
		tags = s;
		break;
	case TOGGLE:
		s = skipsp(s);
		tags = lglob(s);
		break;
	case QUERY:
		parg.p_string = unquote_file(tags);
		error("Tags file \"%s\"", &parg);
		free(parg.p_string);
		break;
	}
}
#endif

/*
 * Handler for -p option.
 */
	public void
opt_p(type, s)
	int type;
	register char *s;
{
	switch (type)
	{
	case INIT:
		/*
		 * Unget a search command for the specified string.
		 * {{ This won't work if the "/" command is
		 *    changed or invalidated by a .lesskey file. }}
		 */
		plusoption = TRUE;
		ungetsc(s);
		ungetsc("/");
		break;
	}
}

/*
 * Handler for -P option.
 */
	public void
opt__P(type, s)
	int type;
	register char *s;
{
	register char **proto;
	PARG parg;

	switch (type)
	{
	case INIT:
	case TOGGLE:
		/*
		 * Figure out which prototype string should be changed.
		 */
		switch (*s)
		{
		case 's':  proto = &prproto[PR_SHORT];	s++;	break;
		case 'm':  proto = &prproto[PR_MEDIUM];	s++;	break;
		case 'M':  proto = &prproto[PR_LONG];	s++;	break;
		case '=':  proto = &eqproto;		s++;	break;
		case 'h':  proto = &hproto;		s++;	break;
		default:   proto = &prproto[PR_SHORT];		break;
		}
		free(*proto);
		*proto = save(s);
		break;
	case QUERY:
		parg.p_string = prproto[pr_type];
		error("%s", &parg);
		break;
	}
}

/*
 * Handler for the -b option.
 */
	/*ARGSUSED*/
	public void
opt_b(type, s)
	int type;
	char *s;
{
	switch (type)
	{
	case TOGGLE:
	case QUERY:
		/*
		 * Allocate the new number of buffers.
		 */
		cbufs = ch_nbuf(cbufs);
		break;
	case INIT:
		break;
	}
}

/*
 * Handler for the -i option.
 */
	/*ARGSUSED*/
	public void
opt_i(type, s)
	int type;
	char *s;
{
	switch (type)
	{
	case TOGGLE:
		chg_caseless();
		break;
	case QUERY:
	case INIT:
		break;
	}
}

/*
 * Handler for the -V option.
 */
	/*ARGSUSED*/
	public void
opt__V(type, s)
	int type;
	char *s;
{
	switch (type)
	{
	case TOGGLE:
	case QUERY:
	case INIT:
		dispversion();
		if (type == INIT)
			quit(QUIT_OK);
		break;
	}
}

#if MSDOS_COMPILER
/*
 *
 */
   	static void
colordesc(s, fg_color, bg_color)
	char *s;
	int *fg_color;
	int *bg_color;
{
	int fg, bg;
	int err;
	
	fg = getnum(&s, 'D', &err);
	if (err)
	{
		error("Missing fg color in -D", NULL_PARG);
		return;
	}
	if (*s != '.')
		bg = 0;
	else
	{
		s++;
		bg = getnum(&s, 'D', &err);
		if (err)
		{
			error("Missing fg color in -D", NULL_PARG);
			return;
		}
	}
	*fg_color = fg;
	*bg_color = bg;
}

/*
 * Handler for the -D option.
 */
	/*ARGSUSED*/
	public void
opt_D(type, s)
	int type;
	char *s;
{
	switch (type)
	{
	case INIT:
	case TOGGLE:
		switch (*s++)
		{
		case 'n':
			colordesc(s, &nm_fg_color, &nm_bg_color);
			break;
		case 'd':
			colordesc(s, &bo_fg_color, &bo_bg_color);
			break;
		case 'u':
			colordesc(s, &ul_fg_color, &ul_bg_color);
			break;
		case 'k':
			colordesc(s, &bl_fg_color, &bl_bg_color);
			break;
		case 's':
			colordesc(s, &so_fg_color, &so_bg_color);
			break;
		default:
			error("-D must be followed by n, d, u, k or s", NULL_PARG);
			break;
		}
		if (type == TOGGLE)
		{
			so_enter();
			so_exit();
		}
		break;
	case QUERY:
		break;
	}
}
#endif

/*
 * Handler for the -" option.
 */
	public void
opt_quote(type, s)
	int type;
	register char *s;
{
	char buf[3];
	PARG parg;

	switch (type)
	{
	case INIT:
	case TOGGLE:
		if (s[1] != '\0' && s[2] != '\0')
		{
			error("-\" must be followed by 1 or 2 chars", NULL_PARG);
			return;
		}
		openquote = s[0];
		if (s[1] == '\0')
			closequote = openquote;
		else
			closequote = s[1];
		break;
	case QUERY:
		buf[0] = openquote;
		buf[1] = closequote;
		buf[2] = '\0';
		parg.p_string = buf;
		error("quotes %s", &parg);
		break;
	}
}

/*
 * "-?" means display a help message.
 * If from the command line, exit immediately.
 */
	/*ARGSUSED*/
	public void
opt_query(type, s)
	int type;
	char *s;
{
	switch (type)
	{
	case QUERY:
	case TOGGLE:
		error("Use \"h\" for help", NULL_PARG);
		break;
	case INIT:
		dohelp = 1;
	}
}

/*
 * Get the "screen window" size.
 */
	public int
get_swindow()
{
	if (swindow > 0)
		return (swindow);
	return (sc_height + swindow);
}

