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
int sockfd;
struct sockaddr_in *addr;
struct sockaddr_in server;

state_t state = NIL;
unsigned int current_ack;
unsigned int last_seq;
unsigned char last_recv_type=-1;
unsigned char last_sent_type=-1;
ssize_t sendto_err = 0;


buffer_t *sendbuf;

static pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

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
	//TBD

	//initialize the address for sending thread to use
	current_ack=0;//ack
	sockfd=socket_fd;//destination
	addr = malloc(sizeof(*addr));
	memcpy(addr,server_addr,sizeof(*addr));//address

    //create the sending thread, create the recieving thread, 
    pthread_create(&send_thread_pid,NULL,send_thread,NULL);
    pthread_create(&recv_thread_pid,NULL,receive_thread,NULL);

    //wake the sending thread to get the 3-way-handshake
    //TBD : what state is it
    pthread_cond_signal(&send_thread_sig);
    pthread_cond_wait(&app_thread_sig,&app_thread_sig_mutex);

    //change the state to transmission
    //what state is it

    return;
}

/* Write Function Call (mtcp Version) */
int mtcp_write(int socket_fd, unsigned char *buf, int buf_len){
	//check if i am in the ending state
	//TBD : if I am in the ending state, don't send anymore, return -1
	int cond=3;
	if(cond==1){
		return -1;
	}
    //keep writing to the maximum buffer area
    //if buffer is full, don't send anymore, return 0
	else if(cond==2){
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
   	int ack_num=1;
   	while(ack_num==end_ack_num){
   		pthread_mutex_lock(&info_mutex);
		ack_num=update_ack_num(); // TBD : to be implemented
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
