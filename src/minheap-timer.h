#ifndef __MINHEAP_TIMER_H__
#define __MINHEAP_TIMER_H__

#include <stdint.h>

#define MINHEAP_TIMER_SIZE_MIN		9


typedef void (*timer_cb)(void *arg);

typedef struct timer_entry {
	uint16_t id;
	uint32_t expire;
	uint8_t shutdown;
	struct timer_entry *next;
	timer_cb callback;
	void *arg;
} timer_entry_t;


typedef struct minheap_timer {
	struct timer_entry **list;
	uint32_t last;
	uint32_t amount;
} mhp_timer_t;


mhp_timer_t* mhp_timer_create(uint32_t size);
void mhp_timer_destroy(mhp_timer_t *timer);
int mhp_timer_push(mhp_timer_t *timer, uint32_t expire, timer_cb cb, void *arg);
int nhp_timer_pop(mhp_timer_t *timer, timer_entry_t** entry);
timer_entry_t* mhp_timer_top(mhp_timer_t *timer);
uint32_t mhp_timer_get_min(mhp_timer_t *timer);


#endif // !__MINHEAP_TIMER_H__