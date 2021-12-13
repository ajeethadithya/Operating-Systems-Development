#include <sched.h>
#include <stdio.h>
#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <math.h>
 
int schedclass = 0;

int setschedclass(int scheduleclass) {
	schedclass = scheduleclass;
	}

int getschedclass() {
	return schedclass;
	}
