/*	$NetBSD: db_disasm.c,v 1.11 1997/10/14 10:18:02 mark Exp $	*/

/*
 * Copyright (c) 1996 Mark Brinicombe.
 * Copyright (c) 1996 Brini.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Brini.
 * 4. The name of the company nor the name of the author may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY BRINI ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL BRINI OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * RiscBSD kernel project
 *
 * db_disasm.c
 *
 * Kernel disassembler
 *
 * Created      : 10/02/96
 *
 * Structured after the sparc/sparc/db_disasm.c by David S. Miller &
 * Paul Kranenburg
 *
 * This code is not complete. Not all instructions are disassembled.
 * Current LDF, STF and CDT are not supported but these are low priority
 * as FP is not used in the kernel.
 */

#include <sys/param.h>
#include <machine/db_machdep.h>
#include <ddb/db_sym.h>
#include <ddb/db_output.h>
#include <ddb/db_access.h>

#include <arm32/arm32/disassem.h>

/* Glue code to interface db_disasm to the generic ARM disassembler */

static u_int
db_disasm_read_word(address)
	u_int address;
{
	return(db_get_value(address, 4, 0));
}

static void
db_disasm_printaddr(address)
	u_int address;
{
	db_printsym((db_addr_t)address, DB_STGY_ANY);
}

vm_offset_t
db_disasm(loc, altfmt)
	vm_offset_t loc;
	boolean_t altfmt;
{
	disasm_interface_t di;

	di.di_readword = db_disasm_read_word;
	di.di_printaddr = db_disasm_printaddr;
	di.di_printf = db_printf;

	return(disasm(&di, loc, altfmt));
}

/* End of db_disasm.c */
