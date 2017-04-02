#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "mtcp_client.h"

/* -------------------- Global Variables -------------------- */
//global variable for making connection
int sockfd;
struct sockaddr_in *addr;
struct sockaddr_in server;

//global variable for determine which state we are in
typedef enum {INIT,HS3,RW,HS4,END,NIL} state_t;	
state_t state = NIL;
unsigned int current_ack;
unsigned int last_seq;
unsigned char last_recv_type=-1;
unsigned char last_sent_type=-1;
ssize_t sendto_err = 0;

//global variable for the sending buffer, which is a common thing
static pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MAX_BUF_SIZE 1024
#define SEND_BUF_SIZE 268435456
typedef struct
{
	int front,rear;
	int capacity;
	unsigned char *array;
}buffer_t;
//the global sendbuf
buffer_t *sendbuf=(buffer_t *)malloc(sizeof(buffer_t));
sendbuf->capacity = size;
sendbuf->front = -1;
sendbuf->rear = -1;
sendbuf->array=(unsigned char *)malloc(sendbuf->capacity*sizeof(unsigned char));
//function for making change to the buffer
int is_empty(buffer_t *q)
{
	return (q->front==-1);
}

int is_full(buffer_t *q)
{
	return (((q->rear+1)%q->capacity) == q->rear);
}

int buf_size(buffer_t *q)
{
	if(q->front==-1)return 0;
	return (q->rear - q->front +1)%q->capacity;
}
	
int enqueue(buffer_t *q, unsigned char *src,  int len)
{	
	if(DEBUG)printf("buf_size=%d\n",buf_size(q));
	if(is_full(q))return 0;
	else if(len > (q->capacity-buf_size(q)))return 0;
	else
	{
		int tmp = buf_size(q);
		q->rear = (q->rear + len)%q->capacity;
		memcpy(&(q->array[q->rear-len+1]),src,len);
		if(q->front==-1)
		{
			q->front = q->rear - len+1;
		}
		return buf_size(q)-tmp;
	}
}

int dequeue(buffer_t *q,unsigned char *dst, int len)
{
	if(is_empty(q))return 0;
	else
	{
		memcpy(dst,&(q->array[q->front]),len);
		if(q->front==(q->rear - len+1))
			q->front=q->rear=-1;
		else
			q->front=(q->front + len)%q->capacity;
		return 1;
	}
}


/* ThreadID for Sending Thread and Receiving Thread */
static pthread_t send_thread_pid;
static pthread_t recv_thread_pid;

static pthread_cond_t app_thread_sig = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t app_thread_sig_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t send_thread_sig = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t send_thread_sig_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t info_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Connect Function Call (mtcp Version) */
void mtcp_connect(int socket_fd, struct sockaddr_in *server_addr){

	//create a common used buffer
	//which is the sendbuf, created

	//initialize the address for sending thread to use
	current_ack=0;//ack
	sockfd=socket_fd;//destination
	addr = malloc(sizeof(*addr));
	memcpy(addr,server_addr,sizeof(*addr));//address

    //create the sending thread, create the recieving thread, 
    pthread_create(&send_thread_pid,NULL,send_thread,NULL);
    pthread_create(&recv_thread_pid,NULL,receive_thread,NULL);

    //wake the sending thread to get the 3-way-handshake
    state=HS3;
    pthread_cond_signal(&send_thread_sig);
    pthread_cond_wait(&app_thread_sig,&app_thread_sig_mutex);

    //change the state to transmission
    state=RW;

    return;
}

/* Write Function Call (mtcp Version) */
int mtcp_write(int socket_fd, unsigned char *buf, int buf_len){
	//check if i am in the ending state
	//TBD : if I am in the ending state, don't send anymore, return -1
	int cond=3;
	if(sendto_err==-1 || state==HS4 || state==NIL || state==END){cond=1;}
	else if(if_full(buffer)){cond=2;}

	if(cond==1){
		printf("the connection is ended/ going to end\n");
		return -1;
	}
    //keep writing to the maximum buffer area
    //if buffer is full, don't send anymore, return 0
	else if(cond==2){
		printf("the buffer is full.\n");
		return 0;
	}

    //call the sending thread with the correct number of data
	else{
	    pthread_mutex_lock(&send_thread_sig_mutex);
		pthread_cond_signal(&send_thread_sig);
		pthread_mutex_unlock(&send_thread_sig_mutex);
	}
	return 1;
}

/* Close Function Call (mtcp Version) */
void mtcp_close(int socket_fd){
    //wake the sending thread to send signal to finish the 4-way-handshake
   	//wait for the buffer to send out, and ack is recieved
   	unsigned char flag = 1;
	while(flag)	//block until all buffer sent
	{
		pthread_mutex_lock(&sendbuf_mutex);
		flag = !is_empty(sendbuf);
		pthread_mutex_unlock(&sendbuf_mutex);
	}
	flag = 1;
	while(flag)	//block until ACK of last sent DATA recieved
	{
		pthread_mutex_lock(&info_mutex);
		if(last_seq!=current_ack) flag = 0;
		pthread_mutex_unlock(&info_mutex);
	}
    //TBD : change the state to end

   	//wake the send thread for 4-way-handshake
    pthread_mutex_lock(&send_thread_sig_mutex);
	pthread_cond_signal(&send_thread_sig);
	pthread_mutex_unlock(&send_thread_sig_mutex);	
	
	//finished 4-way-handshake
	pthread_mutex_lock(&app_thread_sig_mutex);
	pthread_cond_wait(&app_thread_sig,&app_thread_sig_mutex);	//wait for send thread
	pthread_mutex_unlock(&app_thread_sig_mutex);	
		
	//for 2 thread came back
	pthread_join(recv_thread_pid,NULL);
	pthread_join(send_thread_pid,NULL);

	//close all
	close(socket_fd);
	free(addr);
}

static void *send_thread(){

}

static void *receive_thread(){

}
