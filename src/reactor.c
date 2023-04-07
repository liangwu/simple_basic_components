#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>


#define EPOLL_SIZE		1024
#define MAX_ITEMS_SIZE	1024
#define MAX_EPOLL_SIZE	1024
#define BUFFER_SIZE		1024
#define LISTEN_SIZE		1024
#define PORT			8848

#define ERROR_MALLOC	-2

#define READ_EVENT		0
#define WRITE_EVENT     1
#define ACCEPT_EVENT    2


#define DEBUG			1		// 1：开启调试 | 0：关闭调试


typedef void (*HANDLE)(int fd, int event, void *arg);

/*
* @brief 连接请求的信息、处理函数和缓冲区
*/
struct nitem
{
	int fd;
#if 0
	void (*handle)(int fd, int event, void *arg);
#else
	HANDLE write_handle;
	HANDLE read_hanle;
	HANDLE accept_handle;
#endif
	void *arg;

	char buf_write[BUFFER_SIZE];
	char buf_read[BUFFER_SIZE];
	size_t buf_w_len;
	size_t buf_r_len;
};


struct itemblock {
	struct itemblock *next;
	struct nitem *item_array;
};


struct reactor {
	int epfd;
	struct itemblock *head;
	//struct itemblock **tail;
};

struct itemblock* itemblock_create(struct itemblock *prev) {
	struct itemblock *block = (struct itemblock*)malloc(sizeof(struct itemblock));
	if (block == NULL) {
		perror("malloc");
		return ERROR_MALLOC;
	}

	block->next = NULL;

	block->item_array = (struct nitem*)malloc(MAX_ITEMS_SIZE*sizeof(struct nitem));
	if (block->item_array == NULL) {
        perror("malloc");
        return ERROR_MALLOC;
    }
	memset(block->item_array, 0, sizeof(struct nitem)*MAX_ITEMS_SIZE);

	if (prev != NULL) {			// 设置前节点
		prev->next = block;
	}

    return block;
}

/*
* @brief 输出 struct nitme*
*/
struct nitem* itemblock_find_nitem(struct itemblock* head, int fd) {
	if (head == NULL) return NULL;
	uint32_t u32i;

	struct itemblock* block = head;
	size_t page_num = fd/MAX_ITEMS_SIZE;
	size_t index = fd%MAX_ITEMS_SIZE;

	for (u32i=0; u32i<page_num; u32i++) {
		block = block->next;
	}
	if (block == NULL) return NULL;

	return &(block->item_array[index]);

}


void handle_write(int fd, int event, void *arg) {

}


void handle_read(int fd, int event, void *arg) {

}

void handle_accept(int fd, int event, void *arg) {
	int connfd;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(struct sockaddr_in);
	connfd = accept(fd, (struct sockaddr *)&client_addr, len);
	if (connfd < 0) {
        perror("accept");
        return;
    }

	reactor_set_event(connfd, handle_read, READ_EVENT, NULL);
}

void server_init() {
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_fd, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
        exit(1);
	}

	if (listen(server_fd, LISTEN_SIZE)) {
		perror("listen");
        exit(1);
	}

	// 设置反应堆
	reactor_set_event(server_fd, handle_accept, ACCEPT_EVENT, NULL);

}


static int reactor_init() {
	struct reactor *reactor = get_reactor_instance();

	reactor->epfd = epoll_create(0);
	if (reactor->epfd < 0) {
        perror("epoll_create");
        return -3;
    }

	reactor->head = (struct itemblock*)malloc(sizeof(struct itemblock));
	if (reactor->head == NULL) {
        perror("malloc");
        return ERROR_MALLOC;
    }
	memset(reactor->head, 0, sizeof(struct itemblock));
	reactor->head = itemblock_create(NULL);
	if (reactor->head == ERROR_MALLOC) {
		return -1;
	}

	return 0;
}


struct reactor* get_reactor_instance() {	// singleton
	static struct reactor* instance = NULL;
	if (instance == NULL) {
		instance = (struct reactor*)malloc(sizeof(struct reactor));
        if (instance == NULL) {
            perror("malloc");
            return ERROR_MALLOC;
        }
		memset(instance, 0, sizeof(struct reactor));
        if (reactor_init(instance) < 0 ) {return NULL;}
	}
	return instance;
}


int reactor_set_event(int fd, HANDLE handle,int event, void* arg) {
	struct reactor *reactor = get_reactor_instance();

	struct nitem* item = itemblock_find_nitem(reactor->head, fd);
	if (reactor == NULL) {return -1;}

	struct epoll_event ep_event = {0};
	ep_event.data.ptr = arg;
	if (event == READ_EVENT) {
		item->fd = fd;
		item->read_hanle = handle;
		item->arg = arg;
		ep_event.events = EPOLLIN;
	} else if (event == WRITE_EVENT) {
		item->fd = fd;
		item->write_handle = handle;
		item->arg = arg;
		ep_event.events = EPOLLOUT;
	} else if (event == ACCEPT_EVENT) {
		item->fd = fd;
		item->accept_handle = handle;
		item->arg = arg;
		ep_event.events = EPOLLIN;
	} else {
		return -1;
	}
	epoll_ctl(reactor->epfd, EPOLL_CTL_ADD, fd, &ep_event);

    return 0;
}

void reactor_run() {
	struct reactor *reactor = get_reactor_instance();
	if (reactor == NULL) {
		printf("reactor_run failed\n");
		return;
	}

	struct epoll_event event_array[MAX_EPOLL_SIZE] = {0};
	while (1) {
		int nready = epoll_wait(reactor->epfd, event_array, MAX_EPOLL_SIZE, -1);
		while (nready-- > 0) {

		}
	}
}




#if DEBUG


int main(int arg, char** argv) {

}


#endif
