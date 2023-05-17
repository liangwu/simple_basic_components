#include "minheap-timer.h"
#include <stdlib.h>

#define swap_ptr(a, b) 	{void* tmp = a; a = b; b = tmp;}

static inline int timer_entry_greater(const timer_entry_t *a, const timer_entry_t *b) {
	return a->expire > b->expire;
}


static int reserve(mhp_timer_t *timer, uint32_t n) {

	if (n > timer->amount) {
		void *ptr = NULL;
		uint32_t size = timer->amount * 2 > n ? timer->amount * 2 : n ;
		if ((ptr = realloc(timer->list, size)) == NULL) {
			return -1;
		}
		timer->list = ptr;
		timer->amount = size;
	}
	return 0;
}


static void adjust_up(mhp_timer_t *timer, uint32_t pos) {
	uint32_t parent = (pos-1)>>1;
	timer_entry_t **list = timer->list;
	timer_entry_t *tmp = NULL;
	while (pos > 0 && timer_entry_greater(list[parent], list[pos])) {
		swap_ptr(list[parent], list[pos]);
		pos = parent;
		parent = (pos-1)>>1;
	}
}


// TODO: 相同key
static void adjust_down(mhp_timer_t *timer, uint32_t pos) {
	uint32_t lchild = (pos+1)>>1 - 1, rchild = (pos+1)>>1;
	timer_entry_t *entry = timer->list[pos];
	timer_entry_t **list = timer->list;

	uint32_t lower;
	timer_entry_t *tmp = NULL;

	lower = timer_entry_greater(list[lchild], list[rchild]) ? rchild : lchild;
	while (timer_entry_greater(list[pos], list[lower])) {
		swap_ptr(list[pos], list[lower]);
		pos = lower;
		lchild = (pos+1)>>1 - 1;
		rchild = (pos+1)>>1;
		lower = timer_entry_greater(list[lchild], list[rchild]) ? rchild : lchild;
	}
}


mhp_timer_t* mhp_timer_create(uint32_t size) {

	mhp_timer_t *timer = calloc(1, sizeof(mhp_timer_t));
	if (timer == NULL) return NULL;
	size = size > MINHEAP_TIMER_SIZE_MIN ? size : MINHEAP_TIMER_SIZE_MIN;
	timer->amount = size;
	timer->last = 0;

	timer->list = calloc(size, sizeof(timer_entry_t *));
	if (timer->list == NULL) {
		free(timer);
		return NULL;
	}

	return timer;
}


void mhp_timer_destroy(mhp_timer_t *timer) {
	if (timer == NULL) return;

	int i, cnt=0;
	for (i = 0; i < timer->amount; i++) {
		if (timer->list[i] != NULL) {
			free(timer->list[i]);
			timer->list[i] = NULL;
			cnt++;
		}
		if (cnt == timer->last) break;
	}
	if (timer->list != NULL) free(timer->list);

	free(timer);
}



int mhp_timer_push(mhp_timer_t *timer, uint32_t expire, timer_cb cb, void *arg) {
	if (timer == NULL) return -1;

	timer_entry_t *entry = calloc(1, sizeof(timer_entry_t));
	if (entry == NULL) return -1;
	entry->expire = expire;
	entry->callback = cb;
	entry->arg = arg;

	timer_entry_t *cur = timer->list[0];
	if (cur == NULL) {
		timer->list[0] = entry;
		return 0;
	}

	int ret = reserve(timer, timer->last+1);
	if (ret == -1) return -1;

	timer->list[++timer->last] = entry;
	adjust_up(timer, timer->last);

	return 0;
}


int nhp_timer_pop(mhp_timer_t *timer, timer_entry_t** entry) {
	if (timer == NULL) return -1;

	timer_entry_t **list = timer->list;
	*entry = timer->list[0];
	swap_ptr(list[0], list[timer->last]);
	timer->last --;

	adjust_down(timer, 0);
}


timer_entry_t* mhp_timer_top(mhp_timer_t *timer) {
	if (timer == NULL) return NULL;

	return timer->list[0];
}