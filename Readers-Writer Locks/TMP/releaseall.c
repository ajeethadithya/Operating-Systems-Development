#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>


int releaseall( int numlocks, int ldes1, ...) {

	STATWORD ps;
	disable(ps);
	

	struct lentry *lptr;
	lptr = &locktab[currpid];	

	struct pentry *pptr, *tempptr;
	pptr = &proctab[currpid];

	int i, lockdescriptor, badlockflag, cnext;
	int tempreaderpriority, tempwriterpriority, maxreaderpriority, maxwriterpriority, checkforreader, checkforwriter,greaterpriorityreader,  greaterprioritywriter;
	badlockflag = 0;
	checkforreader = 0;
	checkforwriter = 0;
	greaterpriorityreader = 0;
	greaterprioritywriter = 0;

	int checkifanotherreaderholdslock = 0;


	//int *ldes = &ldes1;
	

	for( i = 0 ; i < numlocks; i++) {
		lockdescriptor = *(&ldes1 + i);	
		lptr = &locktab[lockdescriptor];


		//kprintf("\nlockdescriptor in releaseall: %d",lockdescriptor);
	
		//bad lock descriptor	
		if(lockdescriptor < 0 || lockdescriptor > NLOCKS || locktab[lockdescriptor].lstate==LFREE || lptr->lproctab[currpid] == 0) {
	
                	badlockflag = 1;
			continue; //skip current iteration of the loop due to bad lock
		}
		
		
		//releasing lock
		
		int current = lptr->lqhead;
                current = q[current].qnext;

		
		//temporary solution for maxpriority
		maxwriterpriority = -1000;
		maxreaderpriority = -1000;

		//check if writer exists in queue
                while(current != lptr->lqtail) {

			tempreaderpriority = q[current].qkey;		
                        tempwriterpriority = q[current].qkey;          //qkey is the waiting priority of the process

			//get reader information waiting for lockdescriptor
			if(proctab[current].lockid == lockdescriptor && q[current].waittype == READ)
                                checkforreader = 1;
                        if(proctab[current].lockid == lockdescriptor && tempreaderpriority > maxreaderpriority && q[current].waittype == READ) {
                                greaterpriorityreader = current;
                                maxreaderpriority = tempreaderpriority;
                        }
			

			//get writer information waiting for lockdescriptor
			if(proctab[current].lockid == lockdescriptor && q[current].waittype == WRITE) 
				checkforwriter = 1;
                        if(proctab[current].lockid == lockdescriptor && tempwriterpriority > maxwriterpriority && q[current].waittype == WRITE) {                        	
                                greaterprioritywriter = current;
				maxwriterpriority = tempwriterpriority;
                        }
                                        
                        current = q[current].qnext;
                }

	



		//if queue is empty, do nothing
		if(checkforreader == 0 && checkforwriter == 0) {
			//kprintf("\n\n\ntest r = 0 and w = 0");		
		 	lptr->lproctab[currpid] = 0;

                        pptr->heldlocks[lockdescriptor] = 0;
                        if(pptr->lockid == lockdescriptor)
                                pptr->lockid = -1;

                        lptr->ltype = -1;
                        lptr->lstate = LFREE;
                        lptr->lprio = getmaxlprio(lockdescriptor);


		}
		//if no writer exists, give lock to all readers
		//if the lock was released by one of many readers and no writers exist, we can give all readers in queue the lock
		else if(checkforreader == 1 && checkforwriter == 0) {
			

			//kprintf("\nTest case 3 check");
	 		/*//debugging code start
	 		int debugflag = 0;	
        		current = lptr->lqhead;
        		current = q[current].qnext;
        		
        		while(current != lptr->lqtail) {
        		        kprintf("\nqueue: %d\n",current);
        		        current = q[current].qnext;
        		}

			//debudding code end  */
			
			lptr->lproctab[currpid] = 0;
	
        	       	pptr->heldlocks[lockdescriptor] = 0;
        	        if(pptr->lockid == lockdescriptor)
        	               	pptr->lockid = -1;
			
			lptr->ltype = READ;
			lptr->lstate = LUSED;
			lptr->lprio = -1;	


	
			current = lptr->lqhead;
               		current = q[current].qnext;
			
			//give all the readers in the queue the lock
			while(current != lptr->lqtail) {
			
				//kprintf("\ncurrent value:%d\tnext value:%d",current,q[current].qnext);
				
				//kprintf("\ncurrenttest: %d", current);	
				proctab[current].lockid = -1;
				proctab[current].heldlocks[lockdescriptor] = 1;
				lptr->lproctab[current] = 1;
				lptr->ltype = READ;	

				dequeue(current);	

				/*//testing
			
                        	int currenttest = lptr->lqhead;
                        	currenttest = q[currenttest].qnext;

                        	while(currenttest != lptr->lqtail) {
                        	        debugflag = 1;
                                	currenttest = q[currenttest].qnext;
                        	}
                       		if(debugflag == 0)
                                	kprintf("\nqueue empty!");
				//testing*/
				//kprintf("\npstate:%d\n",proctab[current].pstate);
				//kprintf("\nCurrent value before ready: %d",current);
				//kprintf("\tNext Value before ready: %d", q[current].qnext);
				cnext = q[current].qnext;		
				ready(current, RESCHNO);
				
				//kprintf("\ncurrent value after ready: %d",current);
				//kprintf("\tNext value after ready: %d", q[current].qnext);
				current = cnext;		
			}	

			//kprintf("\ncheck for queue exit!");
		}

		//if only writer is present, give the lock the greatest writer
		else if(checkforreader == 0 && checkforwriter == 1) {
			
			//if the process releasing is WRITER
			if(lptr->ltype == WRITE) {
				
				//releasing the lock            
				lptr->lproctab[currpid] = 0;
				
				pptr->heldlocks[lockdescriptor] = 0;
			       	if(pptr->lockid == lockdescriptor)
					pptr->lockid = -1;
	
				//since no reader present in queue, give the lock to highet priority writer
				proctab[greaterprioritywriter].lockid = -1;
				proctab[greaterprioritywriter].heldlocks[lockdescriptor] = 1;
				lptr->lproctab[greaterprioritywriter] = 1;
				lptr->ltype = WRITE;
				dequeue(greaterprioritywriter);
				ready(greaterprioritywriter, RESCHNO);			
			
			}

			//if the process releasing is READER, then check if any another reader is holding the lock,if so, don't do anything
			else if(lptr->ltype == READ) {
		
				//releasing the lock		
				lptr->lproctab[currpid] = 0;
	
	                        pptr->heldlocks[lockdescriptor] = 0;
	                        if(pptr->lockid == lockdescriptor)
	                                pptr->lockid = -1;
	
	
				//check if another reader holds the lock
				for(i = 0; i < NPROC; i++) {
					if(lptr->lproctab[i] == 1) {
						checkifanotherreaderholdslock++;
						break;
					}
				}
				
				//if lock is not held by another reader give to the highest priority writer		
				if(checkifanotherreaderholdslock == 0) {

					//since no reader present in queue, give the lock to highet priority writer
					proctab[greaterprioritywriter].lockid = -1;
					proctab[greaterprioritywriter].heldlocks[lockdescriptor] = 1;
					lptr->lproctab[greaterprioritywriter] = 1;
					lptr->ltype = WRITE;
					dequeue(greaterprioritywriter);
					ready(greaterprioritywriter, RESCHNO); 							
				}

				//if another reader holds locks, don't do anything
				else {
					//nothing is done, since the lock is held by atleast another one reader
				}	

			}		
			
		}
		
		//if both reader and writer are present
		else if(checkforreader == 1 && checkforwriter == 1) {
			

			//releasing the lock	
			lptr->lproctab[currpid] = 0;

                                pptr->heldlocks[lockdescriptor] = 0;
                                if(pptr->lockid == lockdescriptor)
                                        pptr->lockid = -1;


			//check if another reader holds the lock
			for(i = 0; i < NPROC; i++) {
                                        if(lptr->lproctab[i] == 1) {
                                                checkifanotherreaderholdslock++;
                                                break;
                                        }
                                }
				
			//if lock is not held by another reader, give to MAX(READER,WRITER)          
			if(lptr->ltype == WRITE || (lptr->ltype == READ && checkifanotherreaderholdslock == 0)) {


				//if the writer has the greater priority, give the lock to the writer
				if(maxwriterpriority > maxreaderpriority) {

					proctab[greaterprioritywriter].lockid = -1;
                                        proctab[greaterprioritywriter].heldlocks[lockdescriptor] = 1;
                                        lptr->lproctab[greaterprioritywriter] = 1;
                                        lptr->ltype = WRITE;
                                        dequeue(greaterprioritywriter);
                                        ready(greaterprioritywriter, RESCHNO);

				}
			
				//if the reader has the greater priority, give the lock to all the readers having greater priority the the greatest writer
				else if(maxreaderpriority > maxwriterpriority) {

					int readerprioritycheck;
					current = lptr->lqhead;
		                        current = q[current].qnext;
			
					 while(current != lptr->lqtail) {
						
						readerprioritycheck = q[current].qkey;

						if(proctab[current].lockid == lockdescriptor && readerprioritycheck >= maxwriterpriority) {
                                			proctab[current].lockid = -1;
                                			proctab[current].heldlocks[lockdescriptor] = 1;
                                			lptr->lproctab[current] = 1;
							lptr->ltype = READ;
	                                		dequeue(current);

							cnext = q[current].qnext;
	                                		ready(current, RESCHNO);

                                			current = cnext;
						}
        			         }
	
				}

				else if (maxreaderpriority == maxwriterpriority) {

					unsigned long waittimeofreader = q[greaterpriorityreader].milliseconds;
					unsigned long waittimeofwriter = q[greaterprioritywriter].milliseconds;
					unsigned long timedifference;

					//this implementation is not needed since unsigned long does not store negative values
					if(waittimeofreader > waittimeofwriter)
						timedifference = waittimeofreader - waittimeofwriter;

					else
						timedifference = waittimeofwriter - waittimeofreader;
					
					//if the time difference is within 1 second or if time difference greater than 1 second and writer waited for longer
					if(timedifference <= 1000 || (timedifference > 1000 && waittimeofwriter > waittimeofreader)) {
						proctab[greaterprioritywriter].lockid = -1;
                                        	proctab[greaterprioritywriter].heldlocks[lockdescriptor] = 1;
                                        	lptr->lproctab[greaterprioritywriter] = 1;
                                        	lptr->ltype = WRITE;
                                        	dequeue(greaterprioritywriter);
                                        	ready(greaterprioritywriter, RESCHNO);
					}

					//if the time difference is greater than 1 second and reader has greater waiting time
					else if(timedifference > 1000 && waittimeofreader > waittimeofwriter) {

						int readerprioritycheck;
						current = lptr->lqhead;
                                        	current = q[current].qnext;
	
	                                        while(current != lptr->lqtail) {
	
	                                        	readerprioritycheck = q[current].qkey;
	
	                                                if(proctab[current].lockid == lockdescriptor && readerprioritycheck >= maxwriterpriority) {
	                                                        proctab[current].lockid = -1;
	                                                        proctab[current].heldlocks[lockdescriptor] = 1;
	                                                        lptr->lproctab[current] = 1;
	                                                        lptr->ltype = READ;
	                                                        dequeue(current);
								cnext = q[current].qnext;
	                                                        ready(current, RESCHNO);
	
	                                                        current = cnext;
	                                                }
	                                        }
				 	}
				} 
			}
		}



	lptr->lprio = getmaxlprio(lockdescriptor);
	int newmaxprioinvariant = maxpriorityinvariant(currpid);//proc prio to be reset to the max prio of all the procs in wait queues of all the locks still held by the procc
	
	//since the lock what was released could have been the reason  pinh value was greater than pptr
	if(newmaxprioinvariant > pptr->pprio)
		  
		pptr->pinh = newmaxprioinvariant;
	else
		pptr->pinh = 0;
	
	//for loop end	
	} 
  	
	resched();
	restore(ps);
	if(badlockflag == 0) 
		return(OK);
	else
		return(SYSERR);

}
