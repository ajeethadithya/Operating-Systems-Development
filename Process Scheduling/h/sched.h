#ifndef _SCHED_H_
#define _SCHED_H_

#define EXPDISTSCHED		1
#define LINUXSCHED 		2


int setschedclass( int scheduleclass);
int getschedclass();

#endif
extern int schedclass;
