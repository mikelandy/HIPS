/* UTHREAD . H
#
%	Copyright (c)	Jin, Guojun -	All rights reserved
%
% Simple Macros for compromizing different thread or light-weight-process
% For more complicatedly and completely unique u_thread_*() routines, use
% uthread.c CCS-ECL library.
%
% Warning:
%	Mutex is not available for non-thread programming at this time!
%	use u_sema_*() for them.
%
% Author:	Jin Guojun	1/1/94
*/

#ifndef	U_THREAD_H
#define	U_THREAD_H

#define	u_fork(pid_p, subr, retv)	retv = subr
/* typedef type functype() is legal for funcdef, but not for casting	*/
typedef	void*	(*u_start_routine_t)();	/* to handle different systems	*/
typedef	void	(*u_start_routine_noret)();	/* for casting only!	*/


#ifdef	USE_THREAD

#include <thread.h>
#define	u_thread_t	thread_t
#define	UU_THREAD
	/* flags for thread creation	*/
#define	U_BOUND_THREAD		THR_BOUND
#define	U_DAEMON_THREAD		THR_DAEMON
#define	U_DETACHED_THREAD	THR_DETACHED
#define	U_SUSPENDED_THREAD	THR_SUSPENDED
	/* create a thread with a default stack	and 1 argument	*/
#define	u_thread_create(thr_p, flags, subr, arg1)	\
	thr_create(NULL, No, (u_start_routine_t) subr, arg1, flags, thr_p)
#define	u_thread_join(thr_id, status_p)	thr_join(thr_id, NULL, status_p)
#define	u_thread_kill(tid, sig)	thr_kill(tid, sig)
#define	u_thread_yield(tid)	thr_yield()
#define	u_thread_exit(status_p)	thr_exit(status_p)
#define	u_thread_continue(tid)	thr_continue(tid)
#define	u_thread_suspend(tid)	thr_suspend(tid)
#define	u_thread_self()		thr_self()
	/* keys	*/
#define u_key_t	thread_key_t
#define u_key_init(kp, destructor)	thr_keycreate(kp, destructor)
#define u_key_get(kp, vptr)	thr_getspecific(kp, (void *)&vptr)
#define u_key_set(kp, vp)	thr_setspecific(kp, vp)

#define	U_SYNC_PROCESS	USYNC_PROCESS
#define	U_SYNC_THREAD	USYNC_THREAD
#define	U_SYNC_DEFAULT	U_SYNC_THREAD
#define	u_cond_t	cond_t
#define	u_cond_init(cvp, type, arg)	cond_init(cvp, type, arg)
#define	u_cond_destroy(cvp)		cond_destroy(cvp)
#define	u_cond_wait(cvp, mp)		cond_wait(cvp, mp)
#define	u_cond_timedwait(cvp, mp, at)	cond_timedwait(cvp, mp, at)
#define	u_cond_signal(cvp)	cond_signal(cvp)
#define	u_cond_braodcast(cvp)	cond_braodcast(cvp)

#define	u_mutex_t	mutex_t
#define	u_mutex_init(mp, type, arg)	mutex_init(mp, type, arg)
#define	u_mutex_destroy(mp)		mutex_destroy(mp)
#define	u_mutex_lock(mp)		mutex_lock(mp)
#define	u_mutex_trylock(mp)		mutex_trylock(mp)
#define	u_mutex_unlock(mp)	mutex_unlock(mp)
#define	u_sema_t	sema_t
#define	u_sema_init(sp, count, type, arg)	sema_init(sp, count, type, arg)
#define	u_sema_destroy(sp)	sema_destroy(sp)
#define	u_sema_wait(sp)		sema_wait(sp)
#define	u_sema_trywait(sp)	sema_trywait(sp)
#define	u_sema_post(sp)		sema_post(sp)


#elif	defined	USE_PTHREAD

#include <pthread.h>
#define	u_thread_t	pthread_t
#define	UU_THREAD

	/* flags for thread creation	*/
#define	U_BOUND_THREAD		No
#define	U_DAEMON_THREAD		No
#define	U_DETACHED_THREAD	(1<<2)	/* may change later	*/
#define	U_SUSPENDED_THREAD	No
#define	u_thread_create(thr_p, flags, subr, arg1)	\
	pthread_create(thr_p, pthread_attr_default, subr, arg1);	\
	if (flags & U_DETACHED_THREAD && thr_p)	pthread_detach(thr_p);
#define	u_thread_join(thr_id, status_p)	pthread_join(thr_id, status_p)
#define	u_thread_kill(tid, sig)	pthread_kill(tid, sig)
#define	u_thread_yield(tid)	pthread_yield()
#define	u_thread_exit(status_p)	pthread_exit(status_p)
#define	u_thread_continue(tid)	pthread_continue(tid)
#define	u_thread_suspend(tid)	pthread_suspend(tid)
#define	u_thread_self()		pthread_self()
	/* keys */
#define u_key_t	pthread_key_t
#define u_key_init(kp, destructor)	pthread_key_create(kp, destructor)
#ifdef	__osf__
#define u_key_get(kp, vptr)	(pthread_getspecific(kp, (void *)&vptr))
#else
#define u_key_get(kp, vptr)	\
		(vptr = pthread_getspecific(kp), (vptr != NULL ? 0 : -1))
#endif
#define u_key_set(kp, vp)	pthread_setspecific(kp, vp)

#define	U_SYNC_PROCESS	pthread_condattr_default
#define	U_SYNC_THREAD	pthread_condattr_default
#define	U_SYNC_DEFAULT	No
#define	u_cond_t	pthread_cond_t
#define	u_cond_init(cvp, type, arg)	pthread_cond_init(cvp, type)
#define	u_cond_destroy(cvp)		pthread_cond_destroy(cvp)
#define	u_cond_wait(cvp, mp)		pthread_cond_wait(cvp, mp)
#define	u_cond_timedwait(cvp, mp, at)	pthread_cond_timedwait(cvp, mp, at)
#define	u_cond_signal(cvp)	pthread_cond_signal(cvp)
#define	u_cond_braodcast(cvp)	pthread_cond_braodcast(cvp)

#define	u_mutex_t	pthread_mutex_t
#define	u_mutex_init(mp, type, arg)	pthread_mutex_init(mp, type)
#define	u_mutex_destroy(mp)		pthread_mutex_destroy(mp)
#define	u_mutex_lock(mp)		pthread_mutex_lock(mp)
#define	u_mutex_trylock(mp)		pthread_mutex_trylock(mp)
#define	u_mutex_unlock(mp)	pthread_mutex_unlock(mp)

#ifdef	__osf__
#include <semaphore.h>	/* Currently, it is for OSF1.	*/
#else
#define	pthread_attr_default		No	/* MIT?	*/
#define	pthread_condattr_default	NULL
#endif
#define	u_sema_t	sem_t
#define	u_sema_init(sp, count, type, arg)	sem_init(sp, type, count)
#define	u_sema_destroy(sp)	sem_destroy(sp)
#define	u_sema_wait(sp)		sem_wait(sp)
#define	u_sema_trywait(sp)	sem_trywait(sp)
#define	u_sema_post(sp)		sem_post(sp)


#elif	defined	USE_LWP

#include <lwp/lwp.h>
#define	u_thread_t	thread_t
#	define	CNCT_PRIO	8	/* valid 1 - 10	*/

#define	U_BOUND_THREAD		No
#define	U_DAEMON_THREAD		LWPSERVER
#define	U_DETACHED_THREAD	No
#define	U_SUSPENDED_THREAD	LWPSUSPEND
#define	u_thread_create(thr_p, flags, subr, arg1)	\
	lwp_setstkcache(4096, 2),	pod_setmaxpri(CNCT_PRIO),	\
	lwp_create(thr_p, subr, CNCT_PRIO, flags, lwp_newstk(), 1, arg1)
#define	u_thread_join(thr_id, status_p)	lwp_join(thr_id)
/*	if (status_p)	*(status_p) = lwp_join(thr_id);
	else	lwp_join(thr_id);	*/
#define	u_thread_kill(tid, sig)	lwp_destory(tid)
#define	u_thread_yield(tid)	lwp_yield(tid)
#define	u_thread_exit(status_p)
#define	u_thread_cont(tid)	lwp_resume(tid)
#define	u_thread_suspend(tid)	lwp_suspend(tid)
#define	u_thread_self( )	lwp_self(NULL)	/* could fail	*/


#elif	defined	USE_SPROC	/* IRIX	*/

#include <sys/prctl.h>
#define	u_thread_t	int
#define	U_BOUND_THREAD		No
#define	U_DAEMON_THREAD		PR_SALL
#define	U_DETACHED_THREAD	No
#define	U_SUSPENDED_THREAD	PR_BLOCK

#define	u_thread_create(pid_p, flag, subr, arg1)	\
	{	int	spid, *tp = pid_p;	\
		spid = sproc((u_start_routine_noret)subr, flag | PR_SADDR, arg1);	\
		if (tp)	*tp = spid;	\
	/*	return	spid;	for inline	*/	\
		spid == NULL;		\
	}
#define	u_thread_join(thr_id, status_p)	wait(NULL)	/* interesting	*/
#define	u_thread_kill(tid, sig)	kill(tid, sig)
#define	u_thread_yield(tid)
#define	u_thread_exit(status_p)	exit(0)
#define	u_thread_continue(tid)	unblockproc(tid)
#define	u_thread_suspend(tid)	blockproc(tid)
#define	u_thread_self()		-1	/* not a real thread	*/
	/* no keys */
#include <sys/sema.h>		/* for faking u_mutex	*/


#else

#define	u_thread_t	int	/* faked thread type for both fork and none */

#define	U_BOUND_THREAD		No
#define	U_DAEMON_THREAD		No
#define	U_DETACHED_THREAD	No
#define	U_SUSPENDED_THREAD	No
#define	u_thread_yield(tid)	/*	Thread's Feature	*/
#define	u_thread_exit(status_p)
#define	u_thread_continue(tid)
#define	u_thread_suspend(tid)
#define	u_thread_self()		-1	/* not a real thread	*/

#ifdef	FORK_CONCURRENCY_OK

#define	USE_FORK_ONLY
/*	fork is the one for concurrent control	*/

#define	u_thread_create(pid_p, flag, subr, arg1) {	\
	register u_thread_t	fpid, *fpid_p=pid_p;	\
	switch (fpid = fork())	{	\
	case 0:	subr(arg1);	exit(0);	\
	case -1:prgmerr(fpid, "fork error %d", fpid);	\
		break;	\
	default:	if (fpid_p)	*fpid_p = fpid;	\
	}	}

#undef	u_fork
	/* this can have a return value	*/
#define	u_fork(pid_p, subr, retv)	{	\
	register int	fpid, *fpid_p=pid_p;	\
	switch (fpid = fork())	{		\
	case 0:	retv = subr;	exit(0);	\
	case -1:prgmerr(fpid, "fork error %d", fpid);	\
		break;	\
	default:	if (fpid_p)	*fpid_p = fpid;	\
	}	}
#define	u_thread_join(pid, status_p)	waitpid(pid, status_p, No)
#define	u_thread_kill(pid, sig)	kill(pid, sig)


#else	/*	No Any Thread type Defined.
		Check with ACS or define FORK_CONCURRENCY_OK !
		Otherwise,
	*/
#define	NO_CONCURRENCY

#define	u_thread_create(pid_p, flag, subr, arg1)	subr(arg1)
#define	u_thread_join(d, d2)
#define	u_thread_kill(pid, sig)

#endif	/* no thread	*/

#endif	/* ALL	*/



#ifndef	UU_THREAD

#include <sys/ipc.h>	/* not always	*/
#include <sys/sem.h>	/* traditional semaphore	*/
#ifdef	sema_t
# define	u_sema_t	sema_t
#else
# define	u_sema_t	int
#endif
#define	u_sema_init(sid, count, type, arg)	(sid=semget(arg, count, type))
#define	u_sema_destroy(sid)	semctl(sid, 0, IPC_RMID, 0)
#define	u_sema_wait(sid)	\
	{ struct sembuf	sb;	sb.sem_num=sb.sem_flg=0;	\
		sb.sem_op=-1;	semop(sid, &sb, 1);	\
	}
#define	u_sema_trywait(sid)	u_sema_wait(sid)
#define	u_sema_post(sid)	\
	{ struct sembuf	sb;	sb.sem_num=sb.sem_flg=0;	\
		sb.sem_op=1;	semop(sid, &sb, 1);	\
	}

#define	u_mutex_t	u_sema_t
#define	U_SYNC_THREAD	(IPC_CREAT | 0664)
#define	U_MUTEX_ARGs	IPC_PRIVATE
#define	u_mutex_init(mp, type, arg)	u_sema_init(*(mp), 1, type, arg)
#define	u_mutex_destroy(mp)	u_sema_destroy(*(mp))
#define	u_mutex_lock(mp)	u_sema_wait(*(mp))
#define	u_mutex_trylock(mp)	u_sema_trywait(*(mp))
#define	u_mutex_unlock(mp)	u_sema_post(*(mp))

#define	FAKED_U_COND_
typedef	struct	{	int	pp[2];	}	u_cond_t;
#define	u_cond_init(cvp, type, arg)	pipe((cvp)->pp)
#define	u_cond_destroy(cvp)	close((cvp)->pp[0]) | close((cvp)->pp[1])
#define	u_cond_wait(cvp, mp)	{	int	v;	\
				if (*(mp) >= 0)	u_mutex_unlock(mp);	\
				read((cvp)->pp[0], &v, sizeof(v));	\
				if (*(mp) >= 0)	u_mutex_lock(mp);	}
#define	u_cond_signal(cvp)	write((cvp)->pp[1], &(cvp)->pp[1], sizeof(int))
#define	u_cond_braodcast(cvp)	/* need a clean up handler	*/	\
				write((cvp)->pp[1], (cvp)->pp, sizeof(int)<<1)
	/* the pp[1] is checked at write-end	*/
#define	u_cond_check(cvp, uc)	{	struct	stat	sb;	\
				fstat((cvp)->pp[1], &sb);	\
				uc = sb.st_size / sizeof(int);	}
#define	CV_VALUE(cv)	cv.pp[1]
#define	CV_VALUE0(cv)	cv.pp[0]

#else	/* for CV_checking	*/

#define	U_MUTEX_ARGs	No
#define	CV_VALUE(cv)	cv
#define	CV_VALUE0(cv)	&cv

#endif

typedef	struct	{
	cookie_t	i, o;	/* other structures	*/
	int		id, nfs, eo, flag;
	u_cond_t	cv;
	u_mutex_t	m_b[2];
	int	cv_b[2];	/* 0 = empty; 1 input done; -1 ready output */
	char	*ibuf[2], *obuf[2];
	int	r, c, f, l, m, n, test[2];	/* common reg	*/
} mt_list_t;

#endif	/* U_THREAD_H	*/

