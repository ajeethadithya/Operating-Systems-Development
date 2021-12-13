/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
	//kprintf("xmmap - to be implemented!\n");
	STATWORD ps;
	disable(ps);
	int flag = 0;
	if(bsm_map(currpid, virtpage, source, npages) == SYSERR) {

		restore(ps);
		return SYSERR;
	}

	restore(ps);
	return OK;
	
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
	//kprintf("To be implemented!");
	STATWORD ps;
	disable(ps);
	
	int flag = 0;
	if(virtpage < 4096) {
		flag = 1;
	}
	else {
		bsm_unmap(currpid, virtpage, 0);
	}
	if(flag == 0) {
		restore(ps);
		return OK;
	}
	else {
		restore(ps);
		return SYSERR;
	}
}
