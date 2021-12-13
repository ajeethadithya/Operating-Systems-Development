#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

	/* requests a new mapping of npages with ID map_id */
	//kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	int flag = 0;
	//kprintf("BSID:%d",bs_id);
	if(bs_id < 0 || bs_id >=8 || npages <= 0 || npages > 256 || bsm_tab[bs_id].bs_private == 1) {
		flag = 1;
	}

	else if(bsm_tab[bs_id].bs_status == BSM_MAPPED) {
		npages = bsm_tab[bs_id].bs_npages; 
	}

	else {
		bsm_tab[bs_id].bs_status = BSM_MAPPED;
		bsm_tab[bs_id].bs_pid = currpid;
		//bsm_tab[bs_id].bs_npages = npages;
		
	}
	if(flag == 0) {
		restore(ps);
		return npages;
	}
	else {
		restore(ps);
		return SYSERR;
	}

}


