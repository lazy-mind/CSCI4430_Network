#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mtcp_server.h"

/* global variable*/
int sockfd;
struct sockaddr_in *addr;
socklen_t addrlen;
struct sockaddr_in *client;
state_t state=NIL;
unsigned int current_ack;
unsigned int last_ack;
unsigned char last_recv_type;
unsigned char last_sent_type;
ssize_t recvfrom_err = 0;

buffer_t *recvbuf;


/* ThreadID for Sending Thread and Receiving Thread */
static pthread_t send_thread_pid;
static pthread_t recv_thread_pid;

static pthread_cond_t app_thread_sig = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t app_thread_sig_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t send_thread_sig = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t send_thread_sig_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t info_mutex = PTHREAD_MUTEX_INITIALIZER;

/* The Sending Thread and Receive Thread Function */
static void *send_thread();
static void *receive_thread();

void mtcp_accept(int socket_fd, struct sockaddr_in *server_addr){
	sockfd = socket_fd;
	addr=(struct sockaddr_in)malloc(sizeof(*addr));
	addr = client_addr;
	//change the state to accpet 3-way-handshake
	printf("I am changing the state to 3-way-handshake");

	//create the thread:
	pthread_create(&send_thread_pid,NULL,send_thread,NULL)
	pthread_create(&recv_thread_pid,NULL,receive_thread,NULL)
	
	//wait for the 3-way-handshake to complete
	pthread_mutex_lock(&app_thread_sig_mutex);
	pthread_cond_wait(&app_thread_sig,&app_thread_sig_mutex);
	pthread_mutex_unlock(&app_thread_sig_mutex);

	return;
}

int mtcp_read(int socket_fd, unsigned char *buf, int buf_len){
	int cond=3;
	if(cond==1){
		//the connection is end, while the main program still try to read
		printf("the connection is already ended!");
		return -1;
	}
	else if(cond==2){
		//the recieve buffer is empty, while the program still want to read
		//in this case we wait for the 
		printf("the recieve buffer is empty!");
		return 
	}
	else if(cond==3){
		//the buffer has data, then we perform the read function
		printf("now we are reading!");
	}
	return 1;
}

void mtcp_close(int socket_fd){
	//wait until the 2 thread to end
	pthread_join(send_thread_pid,NULL);
	pthread_join(recv_thread_pid,NULL);
	
	//change the state to indicate no more listening

	close(socket_fd);
	free(addr);
	return;

}

static void *send_thread(){

}

static void *receive_thread(){

}
