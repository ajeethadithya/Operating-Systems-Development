/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <math.h>
#include <sched.h>
unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	int i, randomnumber, nextproc;
	
	nextproc = q[rdyhead].qnext;

	//exponential distribution scheduler
	if(getschedclass() == EXPDISTSCHED) {
		randomnumber = (int) expdev(0.1);
		//kprintf("\nrandom: %d",(int)randomnumber);
		optr = &proctab[currpid];
		
		//nullproc
		if(q[rdyhead].qnext == q[rdytail].qprev && optr->pstate != PRCURR) {
			nextproc = NULLPROC;
		}

		else if(optr->pstate == PRCURR) {
                        optr->pstate = PRREADY;
                        insert(currpid,rdyhead,optr->pprio);
                
		
	
			while(q[nextproc].qkey < randomnumber) {
				nextproc = q[nextproc].qnext;
			}
		 	
			if(nextproc > NPROC) {
				nextproc = q[rdytail].qprev;
			}
		}
	
		nptr = &proctab[nextproc];
		currpid = nextproc;
		nptr->pstate = PRCURR;          /* mark it currently running    */
		dequeue(nextproc);
		#ifdef	RTCLOCK
			preempt = QUANTUM;	/* reset preemption counter     */
		#endif
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
                                        return OK;	
	}

	//linux-like scheduler
	else if(getschedclass() == LINUXSCHED) {
		//kprintf("\nHello Raleigh");
	
		int maxgoodness = 0, maxprocess = 0;;
		
		optr = &proctab[currpid];
		optr->quantum = preempt;	
		
		if(optr->quantum == 0) {
			optr->goodness = 0;
			optr->counter = 0;	
		}
		else {
			optr->goodness = optr->pprio + preempt;
			optr->counter = preempt;	
		}
		
		//finding max goodness
		for (i = 0; i < NPROC; i++) {   
			if(proctab[i].goodness > maxgoodness && proctab[i].pstate != PRFREE) {
				maxgoodness = proctab[i].goodness;
				maxprocess = i;
			}
		}
		//kprintf("\nMaxgoodness: %d",maxgoodness );

		if(maxgoodness > 0) {
		
				if(optr->pstate == PRCURR && maxgoodness < optr->goodness) {
				maxgoodness = optr->goodness;
				maxprocess = currpid;
				}
	
                        	else if(optr->pstate == PRCURR) {
                        	                optr->pstate = PRREADY;
                        	                insert(currpid,rdyhead,optr->pprio);
                        	}				
				
                        	nptr = &proctab[maxprocess];
                        	currpid = maxprocess;
                        	nptr->pstate = PRCURR;
                        	dequeue(maxprocess);
                        	preempt = nptr->quantum;
	
	                        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	                                        return OK;
			
                }

		//new epoch
		else if(maxgoodness == 0 && (optr->counter == 0 || optr->pstate != PRCURR)) {
			for(i = 0; i < NPROC; i++) {
					proctab[i].quantum = proctab[i].pprio + proctab[i].counter/2;
					proctab[i].goodness = proctab[i].pprio + proctab[i].counter;
					proctab[i].counter = proctab[i].quantum;
			}	
			
			if(optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
				insert(currpid,rdyhead,optr->pprio);
	
				nptr = &proctab[NULLPROC];
				currpid = NULLPROC;
				nptr->pstate = PRCURR;
				dequeue(NULLPROC);
				#ifdef RTCLOCK
                        		preempt = QUANTUM;
                        	#endif
	
	                        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
        	                                return OK;
			
			}
			
			
		}

	}
		

	//xinu scheduler
	else {
		optr= &proctab[currpid];
		
		/* no switch needed if current process priority higher than next*/
	
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
		   (lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
		
		/* force context switch */
	
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}
	
		/* remove highest priority process at end of ready list */
	
		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
		 ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
                                        return OK;		
	}
}
