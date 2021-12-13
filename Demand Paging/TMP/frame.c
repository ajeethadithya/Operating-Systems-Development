/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern int debug_flag;
extern int page_replace_policy;

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  //kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	int i = 0;
	for(i = 0; i < NFRAMES; i++) {
		frm_tab[i].fr_status = FRM_UNMAPPED; 
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = 0;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = FR_PAGE;
		frm_tab[i].fr_dirty = 0;
	}
	restore(ps);
 	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
	//kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	int i = 0;
	int flag = 0;

	//if a free frame is available, page replacement is not required
	for(i = 0; i < NFRAMES; i++) {
		if(frm_tab[i].fr_status == FRM_UNMAPPED) {
			*avail = i;
			flag = 0;
			break;
		}
	}
	

	if(flag == 0) {
		restore(ps);
		return OK;
	}

	//page replacement is done to obtain free frame	
	int get_frame;
	
	switch(page_replace_policy) {
		
		case SC:   
			//kprintf("\nSC TEST");
			get_frame = SC_page_replacement();
			break;
								
			
		case AGING:
			get_frame = AGING_page_replacement();
			break;

		default:
			flag = 1;
			break;
	}	
	
	free_frm(get_frame);
	//kprintf("\nframe no:%d",get_frame);
	*avail = get_frame;
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

	//kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	
	pd_t *pd_entry;
	pt_t *pt_entry;
	int j, pid, store, pageth;
	int flag = 0;
	unsigned long pdbr, vaddress;
	
	if(i < 0 || i >= NFRAMES) {
		flag = 1;
	}
	
	if(frm_tab[i].fr_type == FR_TBL || frm_tab[i].fr_type == FR_DIR) {
		flag = 1;
	}
	if(flag == 1) {
		restore(ps);
		return SYSERR;
	}
	//after all checks done and the frame is legal	
	pid = frm_tab[i].fr_pid;
	int vheapno = proctab[pid].vhpno;
	pdbr = proctab[pid].pdbr;
	store = proctab[pid].store;
	pageth = frm_tab[i].fr_vpno - vheapno;
		
	virt_addr_t *vpointer;

		
	//finding virtual address inorder to calculate the PD and PT offsets
	unsigned int pd_offset, pt_offset;
	vaddress = frm_tab[i].fr_vpno;	
	pt_offset = vaddress & 1023; //Since vaddress starts from 4096 and beyond, we need offset in terms of 0 to 1023 	
	pd_offset = vaddress / NFRAMES;
	
	int pde_offset = pd_offset * sizeof(pd_t);
	pd_entry = pdbr + pde_offset; 

	int pte_base = pd_entry->pd_base * NBPG;
	int pte_offset = pt_offset * sizeof(pt_t);
	pt_entry = pte_base + pte_offset;  
	
	write_bs(NBPG * (FRAME0 + i), store, pageth); // write back the page
	pt_entry->pt_pres = 0; // since page is not present anymore
	
	int frm_index = pd_entry->pd_base - FRAME0;
	frm_tab[frm_index].fr_refcnt--;
	int ref_cnt = frm_tab[frm_index].fr_refcnt;
	
	//if ref_cnt == 0 then remove page directory entry
	if(ref_cnt == 0) {
		frm_tab[frm_index].fr_status = FRM_UNMAPPED;
		frm_tab[frm_index].fr_pid = -1;
		frm_tab[frm_index].fr_vpno = 4096;
		frm_tab[frm_index].fr_type = FR_PAGE;
		pd_entry->pd_pres = 0; // since page directory entry is not present anymore
	}

	if(flag == 1) {
		restore(ps);
		return SYSERR;
	}
restore(ps);
return OK;
}	

//SC page replacement
int SC_page_replacement_policy() {

	STATWORD ps;
	disable(ps);
	//kprintf("helloword");	
	int frame = 0;
	int current_frame = qhead; //current position starts from the head of the queue
	int prev_frame = -1;
	pd_t *pd_entry;
	pt_t *pt_entry;
	virt_addr_t *vpointer;
	unsigned long vaddress, pdbr;
	unsigned int pd_offset, pt_offset;
	int flag = 0;

	//traversing through the queue to find the frame to replace
	while (current_frame != -1) {
		
		pdbr = proctab[currpid].pdbr;	
		vaddress = frm_tab[current_frame].fr_vpno;   //check
		vpointer = (virt_addr_t*)&vaddress;
		pd_offset = vpointer->pd_offset;
		pt_offset - vpointer->pt_offset;	
		frame = current_frame;
		
		int pde_offset = pd_offset * sizeof(pd_t);
        	pd_entry = pdbr + pde_offset;
	
        	int pte_base = pd_entry->pd_base * NBPG;
        	int pte_offset = pt_offset * sizeof(pt_t);
        	pt_entry = (pt_t*)(pte_base + pte_offset);		
		
		//if reference bit is set, reference bit is cleared and second chance is given for the frame			
		if(pt_entry->pt_acc == 1) {
			pt_entry->pt_acc = 0;
		}
		
		//if reference bit is 0, then the page is chosen for page replacement
		else {
		
			//if the chosen frame is the head of the queue
			if(prev_frame == -1) {
				//making the next frame the head of the queue
				qhead = pr_tab[current_frame].fr_next;
				pr_tab[current_frame].fr_next = -1;
				if(debug_flag == 1) {
					kprintf("\nChosen frame to be removed: %d", frame);
				}
				flag = 1;
			}
	
			//if the chosen frame is not the head of the queue
			else {
				pr_tab[prev_frame].fr_next = pr_tab[current_frame].fr_next;
				pr_tab[current_frame].fr_next == -1;
				
				if(debug_flag == 1) {
                                        kprintf("\nChosen frame to be removed: %d", frame);
                                }
                                flag = 1;					
			}
			//if frame has been found
			if(flag == 1) {
				restore(ps);
				return frame;
			}			
		}
		//change previous and current frame for the next loop iteration
		prev_frame = current_frame;
		current_frame = pr_tab[current_frame].fr_next;
	}
	current_frame = qhead;
	qhead = pr_tab[qhead].fr_next;	
	pr_tab[current_frame].fr_next = -1;
	
	if(debug_flag == 1) {
        	kprintf("\nChosen frame to be removed: %d", frame);
        }
	
	restore(ps);
	return frame;
}

//AGING page replacement
int AGING_page_replacement() {

	STATWORD ps;
	disable(ps);
	
	int frame = 0;
	int current_frame = 0; //current position starts from the head of the queue
	int prev_frame = -1;
	pd_t *pd_entry;
	pt_t *pt_entry;
	virt_addr_t *vpointer;
	unsigned long vaddress, pdbr;
	unsigned int pd_offset, pt_offset;
	int flag = 0;
	int current_prev = 0;
	
	//traversing through the queue to find the frame to replace
	while (current_frame != -1) {	
		
		pr_tab[current_frame].fr_age = pr_tab[current_frame].fr_age >> 1; //decreasing by half
		
		pdbr = proctab[currpid].pdbr;
                vaddress = frm_tab[current_frame].fr_vpno;   //check
                vpointer = (virt_addr_t*)&vaddress;
                pd_offset = vpointer->pd_offset;
                pt_offset - vpointer->pt_offset;
                //frame = current_frame;

                int pde_offset = pd_offset * sizeof(pd_t);
                pd_entry = pdbr + pde_offset;

                int pte_base = pd_entry->pd_base * NBPG;
                int pte_offset = pt_offset * sizeof(pt_t);
                pt_entry = (pt_t*)(pte_base + pte_offset);		

		//if the reference bit is set, set it to 0 and increase the age by 128 (max age is 255)
		if(pt_entry->pt_acc == 1) {
			pt_entry->pt_acc = 0;
			pr_tab[current_frame].fr_age = pr_tab[current_frame].fr_age + 128;
			if(pr_tab[current_frame].fr_age > 255) {
				pr_tab[current_frame].fr_age = 255;
			}
		}

		//find the frame with the lowest age
		if(pr_tab[current_frame].fr_age < pr_tab[frame].fr_age) {
			frame = current_frame;
			if(current_frame != qhead) {
				current_prev = prev_frame;
			}
		}
		prev_frame = current_frame;
		current_frame = pr_tab[current_frame].fr_next;		
	}
	
	if(debug_flag == 1) {
                kprintf("\nChosen frame to be removed: %d", frame);
        }
	
	//if the chosen frame is the head of the queue
	if(frame == qhead) {
		current_frame = qhead;
		qhead = pr_tab[qhead].fr_next;
		pr_tab[current_frame].fr_next = -1;
	}
	
	//if the chosen frame is not the head of the queue
	else {
		pr_tab[current_prev].fr_next = pr_tab[frame].fr_next;
		pr_tab[frame].fr_next = -1;		
	}

	restore(ps);
	return frame;
	

}

//insert new frame into page replacement queue
void page_replacement_queue_push(int frame) {

	STATWORD ps;
	disable(ps);
	
	int i, prev_frame, next_frame, current_frame;
	int flag = 0;
	
	//if the page replacement queue is empty
	if(qhead == -1) {
		qhead = frame;
		flag = 1;
	}
	
	if(flag == 1) {
		restore(ps);
		return OK;
	} 

	//if the page replacement queue is not empty and has elements
	
	current_frame = qhead;
	prev_frame = qhead;
	while(current_frame != -1) {
	
		prev_frame = current_frame;
		current_frame = pr_tab[current_frame].fr_next;
		
	}
	current_frame = prev_frame; // the previous frame is the last frame in the queue, and hence copying it to the current frame
	pr_tab[current_frame].fr_next = frame; // assigning the next frame to be FRAME frame
	pr_tab[frame].fr_next = -1; //next frame of FRAME frame is -1 since it is the last element

	restore(ps);
	return OK; 
}	
