/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>


/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);


	//added for PA3

	int i,tempflag = 0;

	//release all the locks held by the process
	for(i = 0; i < NLOCKS; i++) {
		if(pptr->heldlocks[i] == 1) {
			tempflag = 1;
			releaseall(1, i); // call releaseall(numlocks, ldes1) to release the lock;
		}
	}


	struct lentry *lptr;

	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;
			//if the process was waiting in a queue, priorities need to be updated
			int lockdescriptor = pptr->lockid;	
			
			if(lockdescriptor != -1) {
				pptr->pinh = 0;	//reset priority to original value
				pptr->lockid = -1;
				pptr->lwaitret = DELETED;
				dequeue(pid);	
				lptr = &locktab[lockdescriptor];
                        	lptr->lprio = getmaxlprio(lockdescriptor);
				
				struct pentry *tempptr;
				for( i = 0; i < NPROC; i++) {

					if(lptr->lproctab[i] == 1) {

						tempptr = &proctab[i];
						

						int newmaxprioinvariant = maxpriorityinvariant(i);
						if(newmaxprioinvariant > tempptr->pprio)
                				  
                                			tempptr->pinh = newmaxprioinvariant;
                                       		 else
                                                        tempptr->pinh = 0;	
						
					}
				}			
				//kprintf("\n\nMAX LPRIO VALUE AFTER KILL: %d", lptr->lprio);
				priorityinheritance(lockdescriptor);		
			}
			



	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}


	if(tempflag == 1) {
	
		resched();
	}

	restore(ps);
	return(OK);
}
