#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>


LOCAL int getlock();

int lcreate()
{
        STATWORD ps;
        int     locknum;

        disable(ps);
        if ((locknum=getlock())==SYSERR ) {
                restore(ps);
                return(SYSERR);
        }

	/*int i;
	for(i = 0; i < NLOCKS; i++)
	 kprintf("\nlstate %d : %d",i,locktab[i].lstate);
	*/
        
        /* sqhead and sqtail were initialized at system startup */
        restore(ps);
        return(locknum);
}


LOCAL int getlock()
{
        int     lockdescriptor;
        int     i;

        for (i=0 ; i<NLOCKS ; i++) {

		
                lockdescriptor=nextlock--;
                if (nextlock < 0)
                        nextlock = NLOCKS-1;
                if (locktab[lockdescriptor].lstate==LFREE) {
                        locktab[lockdescriptor].lstate = LUSED;
                        return(lockdescriptor);
                }
        }
        return(SYSERR);
}


