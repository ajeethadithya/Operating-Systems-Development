/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>


/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	//kprintf("To be implemented!\n");
	STATWORD ps;
        disable(ps);

	int bs_number;
	int pid;
	int flag = 0;
 	
	if(hsize <= 0 ) {
		restore(ps);
		return SYSERR;
	}
	
		
	pid = create(procaddr, ssize, priority, name, nargs, args);
	//kprintf("\npid:%D",pid);	
	
	int condition = get_bsm(&bs_number);
	if(condition != SYSERR) {
		bsm_map(pid, 4096, bs_number, hsize);
		bsm_tab[bs_number].bs_private = 1;
		proctab[pid].store = bs_number;
		proctab[pid].vhpno = 4096;
		proctab[pid].vhpnpages = hsize;

		proctab[pid].vmemlist->mnext = 4096 * NBPG; // mnext?
		proctab[pid].vmemlist->mlen = hsize * NBPG;

		
	}

	else { 
		flag = 1;
	}

	if(flag == 0) {
		restore(ps);
		return pid;
	}
	else {
		restore(ps);
		return SYSERR;
	}
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
