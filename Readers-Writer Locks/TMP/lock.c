#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>


unsigned long ctr1000; 


int lock(int ldes1, int type, int priority) {

	STATWORD ps;
	disable(ps);


		
	int i,maxwaitingpriority, tempwaitingpriority, greaterprioritywriter, checkforwriter;
	greaterprioritywriter = 0;
	checkforwriter = 0;
	
	struct pentry *pptr;
	pptr = &proctab[currpid];
	
	struct lentry *lptr;
	lptr = &locktab[ldes1];
	
	//testing
	//lptr->lstate = LFREE;	
	//kprintf("\nlstate: %d\n", lptr->lstate);
	//kprintf("\nltype:  %d\n", lptr->ltype);
	//testing end


	if (ldes1 < 0 || ldes1 > NLOCKS || lptr->lstate==LFREE) {
                restore(ps);
                return(SYSERR);
        }
	
	//when ltype =type = -1, the lock was previously used and is not held by any process now	
	
	if(lptr->ltype == -1) {
		pptr->heldlocks[ldes1] = 1;
	
		lptr->lstate = LUSED;
		lptr->ltype = type;
		//lptr->lprio = -1;	//not needed, since it is already set in ldelete?
		lptr->lproctab[currpid] = 1;
			
	}
	

	//if type == WRITE, put the proc in queue as write's are exclusive
	else if(lptr->lstate == LUSED && lptr->ltype == WRITE) {
	
		pptr->pstate = PRWAIT;		//set the state to wait
		
		pptr->lockid = ldes1;		//set the waiting lock as ldes1
		
		linsert(currpid,lptr->lqhead,priority,type); //insert the process in the lock wait queue;

		lptr->lprio = getmaxlprio(ldes1);	//update the lprio to the max priority of the process in the queue
		
		priorityinheritance(ldes1);		//maintain priority inheritance
		
		pptr->lwaitret = OK;
		resched();

		restore(ps);
		return(OK);
	
	}
	
	else if(lptr->lstate == LUSED && lptr->ltype == READ) {
		
		//there are two possibilites in this scenario
		
		//if the requesting proc type is WRITE and the lock is already held by reader, put the proc in wait queue
		if(type == WRITE) {

			pptr->pstate = PRWAIT;          //set the state to wait

                	pptr->lockid = ldes1;           //set the waiting lock as ldes1

                	linsert(currpid,lptr->lqhead,priority,type); //insert the process in the lock wait queue;

                	lptr->lprio = getmaxlprio(ldes1);       //update the lprio to the max priority of the process in the queue

                	priorityinheritance(ldes1);            //maintain priority inheritance
			
			pptr->lwaitret = OK;
                	resched();

                	restore(ps);
                	return(OK);
		
		}
	
		//if the requsting proc type is READ, give lock if it has sufficient wait priority
		else if(type == READ) {
			
			int current = lptr->lqhead;
			current = q[current].qnext;
			
			while(current != lptr->lqtail) {
						
				tempwaitingpriority = q[current].qkey; 		//qkey is the waiting priority of the process

				if(q[current].waittype == WRITE) {
					checkforwriter = 1;
				}
				if(tempwaitingpriority > priority && q[current].waittype == WRITE) {
					//maxwaitingpriority = tempwaitingpriority;
					//checkforwriter = 1;
					greaterprioritywriter = 1;
				}
			
				current = q[current].qnext;
			}	 						
			

			//if checkforwriter == 0, then no writers exist
			if(checkforwriter == 0) {
				//since no writers exist, we can give the lock

				lptr->lprio = getmaxlprio(ldes1);
				lptr->lproctab[currpid] = 1;
		
				pptr->heldlocks[ldes1] = 1;

				priorityinheritance(ldes1);		
	
			}			

			//if checkforwriter == 1, then writers exist
			else {
					
				//if the new process has a priority no less than the writer with the max priority waiting in the queue
				if(greaterprioritywriter == 0) {

					
					lptr->lprio = getmaxlprio(ldes1);
                                	lptr->lproctab[currpid] = 1;

                                	pptr->heldlocks[ldes1] = 1;

                                	priorityinheritance(ldes1);	//maintain priority inheritance
				}
	
				//if the new process does not have sufficient priority, put it in wait queue	
				else {
					
					proctab[currpid].pstate = PRWAIT;	
					pptr->lockid = ldes1;           //set the waiting lock as ldes1

                        		linsert(currpid,lptr->lqhead,priority,type); //insert the process in the lock wait queue;

                        		lptr->lprio = getmaxlprio(ldes1);       //update the lprio to the max priority of the process in the queue

                        		priorityinheritance(ldes1);            //maintain priority inheritance

					pptr->lwaitret = OK;
                     			resched();
					
                        		restore(ps);
                        		return(OK);					
					
				}
			
			}			
	
		}

	}

	//kprintf("\nLDES1: %d\n", ldes1);
	/*if(locktab[ldes1].lproctab[currpid] == 1)
			kprintf("\nSuccess!\n");
	else
		kprintf("\nError!\n");
	*/

	restore(ps);
	return(OK);

}



//get the max lprio from the list of process waiting for lock ldes1
int getmaxlprio(int ldes1) {

	int i, maxlprio;
	maxlprio = -1;
	
	struct pentry *pptr;
	
	for(i = 0; i <NPROC; i++) {
		
		//check for each process, if the waiting lock is ldes1
		if(proctab[i].lockid == ldes1) {

			//if process is running with original priorioty, pprio
			if(proctab[i].pinh == 0 && proctab[i].pprio > maxlprio)
				maxlprio = proctab[i].pprio;

			//if process is running with pinh
			else if(proctab[i].pinh != 0 && proctab[i].pinh > maxlprio)
				maxlprio = proctab[i].pinh; 
		}
	}	
return maxlprio;
}


//maintains the priority invariant, Prio(p) = max (Prio(p_i))
int maxpriorityinvariant(int pid) {

	struct pentry *pptr;
	pptr = &proctab[pid];

	int i, j, maxprocessinvariant;
	maxprocessinvariant = -1;

	for(i = 0; i < NLOCKS; i++) {
		
		//for every lock held by pid
		if(pptr->heldlocks[i] == 1) {
		
			if(locktab[i].lprio > maxprocessinvariant)
				maxprocessinvariant = locktab[i].lprio;		
		}
	}
	return maxprocessinvariant;
}


//priority inheritance
int priorityinheritance(int ldes1) {

	struct lentry *lptr;
	lptr = &locktab[ldes1];
	
	int i, maxschedulingpriority, processpriority, maxprocessinvariant, transitivitylockid;
	maxschedulingpriority = -1;
	
	for(i = 0; i <NPROC; i++) {
		//for every process that holds the lock ldes1
		if(lptr->lproctab[i] == 1) {

			//get the process priority that holds the lock ldes1		
			if(proctab[i].pinh == 0)
				processpriority = proctab[i].pprio;
			else
				processpriority = proctab[i].pinh;
			
			//check the max priority invariant
			maxprocessinvariant = maxpriorityinvariant(i);
			if(maxprocessinvariant > processpriority) {

				proctab[i].pinh = maxprocessinvariant;
				transitivitylockid = proctab[i].lockid;

				if(transitivitylockid != -1) 
					priorityinheritance(transitivitylockid);

			}					
		}			
	}
	return(OK); 
}
