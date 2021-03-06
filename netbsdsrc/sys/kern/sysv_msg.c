/*	$NetBSD: sysv_msg.c,v 1.21 1996/10/13 02:32:42 christos Exp $	*/

/*
 * Implementation of SVID messages
 *
 * Author:  Daniel Boulet
 *
 * Copyright 1993 Daniel Boulet and RTMX Inc.
 *
 * This system call was implemented by Daniel Boulet under contract from RTMX.
 *
 * Redistribution and use in source forms, with and without modification,
 * are permitted provided that this entire comment appears intact.
 *
 * Redistribution in binary form may occur without any restrictions.
 * Obviously, it would be nice if you gave credit where credit is due
 * but requiring it would be too onerous.
 *
 * This software is provided ``AS IS'' without any warranties of any kind.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/msg.h>
#include <sys/malloc.h>

#include <sys/mount.h>
#include <sys/syscallargs.h>

#define MSG_DEBUG
#undef MSG_DEBUG_OK

#ifdef MSG_DEBUG_OK
#define MSG_PRINTF(a)	printf a
#else
#define MSG_PRINTF(a)
#endif

int nfree_msgmaps;		/* # of free map entries */
short free_msgmaps;		/* head of linked list of free map entries */
struct msg *free_msghdrs;	/* list of free msg headers */

static void msg_freehdr __P((struct msg *));

void
msginit()
{
	register int i;

	/*
	 * msginfo.msgssz should be a power of two for efficiency reasons.
	 * It is also pretty silly if msginfo.msgssz is less than 8
	 * or greater than about 256 so ...
	 */

	i = 8;
	while (i < 1024 && i != msginfo.msgssz)
		i <<= 1;
    	if (i != msginfo.msgssz) {
		MSG_PRINTF(("msginfo.msgssz=%d (0x%x)\n", msginfo.msgssz,
		    msginfo.msgssz));
		panic("msginfo.msgssz not a small power of 2");
	}

	if (msginfo.msgseg > 32767) {
		MSG_PRINTF(("msginfo.msgseg=%d\n", msginfo.msgseg));
		panic("msginfo.msgseg > 32767");
	}

	if (msgmaps == NULL)
		panic("msgmaps is NULL");

	for (i = 0; i < msginfo.msgseg; i++) {
		if (i > 0)
			msgmaps[i-1].next = i;
		msgmaps[i].next = -1;	/* implies entry is available */
	}
	free_msgmaps = 0;
	nfree_msgmaps = msginfo.msgseg;

	if (msghdrs == NULL)
		panic("msghdrs is NULL");

	for (i = 0; i < msginfo.msgtql; i++) {
		msghdrs[i].msg_type = 0;
		if (i > 0)
			msghdrs[i-1].msg_next = &msghdrs[i];
		msghdrs[i].msg_next = NULL;
    	}
	free_msghdrs = &msghdrs[0];

	if (msqids == NULL)
		panic("msqids is NULL");

	for (i = 0; i < msginfo.msgmni; i++) {
		msqids[i].msg_qbytes = 0;	/* implies entry is available */
		msqids[i].msg_perm.seq = 0;	/* reset to a known value */
	}
}

static void
msg_freehdr(msghdr)
	struct msg *msghdr;
{
	while (msghdr->msg_ts > 0) {
		short next;
		if (msghdr->msg_spot < 0 || msghdr->msg_spot >= msginfo.msgseg)
			panic("msghdr->msg_spot out of range");
		next = msgmaps[msghdr->msg_spot].next;
		msgmaps[msghdr->msg_spot].next = free_msgmaps;
		free_msgmaps = msghdr->msg_spot;
		nfree_msgmaps++;
		msghdr->msg_spot = next;
		if (msghdr->msg_ts >= msginfo.msgssz)
			msghdr->msg_ts -= msginfo.msgssz;
		else
			msghdr->msg_ts = 0;
	}
	if (msghdr->msg_spot != -1)
		panic("msghdr->msg_spot != -1");
	msghdr->msg_next = free_msghdrs;
	free_msghdrs = msghdr;
}

int
sys_msgctl(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct sys_msgctl_args /* {
		syscallarg(int) msqid;
		syscallarg(int) cmd;
		syscallarg(struct msqid_ds *) buf;
	} */ *uap = v;
	int msqid = SCARG(uap, msqid);
	int cmd = SCARG(uap, cmd);
	struct msqid_ds *user_msqptr = SCARG(uap, buf);
	struct ucred *cred = p->p_ucred;
	int rval, eval;
	struct msqid_ds msqbuf;
	register struct msqid_ds *msqptr;

	MSG_PRINTF(("call to msgctl(%d, %d, %p)\n", msqid, cmd, user_msqptr));

	msqid = IPCID_TO_IX(msqid);

	if (msqid < 0 || msqid >= msginfo.msgmni) {
		MSG_PRINTF(("msqid (%d) out of range (0<=msqid<%d)\n", msqid,
		    msginfo.msgmni));
		return(EINVAL);
	}

	msqptr = &msqids[msqid];

	if (msqptr->msg_qbytes == 0) {
		MSG_PRINTF(("no such msqid\n"));
		return(EINVAL);
	}
	if (msqptr->msg_perm.seq != IPCID_TO_SEQ(SCARG(uap, msqid))) {
		MSG_PRINTF(("wrong sequence number\n"));
		return(EINVAL);
	}

	eval = 0;
	rval = 0;

	switch (cmd) {

	case IPC_RMID:
	{
		struct msg *msghdr;
		if ((eval = ipcperm(cred, &msqptr->msg_perm, IPC_M)) != 0)
			return(eval);
		/* Free the message headers */
		msghdr = msqptr->msg_first;
		while (msghdr != NULL) {
			struct msg *msghdr_tmp;

			/* Free the segments of each message */
			msqptr->msg_cbytes -= msghdr->msg_ts;
			msqptr->msg_qnum--;
			msghdr_tmp = msghdr;
			msghdr = msghdr->msg_next;
			msg_freehdr(msghdr_tmp);
		}

		if (msqptr->msg_cbytes != 0)
			panic("msg_cbytes is screwed up");
		if (msqptr->msg_qnum != 0)
			panic("msg_qnum is screwed up");

		msqptr->msg_qbytes = 0;	/* Mark it as free */

		wakeup((caddr_t)msqptr);
	}

		break;

	case IPC_SET:
		if ((eval = ipcperm(cred, &msqptr->msg_perm, IPC_M)))
			return(eval);
		if ((eval = copyin(user_msqptr, &msqbuf, sizeof(msqbuf))) != 0)
			return(eval);
		if (msqbuf.msg_qbytes > msqptr->msg_qbytes && cred->cr_uid != 0)
			return(EPERM);
		if (msqbuf.msg_qbytes > msginfo.msgmnb) {
			MSG_PRINTF(("can't increase msg_qbytes beyond %d (truncating)\n",
			    msginfo.msgmnb));
			msqbuf.msg_qbytes = msginfo.msgmnb;	/* silently restrict qbytes to system limit */
		}
		if (msqbuf.msg_qbytes == 0) {
			MSG_PRINTF(("can't reduce msg_qbytes to 0\n"));
			return(EINVAL);		/* non-standard errno! */
		}
		msqptr->msg_perm.uid = msqbuf.msg_perm.uid;	/* change the owner */
		msqptr->msg_perm.gid = msqbuf.msg_perm.gid;	/* change the owner */
		msqptr->msg_perm.mode = (msqptr->msg_perm.mode & ~0777) |
		    (msqbuf.msg_perm.mode & 0777);
		msqptr->msg_qbytes = msqbuf.msg_qbytes;
		msqptr->msg_ctime = time.tv_sec;
		break;

	case IPC_STAT:
		if ((eval = ipcperm(cred, &msqptr->msg_perm, IPC_R))) {
			MSG_PRINTF(("requester doesn't have read access\n"));
			return(eval);
		}
		eval = copyout((caddr_t)msqptr, user_msqptr,
		    sizeof(struct msqid_ds));
		break;

	default:
		MSG_PRINTF(("invalid command %d\n", cmd));
		return(EINVAL);
	}

	if (eval == 0)
		*retval = rval;
	return(eval);
}

int
sys_msgget(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct sys_msgget_args /* {
		syscallarg(key_t) key;
		syscallarg(int) msgflg;
	} */ *uap = v;
	int msqid, eval;
	int key = SCARG(uap, key);
	int msgflg = SCARG(uap, msgflg);
	struct ucred *cred = p->p_ucred;
	register struct msqid_ds *msqptr = NULL;

	MSG_PRINTF(("msgget(0x%x, 0%o)\n", key, msgflg));

	if (key != IPC_PRIVATE) {
		for (msqid = 0; msqid < msginfo.msgmni; msqid++) {
			msqptr = &msqids[msqid];
			if (msqptr->msg_qbytes != 0 &&
			    msqptr->msg_perm.key == key)
				break;
		}
		if (msqid < msginfo.msgmni) {
			MSG_PRINTF(("found public key\n"));
			if ((msgflg & IPC_CREAT) && (msgflg & IPC_EXCL)) {
				MSG_PRINTF(("not exclusive\n"));
				return(EEXIST);
			}
			if ((eval = ipcperm(cred, &msqptr->msg_perm, msgflg & 0700 ))) {
				MSG_PRINTF(("requester doesn't have 0%o access\n",
				    msgflg & 0700));
				return(eval);
			}
			goto found;
		}
	}

	MSG_PRINTF(("need to allocate the msqid_ds\n"));
	if (key == IPC_PRIVATE || (msgflg & IPC_CREAT)) {
		for (msqid = 0; msqid < msginfo.msgmni; msqid++) {
			/*
			 * Look for an unallocated and unlocked msqid_ds.
			 * msqid_ds's can be locked by msgsnd or msgrcv while
			 * they are copying the message in/out.  We can't
			 * re-use the entry until they release it.
			 */
			msqptr = &msqids[msqid];
			if (msqptr->msg_qbytes == 0 &&
			    (msqptr->msg_perm.mode & MSG_LOCKED) == 0)
				break;
		}
		if (msqid == msginfo.msgmni) {
			MSG_PRINTF(("no more msqid_ds's available\n"));
			return(ENOSPC);	
		}
		MSG_PRINTF(("msqid %d is available\n", msqid));
		msqptr->msg_perm.key = key;
		msqptr->msg_perm.cuid = cred->cr_uid;
		msqptr->msg_perm.uid = cred->cr_uid;
		msqptr->msg_perm.cgid = cred->cr_gid;
		msqptr->msg_perm.gid = cred->cr_gid;
		msqptr->msg_perm.mode = (msgflg & 0777);
		/* Make sure that the returned msqid is unique */
		msqptr->msg_perm.seq++;
		msqptr->msg_first = NULL;
		msqptr->msg_last = NULL;
		msqptr->msg_cbytes = 0;
		msqptr->msg_qnum = 0;
		msqptr->msg_qbytes = msginfo.msgmnb;
		msqptr->msg_lspid = 0;
		msqptr->msg_lrpid = 0;
		msqptr->msg_stime = 0;
		msqptr->msg_rtime = 0;
		msqptr->msg_ctime = time.tv_sec;
	} else {
		MSG_PRINTF(("didn't find it and wasn't asked to create it\n"));
		return(ENOENT);
	}

found:
	/* Construct the unique msqid */
	*retval = IXSEQ_TO_IPCID(msqid, msqptr->msg_perm);
	return(0);
}

int
sys_msgsnd(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct sys_msgsnd_args /* {
		syscallarg(int) msqid;
		syscallarg(void *) msgp;
		syscallarg(size_t) msgsz;
		syscallarg(int) msgflg;
	} */ *uap = v;
	int msqid = SCARG(uap, msqid);
	char *user_msgp = SCARG(uap, msgp);
	size_t msgsz = SCARG(uap, msgsz);
	int msgflg = SCARG(uap, msgflg);
	int segs_needed, eval;
	struct ucred *cred = p->p_ucred;
	register struct msqid_ds *msqptr;
	register struct msg *msghdr;
	short next;

	MSG_PRINTF(("call to msgsnd(%d, %p, %d, %d)\n", msqid, user_msgp, msgsz,
	    msgflg));

	msqid = IPCID_TO_IX(msqid);

	if (msqid < 0 || msqid >= msginfo.msgmni) {
		MSG_PRINTF(("msqid (%d) out of range (0<=msqid<%d)\n", msqid,
		    msginfo.msgmni));
		return(EINVAL);
	}

	msqptr = &msqids[msqid];
	if (msqptr->msg_qbytes == 0) {
		MSG_PRINTF(("no such message queue id\n"));
		return(EINVAL);
	}
	if (msqptr->msg_perm.seq != IPCID_TO_SEQ(SCARG(uap, msqid))) {
		MSG_PRINTF(("wrong sequence number\n"));
		return(EINVAL);
	}

	if ((eval = ipcperm(cred, &msqptr->msg_perm, IPC_W))) {
		MSG_PRINTF(("requester doesn't have write access\n"));
		return(eval);
	}

	segs_needed = (msgsz + msginfo.msgssz - 1) / msginfo.msgssz;
	MSG_PRINTF(("msgsz=%d, msgssz=%d, segs_needed=%d\n", msgsz, msginfo.msgssz,
	    segs_needed));
	for (;;) {
		int need_more_resources = 0;

		/*
		 * check msgsz [cannot be negative since it is unsigned]
		 * (inside this loop in case msg_qbytes changes while we sleep)
		 */

		if (msgsz > msqptr->msg_qbytes) {
			MSG_PRINTF(("msgsz > msqptr->msg_qbytes\n"));
			return(EINVAL);
		}

		if (msqptr->msg_perm.mode & MSG_LOCKED) {
			MSG_PRINTF(("msqid is locked\n"));
			need_more_resources = 1;
		}
		if (msgsz + msqptr->msg_cbytes > msqptr->msg_qbytes) {
			MSG_PRINTF(("msgsz + msg_cbytes > msg_qbytes\n"));
			need_more_resources = 1;
		}
		if (segs_needed > nfree_msgmaps) {
			MSG_PRINTF(("segs_needed > nfree_msgmaps\n"));
			need_more_resources = 1;
		}
		if (free_msghdrs == NULL) {
			MSG_PRINTF(("no more msghdrs\n"));
			need_more_resources = 1;
		}

		if (need_more_resources) {
			int we_own_it;

			if ((msgflg & IPC_NOWAIT) != 0) {
				MSG_PRINTF(("need more resources but caller doesn't want to wait\n"));
				return(EAGAIN);
			}

			if ((msqptr->msg_perm.mode & MSG_LOCKED) != 0) {
				MSG_PRINTF(("we don't own the msqid_ds\n"));
				we_own_it = 0;
			} else {
				/* Force later arrivals to wait for our
				   request */
				MSG_PRINTF(("we own the msqid_ds\n"));
				msqptr->msg_perm.mode |= MSG_LOCKED;
				we_own_it = 1;
			}
			MSG_PRINTF(("goodnight\n"));
			eval = tsleep((caddr_t)msqptr, (PZERO - 4) | PCATCH,
			    "msgwait", 0);
			MSG_PRINTF(("good morning, eval=%d\n", eval));
			if (we_own_it)
				msqptr->msg_perm.mode &= ~MSG_LOCKED;
			if (eval != 0) {
				MSG_PRINTF(("msgsnd:  interrupted system call\n"));
				return(EINTR);
			}

			/*
			 * Make sure that the msq queue still exists
			 */

			if (msqptr->msg_qbytes == 0) {
				MSG_PRINTF(("msqid deleted\n"));
				/* The SVID says to return EIDRM. */
#ifdef EIDRM
				return(EIDRM);
#else
				/* Unfortunately, BSD doesn't define that code
				   yet! */
				return(EINVAL);
#endif
			}

		} else {
			MSG_PRINTF(("got all the resources that we need\n"));
			break;
		}
	}

	/*
	 * We have the resources that we need.
	 * Make sure!
	 */

	if (msqptr->msg_perm.mode & MSG_LOCKED)
		panic("msg_perm.mode & MSG_LOCKED");
	if (segs_needed > nfree_msgmaps)
		panic("segs_needed > nfree_msgmaps");
	if (msgsz + msqptr->msg_cbytes > msqptr->msg_qbytes)
		panic("msgsz + msg_cbytes > msg_qbytes");
	if (free_msghdrs == NULL)
		panic("no more msghdrs");

	/*
	 * Re-lock the msqid_ds in case we page-fault when copying in the
	 * message
	 */

	if ((msqptr->msg_perm.mode & MSG_LOCKED) != 0)
		panic("msqid_ds is already locked");
	msqptr->msg_perm.mode |= MSG_LOCKED;

	/*
	 * Allocate a message header
	 */

	msghdr = free_msghdrs;
	free_msghdrs = msghdr->msg_next;
	msghdr->msg_spot = -1;
	msghdr->msg_ts = msgsz;

	/*
	 * Allocate space for the message
	 */

	while (segs_needed > 0) {
		if (nfree_msgmaps <= 0)
			panic("not enough msgmaps");
		if (free_msgmaps == -1)
			panic("nil free_msgmaps");
		next = free_msgmaps;
		if (next <= -1)
			panic("next too low #1");
		if (next >= msginfo.msgseg)
			panic("next out of range #1");
		MSG_PRINTF(("allocating segment %d to message\n", next));
		free_msgmaps = msgmaps[next].next;
		nfree_msgmaps--;
		msgmaps[next].next = msghdr->msg_spot;
		msghdr->msg_spot = next;
		segs_needed--;
	}

	/*
	 * Copy in the message type
	 */

	if ((eval = copyin(user_msgp, &msghdr->msg_type,
	    sizeof(msghdr->msg_type))) != 0) {
		MSG_PRINTF(("error %d copying the message type\n", eval));
		msg_freehdr(msghdr);
		msqptr->msg_perm.mode &= ~MSG_LOCKED;
		wakeup((caddr_t)msqptr);
		return(eval);
	}
	user_msgp += sizeof(msghdr->msg_type);

	/*
	 * Validate the message type
	 */

	if (msghdr->msg_type < 1) {
		msg_freehdr(msghdr);
		msqptr->msg_perm.mode &= ~MSG_LOCKED;
		wakeup((caddr_t)msqptr);
		MSG_PRINTF(("mtype (%d) < 1\n", msghdr->msg_type));
		return(EINVAL);
	}

	/*
	 * Copy in the message body
	 */

	next = msghdr->msg_spot;
	while (msgsz > 0) {
		size_t tlen;
		if (msgsz > msginfo.msgssz)
			tlen = msginfo.msgssz;
		else
			tlen = msgsz;
		if (next <= -1)
			panic("next too low #2");
		if (next >= msginfo.msgseg)
			panic("next out of range #2");
		if ((eval = copyin(user_msgp, &msgpool[next * msginfo.msgssz],
		    tlen)) != 0) {
			MSG_PRINTF(("error %d copying in message segment\n", eval));
			msg_freehdr(msghdr);
			msqptr->msg_perm.mode &= ~MSG_LOCKED;
			wakeup((caddr_t)msqptr);
			return(eval);
		}
		msgsz -= tlen;
		user_msgp += tlen;
		next = msgmaps[next].next;
	}
	if (next != -1)
		panic("didn't use all the msg segments");

	/*
	 * We've got the message.  Unlock the msqid_ds.
	 */

	msqptr->msg_perm.mode &= ~MSG_LOCKED;

	/*
	 * Make sure that the msqid_ds is still allocated.
	 */

	if (msqptr->msg_qbytes == 0) {
		msg_freehdr(msghdr);
		wakeup((caddr_t)msqptr);
		/* The SVID says to return EIDRM. */
#ifdef EIDRM
		return(EIDRM);
#else
		/* Unfortunately, BSD doesn't define that code yet! */
		return(EINVAL);
#endif
	}

	/*
	 * Put the message into the queue
	 */

	if (msqptr->msg_first == NULL) {
		msqptr->msg_first = msghdr;
		msqptr->msg_last = msghdr;
	} else {
		msqptr->msg_last->msg_next = msghdr;
		msqptr->msg_last = msghdr;
	}
	msqptr->msg_last->msg_next = NULL;

	msqptr->msg_cbytes += msghdr->msg_ts;
	msqptr->msg_qnum++;
	msqptr->msg_lspid = p->p_pid;
	msqptr->msg_stime = time.tv_sec;

	wakeup((caddr_t)msqptr);
	*retval = 0;
	return(0);
}

int
sys_msgrcv(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct sys_msgrcv_args /* {
		syscallarg(int) msqid;
		syscallarg(void *) msgp;
		syscallarg(size_t) msgsz;
		syscallarg(long) msgtyp;
		syscallarg(int) msgflg;
	} */ *uap = v;
	int msqid = SCARG(uap, msqid);
	char *user_msgp = SCARG(uap, msgp);
	size_t msgsz = SCARG(uap, msgsz);
	long msgtyp = SCARG(uap, msgtyp);
	int msgflg = SCARG(uap, msgflg);
	size_t len;
	struct ucred *cred = p->p_ucred;
	register struct msqid_ds *msqptr;
	register struct msg *msghdr;
	int eval;
	short next;

	MSG_PRINTF(("call to msgrcv(%d, %p, %d, %ld, %d)\n", msqid, user_msgp,
	    msgsz, msgtyp, msgflg));

	msqid = IPCID_TO_IX(msqid);

	if (msqid < 0 || msqid >= msginfo.msgmni) {
		MSG_PRINTF(("msqid (%d) out of range (0<=msqid<%d)\n", msqid,
		    msginfo.msgmni));
		return(EINVAL);
	}

	msqptr = &msqids[msqid];
	if (msqptr->msg_qbytes == 0) {
		MSG_PRINTF(("no such message queue id\n"));
		return(EINVAL);
	}
	if (msqptr->msg_perm.seq != IPCID_TO_SEQ(SCARG(uap, msqid))) {
		MSG_PRINTF(("wrong sequence number\n"));
		return(EINVAL);
	}

	if ((eval = ipcperm(cred, &msqptr->msg_perm, IPC_R))) {
		MSG_PRINTF(("requester doesn't have read access\n"));
		return(eval);
	}

#if 0
	/* cannot happen, msgsz is unsigned */
	if (msgsz < 0) {
		MSG_PRINTF(("msgsz < 0\n"));
		return(EINVAL);
	}
#endif

	msghdr = NULL;
	while (msghdr == NULL) {
		if (msgtyp == 0) {
			msghdr = msqptr->msg_first;
			if (msghdr != NULL) {
				if (msgsz < msghdr->msg_ts &&
				    (msgflg & MSG_NOERROR) == 0) {
					MSG_PRINTF(("first message on the queue is too big (want %d, got %d)\n",
					    msgsz, msghdr->msg_ts));
					return(E2BIG);
				}
				if (msqptr->msg_first == msqptr->msg_last) {
					msqptr->msg_first = NULL;
					msqptr->msg_last = NULL;
				} else {
					msqptr->msg_first = msghdr->msg_next;
					if (msqptr->msg_first == NULL)
						panic("msg_first/last screwed up #1");
				}
			}
		} else {
			struct msg *previous;
			struct msg **prev;

			for (previous = NULL, prev = &msqptr->msg_first;
			     (msghdr = *prev) != NULL;
			     previous = msghdr, prev = &msghdr->msg_next) {
				/*
				 * Is this message's type an exact match or is
				 * this message's type less than or equal to
				 * the absolute value of a negative msgtyp?
				 * Note that the second half of this test can
				 * NEVER be true if msgtyp is positive since
				 * msg_type is always positive!
				 */

				if (msgtyp == msghdr->msg_type ||
				    msghdr->msg_type <= -msgtyp) {
					MSG_PRINTF(("found message type %d, requested %d\n",
					    msghdr->msg_type, msgtyp));
					if (msgsz < msghdr->msg_ts &&
					    (msgflg & MSG_NOERROR) == 0) {
						MSG_PRINTF(("requested message on the queue is too big (want %d, got %d)\n",
						    msgsz, msghdr->msg_ts));
						return(E2BIG);
					}
					*prev = msghdr->msg_next;
					if (msghdr == msqptr->msg_last) {
						if (previous == NULL) {
							if (prev !=
							    &msqptr->msg_first)
								panic("msg_first/last screwed up #2");
							msqptr->msg_first =
							    NULL;
							msqptr->msg_last =
							    NULL;
						} else {
							if (prev ==
							    &msqptr->msg_first)
								panic("msg_first/last screwed up #3");
							msqptr->msg_last =
							    previous;
						}
					}
					break;
				}
			}
		}

		/*
		 * We've either extracted the msghdr for the appropriate
		 * message or there isn't one.
		 * If there is one then bail out of this loop.
		 */

		if (msghdr != NULL)
			break;

		/*
		 * Hmph!  No message found.  Does the user want to wait?
		 */

		if ((msgflg & IPC_NOWAIT) != 0) {
			MSG_PRINTF(("no appropriate message found (msgtyp=%d)\n",
			    msgtyp));
			/* The SVID says to return ENOMSG. */
#ifdef ENOMSG
			return(ENOMSG);
#else
			/* Unfortunately, BSD doesn't define that code yet! */
			return(EAGAIN);
#endif
		}

		/*
		 * Wait for something to happen
		 */

		MSG_PRINTF(("msgrcv:  goodnight\n"));
		eval = tsleep((caddr_t)msqptr, (PZERO - 4) | PCATCH, "msgwait",
		    0);
		MSG_PRINTF(("msgrcv:  good morning (eval=%d)\n", eval));

		if (eval != 0) {
			MSG_PRINTF(("msgsnd:  interrupted system call\n"));
			return(EINTR);
		}

		/*
		 * Make sure that the msq queue still exists
		 */

		if (msqptr->msg_qbytes == 0 ||
		    msqptr->msg_perm.seq != IPCID_TO_SEQ(SCARG(uap, msqid))) {
			MSG_PRINTF(("msqid deleted\n"));
			/* The SVID says to return EIDRM. */
#ifdef EIDRM
			return(EIDRM);
#else
			/* Unfortunately, BSD doesn't define that code yet! */
			return(EINVAL);
#endif
		}
	}

	/*
	 * Return the message to the user.
	 *
	 * First, do the bookkeeping (before we risk being interrupted).
	 */

	msqptr->msg_cbytes -= msghdr->msg_ts;
	msqptr->msg_qnum--;
	msqptr->msg_lrpid = p->p_pid;
	msqptr->msg_rtime = time.tv_sec;

	/*
	 * Make msgsz the actual amount that we'll be returning.
	 * Note that this effectively truncates the message if it is too long
	 * (since msgsz is never increased).
	 */

	MSG_PRINTF(("found a message, msgsz=%d, msg_ts=%d\n", msgsz,
	    msghdr->msg_ts));
	if (msgsz > msghdr->msg_ts)
		msgsz = msghdr->msg_ts;

	/*
	 * Return the type to the user.
	 */

	eval = copyout((caddr_t)&msghdr->msg_type, user_msgp,
	    sizeof(msghdr->msg_type));
	if (eval != 0) {
		MSG_PRINTF(("error (%d) copying out message type\n", eval));
		msg_freehdr(msghdr);
		wakeup((caddr_t)msqptr);
		return(eval);
	}
	user_msgp += sizeof(msghdr->msg_type);

	/*
	 * Return the segments to the user
	 */

	next = msghdr->msg_spot;
	for (len = 0; len < msgsz; len += msginfo.msgssz) {
		size_t tlen;

		if (msgsz > msginfo.msgssz)
			tlen = msginfo.msgssz;
		else
			tlen = msgsz;
		if (next <= -1)
			panic("next too low #3");
		if (next >= msginfo.msgseg)
			panic("next out of range #3");
		eval = copyout((caddr_t)&msgpool[next * msginfo.msgssz],
		    user_msgp, tlen);
		if (eval != 0) {
			MSG_PRINTF(("error (%d) copying out message segment\n",
			    eval));
			msg_freehdr(msghdr);
			wakeup((caddr_t)msqptr);
			return(eval);
		}
		user_msgp += tlen;
		next = msgmaps[next].next;
	}

	/*
	 * Done, return the actual number of bytes copied out.
	 */

	msg_freehdr(msghdr);
	wakeup((caddr_t)msqptr);
	*retval = msgsz;
	return(0);
}
