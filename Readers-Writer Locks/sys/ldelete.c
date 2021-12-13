#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

int ldelete(int lockdescriptor)
{
        STATWORD ps;
        int     pid;
        struct  lentry  *lptr;

        disable(ps);
        if (lockdescriptor < 0 || lockdescriptor > NLOCKS || locktab[lockdescriptor].lstate==LFREE) {
                restore(ps);
                return(SYSERR);
        }
        lptr = &locktab[lockdescriptor];
        lptr->lstate = LFREE;
	lptr->ltype = -1;
	lptr->lprio = -1;
	
	int i;
	for(i = 0; i < NPROC; i++) {
		lptr->lproctab[i] = 0;
		proctab[i].heldlocks[lockdescriptor] == 0;
	}

        if (nonempty(lptr->lqhead)) {
                while( (pid=getfirst(lptr->lqhead)) != EMPTY)
                  {
                    proctab[pid].lwaitret = DELETED; //CHECK!!!
		    proctab[pid].lockid = -1;
                    ready(pid,RESCHNO);
                  }
                resched();
        }
        restore(ps);
        return(OK);
}               
