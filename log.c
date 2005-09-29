/******************************************************************************
  Copyright (c) 1992, 1995, 1996 Xerox Corporation.  All rights reserved.
  Portions of this code were written by Stephen White, aka ghond.
  Use and copying of this software and preparation of derivative works based
  upon this software are permitted.  Any distribution of this software or
  derivative works must comply with all applicable United States export
  control laws.  This software is made available AS IS, and Xerox Corporation
  makes no warranty about the software, its performance or its conformity to
  any specification.  Any person obtaining a copy of this software is requested
  to send their name and post office or electronic mail address to:
    Pavel Curtis
    Xerox PARC
    3333 Coyote Hill Rd.
    Palo Alto, CA 94304
    Pavel@Xerox.Com
 *****************************************************************************/

#include <errno.h>
#include "my-stdarg.h"
#include "my-stdio.h"
#include "my-string.h"
#include "my-time.h"

#include "bf_register.h"
#include "config.h"
#include "functions.h"
#include "log.h"
#include "options.h"
#include "storage.h"
#include "streams.h"
#include "utils.h"

static FILE *log_file = 0;

void
set_log_file(FILE * f)
{
    log_file = f;
}

int log_pcount = 5000;
static time_t log_prev = 0;
int log_report_progress_cktime()
{
    time_t now = time(0);
    log_pcount = 5000;
    return ((now >= log_prev + 2) && (log_prev = now, 1));
}

static void
do_log(const char *fmt, va_list args, const char *prefix)
{
    FILE *f;

    log_prev = time(0);
    log_pcount = 5000;
    if (log_file) {
	char *nowstr = ctime(&log_prev);

	nowstr[19] = '\0';	/* kill the year and newline at the end */
	f = log_file;
	fprintf(f, "%s: %s", nowstr + 4, prefix);	/* skip the day of week */
    } else
	f = stderr;

    vfprintf(f, fmt, args);
    fflush(f);
}

void
oklog(const char *fmt,...)
{
    va_list args;

    va_start(args, fmt);
    do_log(fmt, args, "");
    va_end(args);
}

void
errlog(const char *fmt,...)
{
    va_list args;

    va_start(args, fmt);
    do_log(fmt, args, "*** ");
    va_end(args);
}

void
log_perror(const char *what)
{
    errlog("%s: %s\n", what, strerror(errno));
}


#ifdef LOG_COMMANDS
static Stream *command_history = 0;
#endif

void
reset_command_history()
{
#ifdef LOG_COMMANDS
    if (command_history == 0)
	command_history = new_stream(1024);
    else
	reset_stream(command_history);
#endif
}

void
log_command_history()
{
#ifdef LOG_COMMANDS
    errlog("COMMAND HISTORY:\n%s", stream_contents(command_history));
#endif
}

void
add_command_to_history(Objid player, const char *command)
{
#ifdef LOG_COMMANDS
    time_t now = time(0);
    char *nowstr = ctime(&now);

    nowstr[19] = '\0';		/* kill the year and newline at the end */
    stream_printf(command_history, "%s: #%d: %s\n",
		  nowstr + 4,	/* skip day of week */
		  player, command);
#endif				/* LOG_COMMANDS */
}

/**** built in functions ****/

static package
bf_server_log(Var arglist, Byte next, void *vdata, Objid progr)
{
    if (!is_wizard(progr)) {
	free_var(arglist);
	return make_error_pack(E_PERM);
    } else {
	int is_error = (arglist.v.list[0].v.num == 2
			&& is_true(arglist.v.list[2]));

	if (is_error)
	    errlog("> %s\n", arglist.v.list[1].v.str);
	else
	    oklog("> %s\n", arglist.v.list[1].v.str);

	free_var(arglist);
	return no_var_pack();
    }
}

void
register_log(void)
{
    register_function("server_log", 1, 2, bf_server_log, TYPE_STR, TYPE_ANY);
}

char rcsid_log[] = "$Id: log.c,v 1.3.2.1 2005-09-29 06:56:18 bjj Exp $";

/* 
 * $Log: log.c,v $
 * Revision 1.3.2.1  2005-09-29 06:56:18  bjj
 * Merge HEAD onto WAIF, bringing it approximately to 1.8.2
 *
 * Revision 1.4  2004/05/22 01:25:43  wrog
 * merging in WROGUE changes (W_SRCIP, W_STARTUP, W_OOB)
 *
 * Revision 1.3.10.1  2003/06/03 12:19:27  wrog
 * added log_report_progress()
 *
 * Revision 1.3  1998/12/14 13:17:59  nop
 * Merge UNSAFE_OPTS (ref fixups); fix Log tag placement to fit CVS whims
 *
 * Revision 1.2  1997/03/03 04:18:48  nop
 * GNU Indent normalization
 *
 * Revision 1.1.1.1  1997/03/03 03:45:00  nop
 * LambdaMOO 1.8.0p5
 *
 * Revision 2.2  1996/04/08  01:06:25  pavel
 * Added `set_log_file()' entry point.  Made logging print undated messages to
 * stderr if they arrive before a log file has been set.  Release 1.8.0p3.
 *
 * Revision 2.1  1996/02/08  07:00:50  pavel
 * Renamed err/logf() to errlog/oklog().  Cleaned up bf_server_log() code.
 * Updated copyright notice for 1996.  Release 1.8.0beta1.
 *
 * Revision 2.0  1995/11/30  04:26:56  pavel
 * New baseline version, corresponding to release 1.8.0alpha1.
 *
 * Revision 1.10  1992/10/23  23:03:47  pavel
 * Added copyright notice.
 *
 * Revision 1.9  1992/10/21  03:02:35  pavel
 * Converted to use new automatic configuration system.
 *
 * Revision 1.8  1992/09/22  22:48:15  pavel
 * Added missing #include of "config.h".
 *
 * Revision 1.7  1992/09/08  22:04:45  pjames
 * changed `register_bf_log()' to `register_log()'
 *
 * Revision 1.6  1992/08/20  17:27:13  pavel
 * Added the prefix `> ' to all log messages generated by the server_log()
 * built-in function, so that they can be distinguished from server-generated
 * messages.
 *
 * Revision 1.5  1992/08/14  00:23:37  pavel
 * Commented text after #endif.
 *
 * Revision 1.4  1992/08/11  17:27:58  pjames
 * Changed server_log to return 0 instead of 1.
 *
 * Revision 1.3  1992/08/10  17:14:00  pjames
 * Updated #includes.  Added bf_server_log to write messages to server
 * log output.   Added registration procedure.
 *
 * Revision 1.2  1992/07/21  00:04:15  pavel
 * Added rcsid_<filename-root> declaration to hold the RCS ident. string.
 *
 * Revision 1.1  1992/07/20  23:23:12  pavel
 * Initial RCS-controlled version.
 */
