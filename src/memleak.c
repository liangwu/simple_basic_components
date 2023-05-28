/*
 * @Author: 凉屋 liangwu.lxy@foxmail.com
 * @Description: 内存泄漏检测
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define __USE_MALLOC_HOOK_HOOK

extern void *__libc_malloc(size_t size);
extern void __libc_free(void* p);



#ifdef __USE_MALLOC_HOOK_MACRO

void *malloc_hook(size_t size, char *file, int line) {



}


void free_hook(void *p, char *file, int line) {

}


#define malloc(size)	malloc_hook(size, __func__, __LINE__)
#define free(size)		free_hook(size, __func__, __LINE__)

#endif


#ifdef __USE_MALLOC_HOOK_OVERRIDE

int enable_malloc_hook = 1;
int enable_free_hook = 1;

void *malloc(size_t size) {

	if (enable_malloc_hook) {
		enable_malloc_hook = 0;
		// 递归终止
		void *p = __libc_malloc(size);
		void *caller = __builtin_return_address(0);

		char filename[128] = {0};
		sprintf(filename, "%p.mem", p);

		FILE *fp = fopen(filename, "w");
		fprintf(fp, "[+%p] --> addr: %p, size: %ld\n", caller, p, size);
		fflush(fp);
		enable_malloc_hook = 1;
		return p;
	} else {
		return __libc_malloc(size);
	}
}


void free(void *p) {

	if (enable_free_hook) {
		enable_free_hook = 0;

		char filename[128] = {0};
		sprintf(filename, "%p.mem", p);

		if (unlink(filename) < 0) {		// no exist
			printf("double free %p \n", p);
		}

		printf("free %p\n", p);
		__libc_free(p);
		enable_free_hook = 1;
	} else {
		__libc_free(p);
	}
}

#endif // !__USE_MALLOC_HOOK_OVERRIDE


#ifdef __USE_MALLOC_HOOK_HOOK

void *malloc_hook_f(size_t size, const void *caller) {

}

#endif // !__USE_MALLOC_HOOK_HOOK


int main(int argc, char **argv) {

	void *p1 = malloc(10);
	void *p2 = malloc(20);
	free(p1);

}
