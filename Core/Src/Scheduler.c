#include "Scheduler.h"

static uint32_t scheduledEvents;

void addSchedulerEvent(uint32_t event) {
	scheduledEvents |= event;
}

void removeSchedulerEvent(uint32_t event) {
	scheduledEvents &= ~(event);
}

uint32_t getScheduledEvents() {
	return scheduledEvents;
}
