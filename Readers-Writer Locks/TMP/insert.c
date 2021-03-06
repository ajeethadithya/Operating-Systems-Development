/* insert.c  -  insert */

//#include <conf.h>
//#include <kernel.h>
//#include <q.h>


#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

unsigned long ctr1000;


/*------------------------------------------------------------------------
 * insert.c  --  insert an process into a q list in key order
 *------------------------------------------------------------------------
 */
int insert(int proc, int head, int key)
{
	int	next;			/* runs through list		*/
	int	prev;

	next = q[head].qnext;
	while (q[next].qkey < key)	/* tail has maxint as key	*/
		next = q[next].qnext;
	q[proc].qnext = next;
	q[proc].qprev = prev = q[next].qprev;
	q[proc].qkey  = key;
	q[prev].qnext = proc;
	q[next].qprev = proc;
	return(OK);
}


int linsert(int proc,int head, int key, int type) 
{
        int     next;                   /* runs through list            */
        int     prev;

        next = q[head].qnext;
        while (q[next].qkey < key)      /* tail has maxint as key       */
                next = q[next].qnext;
        q[proc].qnext = next;
        q[proc].qprev = prev = q[next].qprev;
        q[proc].qkey  = key;

	//added for PA3
	q[proc].waittype = type;
	q[proc].milliseconds = ctr1000;
	//q[proc].waitprio = prio;

        q[prev].qnext = proc;
        q[next].qprev = proc;
        return(OK);
}
