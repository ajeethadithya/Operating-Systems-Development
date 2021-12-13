#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

struct lentry locktab[NLOCKS]; //initialize maximum number of locks to NLOCKS

//call in initialize.c

int linit() {

	STATWORD ps;
	disable(ps);

	int i, j;
	struct lentry *lptr;

	nextlock = NLOCKS-1; 

	//initialize all the locks
	for (i = 0; i < NLOCKS; i++) {      				/* initialize semaphores */
        	lptr = &locktab[i];					//initially all locks are free
                lptr->lstate = LFREE;
		lptr->lqtail = 1 + (lptr->lqhead = newqueue());
		lptr->ltype = -1;					//initialize the ltype to be -1
		lptr->lprio = -1; 					//initialize the priority to be -1
		
        }

	//initializing the lproctab
	for(i = 0; i < NLOCKS; i++) {
		
		lptr = &locktab[i];

		for(j = 0; j < NPROC; j++) {
 
		lptr->lproctab[j] = 0;
		}
	}


	restore(ps);
	return(OK);	



}
