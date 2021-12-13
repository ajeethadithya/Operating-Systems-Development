/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{

	//kprintf("To be implemented!\n");
	
	STATWORD ps;
	disable(ps);
		
	pd_t *pd_entry;
        pt_t *pt_entry;
        virt_addr_t *vpointer;
        unsigned long vaddress, pdbr;
        unsigned int pd_offset, pt_offset, pt_address;
        int flag = 0;
	int frame = 0;
	
	pdbr = proctab[currpid].pdbr;
        vaddress = read_cr2(); //cr2 register contains the faulted page
	//kprintf("\nCR2:%lu",vaddress);
	vpointer = (virt_addr_t*)&vaddress;
	

	pd_offset = vpointer->pd_offset;
        pt_offset = vpointer->pt_offset;
               
	//kprintf("\npd offset = %u",pd_offset);
	//kprintf("\npt offset = %u",pt_offset);

        int pde_offset = pd_offset * sizeof(pd_t);
        pd_entry = pdbr + pde_offset;
	
        int pte_base = pd_entry->pd_base * NBPG;
        int pte_offset = pt_offset * sizeof(pt_t);
        pt_entry = (pt_t*)(pte_base + pte_offset) ;
	
	//if PD entry is not present, then allocate new page
	//kprintf("\npd_res:%d",pd_entry->pd_pres);
	if(pd_entry->pd_pres == 0) {
	 	get_frm(&frame);
		
		//mapping the new frame
		frm_tab[frame].fr_status = FRM_MAPPED;
		frm_tab[frame].fr_pid = currpid;
		frm_tab[frame].fr_type = FR_TBL;
		
	 	pt_address = NBPG * (FRAME0 + frame);
	 	pt_entry = (pt_t*)pt_address;
		
		//initialize PT entries
		int i;
		for(i = 0; i < NFRAMES; i++) {
			pt_entry->pt_pres = 0;
        	        pt_entry->pt_write = 0;
        	        pt_entry->pt_user = 0;
        	       	pt_entry->pt_pwt = 0;
        	        pt_entry->pt_pcd = 0;
        	        pt_entry->pt_acc = 0;
        	        pt_entry->pt_dirty = 0;
        	        pt_entry->pt_mbz = 0;
        	        pt_entry->pt_global = 0;
        	        pt_entry->pt_avail = 0;
        	        pt_entry->pt_base = 0;
        	        pt_entry++;	
		}
	
		//initialize PD entry
		pd_entry->pd_pres = 1;
		pd_entry->pd_write = 1;
		pd_entry->pd_user = 0;
		pd_entry->pd_pwt = 0;
		pd_entry->pd_pcd = 0;
		pd_entry->pd_acc = 0;
		pd_entry->pd_mbz = 0;
		pd_entry->pd_fmb = 0;
		pd_entry->pd_global = 0; 
		pd_entry->pd_avail = 0;
		pd_entry->pd_base = FRAME0 +frame;


	}
	
	pte_base = pd_entry->pd_base * NBPG;
        pte_offset = pt_offset * sizeof(pt_t);
        pt_entry = (pt_t*)(pte_base + pte_offset);	
	
	//if PT entry is not present, then allocate new page
	if(pt_entry->pt_pres == 0) {
		
		get_frm(&frame);
		//mapping the new frame
		frm_tab[frame].fr_status = FRM_MAPPED;
		frm_tab[frame].fr_pid = currpid;
		frm_tab[frame].fr_vpno = vaddress / NBPG;
		frm_tab[frame].fr_type = FR_PAGE;
		//increment the ref cnt of the starting frame
		int index = pd_entry->pd_base - FRAME0;
		frm_tab[index].fr_refcnt += 1;
		
		pt_entry->pt_pres = 1;
		pt_entry->pt_write = 1;
		pt_entry->pt_base = FRAME0 + frame; //base + offset		
		//kprintf("\nframe:%d",frame);
		//insert frame into the page replacement queue
		page_replacement_queue_push(frame);		
		
		int store, pageth;
		unsigned int dst;		
		bsm_lookup(currpid, vaddress, &store, &pageth);
		dst = (FRAME0 + frame) * NBPG;	
		read_bs((char*)dst, store, pageth);
		
	}

	write_cr3(pdbr);
	restore(ps);
	return OK;
}


