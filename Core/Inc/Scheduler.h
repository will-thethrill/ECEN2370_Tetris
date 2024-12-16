#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "stdint.h"

#define MAIN_MENU 	(1 << 0)
#define GAME 		(1 << 1)
#define RESULTS 	(1 << 2)

uint32_t getScheduledEvents();

void addSchedulerEvent(uint32_t event);

void removeSchedulerEvent(uint32_t event);

#endif
