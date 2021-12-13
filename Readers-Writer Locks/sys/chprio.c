/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}

	
	pptr->pprio = newprio;

	int newpossibleprio = pptr->pinh;
        if(newprio > newpossibleprio) {
		//since original pprio is greater, process will go with it
                pptr->pinh = newprio;
	}
	
	//since priority was changed, priority inheritance must be maintained
	int transitivitylockid = proctab[pid].lockid;
		
        if(transitivitylockid != -1) {

		struct lentry *lptr;
        	lptr = &locktab[transitivitylockid];
		lptr->lprio = getmaxlprio(transitivitylockid);
        	priorityinheritance(transitivitylockid);	
	}
	restore(ps);
	return(newprio);
}

