#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>


#define PROC1_VADDR	0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS	1
int pid1,pid2;
void proc1_test1(char *msg, int lck) {
	char *addr;
	int i;
	//kprintf("\nPid: %s",pid1);	
	get_bs(TEST1_BS, 100);
	//kprintf("\nHELLO");
	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}

	
	addr = (char*) PROC1_VADDR;
	for (i = 0; i < 26; i++) {
	//	kprintf("\nhello");
		*(addr + i * NBPG) = 'A' + i;
	//	kprintf("\n%d",(addr + i * NBPG));
	}
	//kprintf("\nHello");
	sleep(6);
	//kprintf("\nHELLO");
	for (i = 0; i < 26; i++) {
		//kprintf("test");
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	xmunmap(PROC1_VPNO);
	return;
}

void proc1_test2(char *msg, int lck) {
	int *x;
	//kprintf("\nPSTATE: &c",proctab[pid1].pstate);
	kprintf("ready to allocate heap space\n");
	x = vgetmem(1024);
	kprintf("heap allocated at %x\n", x);
	*x = 100;
	*(x + 1) = 200;

	kprintf("heap variable: %d %d\n", *x, *(x + 1));
	vfreemem(x, 1024);
}

void proc1_test3(char *msg, int lck) {

	char *addr;
	int i;

	addr = (char*) 0x0;

	for (i = 0; i < 1024; i++) {
		*(addr + i * NBPG) = 'B';
	}

	for (i = 0; i < 1024; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	return;
}

int main() {
	//int pid1;
	//int pid2;
	
	kprintf("\n1: shared memory\n");
	pid1 = create(proc1_test1, 2000, 20, "proc1_test1", 0, NULL);
	//stacktrace(pid1);
	//kprintf("\npid1:%d", pid1);
	resume(pid1);
	sleep(10);
//	kprintf("\nPSTATE: %c",proctab[pid1].pstate);
	kprintf("\n2: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	
	//kprintf("\npid2:%d",pid2);
	resume(pid1);
	sleep(3);

	kprintf("\n3: Frame test\n");
	pid1 = create(proc1_test3, 2000, 20, "proc1_test3", 0, NULL);
	resume(pid1);
	sleep(3); 
}
