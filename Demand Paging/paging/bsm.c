/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	STATWORD ps;
	disable(ps);
	int i = 0;
	for(i = 0; i < 8; i++) {
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = 4096;
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_private = 0;		
	}
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD ps;
	disable(ps);
	int i = 0;
	for(i = 0; i < 8; i++) {
		if(bsm_tab[i].bs_status == BSM_UNMAPPED) {
			*avail = i;
			restore(ps);
			return OK;
		}
	}
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{

	STATWORD ps;
	disable(ps);
	bsm_tab[i].bs_status = BSM_UNMAPPED;
	bsm_tab[i].bs_pid = -1;
	bsm_tab[i].bs_vpno = 4096;
	bsm_tab[i].bs_npages = 0;
	bsm_tab[i].bs_private = 0;
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	STATWORD ps;
	disable(ps);
	int i = 0;
	for(i = 0; i < 8; i++) {
		if(bsm_tab[i].bs_pid == pid) {
		*store = i;
		unsigned int base = bsm_tab[i].bs_vpno;
		unsigned int offset = vaddr / NBPG;
		*pageth = offset - base;
		restore(ps);
		return OK;
		}	
	}
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	STATWORD ps;
	disable(ps);
	
	int flag = 0;
      	if(source < 0 || source >= 8 || npages <= 0 || npages > 256 || vpno < 4096) {
                flag = 1;
        }
	if(flag == 1) {
		restore(ps);
		return SYSERR;
	}
	if(bsm_tab[source].bs_pid == pid) {	
		bsm_tab[source].bs_status = BSM_MAPPED;
		bsm_tab[source].bs_pid = pid;
		bsm_tab[source].bs_vpno = vpno;
		bsm_tab[source].bs_npages = npages;
	
		proctab[currpid].store = source;
		proctab[currpid].vhpno = vpno;
		flag = 1;	
	}
	
	if(flag == 1) {
		restore(ps);
		return OK;
	}
	restore(ps);
	return SYSERR;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)	//incomplete
{
	STATWORD ps;
	disable(ps);
	int i = 0;
	int store, pageth;
	//find the corresponding backing store number and page number
	bsm_lookup(pid, vpno, &store, &pageth);
	
	//write the page to the BS
	for(i = 0; i < NFRAMES; i++) {
		if(frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE) {
			int src = (NFRAMES + i) * NBPG;
			write_bs(src, store, pageth);
			bsm_tab[store].bs_status = BSM_UNMAPPED;
			bsm_tab[store].bs_pid = -1;
			bsm_tab[store].bs_vpno = 4096;
			bsm_tab[store].bs_npages = 0;
			bsm_tab[store].bs_private = 0;						
			break;	
		}
	}
	restore(ps);
	return OK;	
}

