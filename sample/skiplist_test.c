
#define __SKIPLIST_PRINT
#include "skiplist.h"


void func(void *arg) {
	return ;
}


int main(int argc, char **argv) {

	skplist_t *list = skplist_create();
	int i;
	for (i = 1; i < 20; i++) {
		skplist_insert(list, i, func);
	}

	skplist_print(list);
	skplist_destroy(list);

	return 0;
}
