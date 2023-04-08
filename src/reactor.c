#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>


#define EPOLL_SIZE		1024
#define MAX_ITEMS_SIZE	1024
#define MAX_EPOLL_SIZE	1024
#define BUFFER_SIZE		1024
#define LISTEN_SIZE		1024
#define PORT			8888

#define ERROR_MALLOC	-1

#define READ_EVENT		0
#define WRITE_EVENT     1
#define ACCEPT_EVENT    2

#define EPOLL_LT		0
#define EPOLL_ET		1
#define EPOLL_MODE		EPOLL_LT

#define DEBUG			1		// 1：开启调试 | 0：关闭调试

static int ERROR;
static int server_fd;

typedef void (*HANDLE)(int fd, void *arg);

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
	size_t buf_w_off;		// buffer offset
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



struct reactor* get_reactor_instance();
int reactor_set_event(int fd, HANDLE handle,int event, void* arg);
void handle_read(int fd, void *arg);
void handle_write(int fd, void *arg);



struct itemblock* itemblock_create(struct itemblock *prev) {
	struct itemblock *block = (struct itemblock*)malloc(sizeof(struct itemblock));
	if (block == NULL) {
		perror("malloc");
		ERROR = ERROR_MALLOC;
		return NULL;
	}

	block->next = NULL;

	block->item_array = (struct nitem*)malloc(MAX_ITEMS_SIZE*sizeof(struct nitem));
	if (block->item_array == NULL) {
        perror("malloc");
		ERROR = ERROR_MALLOC;
        return NULL;
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


void handle_write(int fd, void *arg) {
	struct reactor *reactor = get_reactor_instance();
	struct nitem *item = itemblock_find_nitem(reactor->head, fd);
	char *buffer = item->buf_write;
	size_t *len = &(item->buf_w_len);
	size_t *offset = &(item->buf_w_off);

again:
	size_t nwrite = send(fd, buffer+*offset, *len, MSG_DONTWAIT);
	if (nwrite < 0) {
		if (errno == EINTR) goto again;
	} else if (nwrite >= 0) {
		*offset += nwrite;
	}

	if (*offset == *len) {
		memset(buffer, 0, *len);
		reactor_set_event(fd, handle_read, READ_EVENT, NULL);
		*offset = 0;
		*len = 0;
	} else {
		// 继续发送剩余数据
		reactor_set_event(fd, handle_write, WRITE_EVENT, NULL);
	}

}


void handle_read(int fd, void *arg) {
	struct reactor *reactor = get_reactor_instance();
	struct nitem *item = itemblock_find_nitem(reactor->head, fd);
	char *buffer = item->buf_read;
	size_t *len = &(item->buf_r_len);

again:
#if (EPOLL_MODE==EPOLL_LT)

	size_t nread = recv(fd, buffer+*len, sizeof(BUFFER_SIZE)-*len, MSG_DONTWAIT);

	if (nread == 0) {
		;
	} else if (nread > 0) {
		*len += nread;
		printf("read %ld bytes\n", nread);
	} else if (nread < 0) {
		if (errno == EINTR) goto again;
	}

#elif (EPOLL_MODE == EPOLL_ET)

#endif
}

void handle_accept(int fd, void *arg) {
	int connfd;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(struct sockaddr_in);
	connfd = accept(fd, (struct sockaddr *)&client_addr, &len);
	if (connfd < 0) {
        perror("accept");
        return;
    }

	reactor_set_event(connfd, handle_read, READ_EVENT, NULL);
}

static void server_init() {
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_fd, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
        exit(1);
	}

	if (listen(server_fd, LISTEN_SIZE) == -1) {
		perror("listen");
        exit(1);
	}

	// 设置反应堆
	reactor_set_event(server_fd, handle_accept, ACCEPT_EVENT, NULL);

}

static inline const int get_server_fd() {
	return server_fd;
}


static int reactor_init(struct reactor *reactor) {
	if (reactor == NULL) return -1;

	reactor->epfd = epoll_create(EPOLL_SIZE);
	if (reactor->epfd < 0) {
        perror("epoll_create");
        return -1;
    }

	reactor->head = itemblock_create(NULL);
	if (reactor->head == NULL) {
		return -1;
	}

	server_init();

	return 0;
}


struct reactor* get_reactor_instance() {	// singleton
	static struct reactor* instance = NULL;
	if (instance == NULL) {
		instance = (struct reactor*)malloc(sizeof(struct reactor));
        if (instance == NULL) {
            perror("malloc");
			ERROR = ERROR_MALLOC;
            return NULL;
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
	ep_event.data.fd = fd;
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
	uint32_t u32i;
	struct reactor *reactor = get_reactor_instance();
	if (reactor == NULL) {
		printf("reactor_run failed\n");
		return;
	}

	struct epoll_event event_array[MAX_EPOLL_SIZE] = {0};
	while (1) {
		int nready = epoll_wait(reactor->epfd, event_array, MAX_EPOLL_SIZE, -1);
		for (u32i=0; u32i<nready; u32i++) {
			int clientfd = event_array[u32i].data.fd;
			int ep_event = event_array[u32i].events;

			struct nitem* nitem = itemblock_find_nitem(reactor->head, clientfd);
			if (clientfd == get_server_fd()) {			// new connection
				printf("new connection\n");
				nitem->accept_handle(clientfd, NULL);
				continue;
			}
			if (ep_event & EPOLLIN) {			// read event
				nitem->read_hanle(clientfd, NULL);
			}
			if (ep_event & EPOLLOUT) {			// write event
				nitem->write_handle(clientfd, NULL);
			}

		}
	}
}




#if DEBUG


int main(int arg, char** argv) {

	printf("server started !!! \n");

	reactor_run();
}


#endif
