// csci 4430 assignment
// server-client model
// one is for client
// one is for server
// server should handle the multi-thread issue, 50 clients max
// use tcp protocol
// client input message is 128 max, include \n

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <signal.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <pthread.h>

// create a global counter:
static int count_num=0;

// create a mutex for all thread:
pthread_mutex_t single_entry;

struct thread_information{
    int client_sd_info;
    struct sockaddr_in client_addr_info;
};

void *thread_handler(void* thread_arg);

// basic function for making the thread
int main(int argc, char** argv){
    
    // check the argument number
    if(argc!=2){fprintf(stderr, "Usage: %s [port] \n", argv[0]);exit(1);}
    
    // create a TCP socket
    // sd stands for socket descrptor, which is similar to file descriptor
    // af_inet protocol is standard for this assignment
    // sock_stream is for making a tcp connection
    int sd=socket(AF_INET,SOCK_STREAM,0);
    
    
    // reuse server port to avoid "bind: address already in use"
    int val = 1;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val))<0)
        {printf("setsocketopt error: %s (Errno:%d)\n",strerror(errno),errno);exit(1);}
    
    
    // handle the arguments
    unsigned short port;
    port = atoi(argv[1]);
    
    // configure settings of the server address struct
    // create a socket address variable, and initilaize it
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    // accept connection to any ip address of this host
    // ??????? need to modify ?
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(port);
    
    
    // bind the address struct to the socket
    // if bind() return a value less than 0, then there is a bind error
    // bind(sd, sock_address, address_len)
    if(bind(sd,(struct sockaddr *) &server_addr,sizeof(server_addr))<0){
        printf("bind error: %s (Errno:%d)\n",strerror(errno),errno);
        exit(1);
    }
    
    // listen on the socket, with 50 max connection requests queued
    if(listen(sd,50)<0){
        printf("listen error: %s (Errno:%d)\n",strerror(errno),errno);
        exit(1);
    }
    
    // create a new socket for the incoming connection
    int client_sd;
    struct sockaddr_in client_addr;
    int addr_len=sizeof(client_addr);
    
    // accept the incoming connection
    while ((client_sd=accept(sd,(struct sockaddr*)&client_addr,(socklen_t*)&addr_len))>=0) {
        //printf("connection established\n");
        // make a thread for the client to connect
        pthread_t connection_thread_id;
        struct thread_information thread_arg;
        thread_arg.client_sd_info=client_sd;
        thread_arg.client_addr_info=client_addr;
        pthread_create(&connection_thread_id, NULL,thread_handler,(void*)&thread_arg);
        //printf("one thread is already in use\n");
        // go to the function
    }
    
    close(sd);
    return 0;
}

void *thread_handler(void* thread_arg){
    int client_sd= ((struct thread_information*)thread_arg)->client_sd_info;
    struct sockaddr_in client_addr= ((struct thread_information*)thread_arg)->client_addr_info;
    
    while(1){
        char buff[129];
        int len;
        // receive message from the client
        // process the message for output
        if((len=recv(client_sd,buff,sizeof(buff),0))<=0){
            //printf("Client disconnected\n");
            fflush(stdout);
            break;
        }
        
        //printf("len is : %d\n",len);
        //printf("len-1 position: %c\n",buff[len-1]);
        
        buff[len]='\0';
        // get the client ip address
        char* client_ip;
        client_ip = inet_ntoa(client_addr.sin_addr);
        // get the client connected port
        unsigned short connected_port;
        connected_port=(unsigned short)ntohs(client_addr.sin_port);
        // print out the message
        pthread_mutex_lock(&single_entry);
        count_num++;
        printf("Message %d, From %s, Port %d: ",count_num,client_ip,connected_port);
        if(strlen(buff)!=0)printf("%s",buff);
        pthread_mutex_unlock(&single_entry);
    }
    return NULL;
}

void signal_handler(int signal)
{
    printf("Signal %d Received.Kill me if you can\n",signal);
}

/*
 if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0){
 printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
 exit(1);
 }
 
 
 while(1){
 char buff[128];
 int len;
 // receive message from the client
 // process the message for output
 if((len=recv(client_sd,buff,sizeof(buff),0))<=0){
 printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
 exit(1);
 }
 buff[len]='\0';
 // get the client ip address
 char* client_ip;
 client_ip = inet_ntoa(client_addr.sin_addr);
 // get the client connected port
 unsigned short connected_port;
 connected_port=(unsigned short)ntohs(client_addr.sin_port);
 // print out the message
 printf("Message number, From %s , Port %d :",client_ip,connected_port);
 if(strlen(buff)!=0)printf("%s",buff);
 }
 close(sd);
 */



// implement a tcp get information function

// implement a display information function