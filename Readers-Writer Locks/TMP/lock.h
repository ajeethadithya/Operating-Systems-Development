#ifndef _LOCK_H_
#define _LOCK_H_

//#define DELETED 	//already defined in kernel.h
#define READ 0
#define WRITE 1
#define NLOCKS 50	//total number of locks

#define LFREE   '\01'           /* this semaphore is free               */
#define LUSED   '\02'           /* this semaphore is used               */


//similar to sentry from sem.h
struct lentry  {                /* lock table entry                     */
        char    lstate;         /* the state LFREE or LUSED             */
        int     lcnt;           /* not used	                        */
        int     lqhead;         /* q index of head of list              */
        int     lqtail;         /* q index of tail of list              */
	int 	ltype;		/* lock held by reader or writer	*/
	int	lprio;		/* max priority process in the queue	*/
	int	lproctab[NPROC];/* processes currently holding the lock	*/
};
extern  struct  lentry  locktab[];
extern  int     nextlock;

//#define isbadsem(s)     (s<0 || s>=NSEM)




#endif
