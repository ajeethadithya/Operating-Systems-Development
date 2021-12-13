#ifndef _SCHED_H_
#define _SCHED_H_

#define EXPDISTSCHED            1
#define LINUXSCHED              2

extern int schedclass;

int setschedclass( int scheduleclass);
int getschedclass();

#endif
