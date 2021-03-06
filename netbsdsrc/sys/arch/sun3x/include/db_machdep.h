/*	$NetBSD: db_machdep.h,v 1.1.1.1 1997/01/14 20:57:04 gwr Exp $	*/

/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#ifndef	_SUN3X_DB_MACHDEP_H_
#define	_SUN3X_DB_MACHDEP_H_

/*
 * Machine-dependent defines for the DDB kernel debugger.
 */

#include <m68k/db_machdep.h>

/* This enables some code in db_command.c */
#define DB_MACHINE_COMMANDS

/* XXX - Put this in db_extern.h instead? -gwr */
extern label_t	*db_recover;	/* db_command.c */

extern char	*esym;	/* end of symbols */
void ddb_init __P((void));
void db_machine_init __P((void));

/* These are in db_memrw.c */
extern void db_read_bytes  __P((vm_offset_t addr, size_t size, char *data));
extern void db_write_bytes __P((vm_offset_t addr, size_t size, char *data));

#endif	/* _SUN3X_DB_MACHDEP_H_ */
