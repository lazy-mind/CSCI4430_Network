// csci 4430 assignment
// server-client model
// one is for client
// one is for server
// server should handle the multi-thread issue, 50 clients max
// use tcp protocol
// client input message is 128 max, include \n

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
# include <errno.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <netdb.h>

int main(int argc, char **argv)
{
    // check if the user provide
    // destination name
    // port number correctly
    if(argc!=3){
        fprintf(stderr, "Usage: %s [hostname] [port] \n", argv[0]);
        exit(1);
    }
    
    // construct a host content variable to store information, do the error checking
    struct hostent * hostinformation;
    hostinformation=gethostbyname(argv[1]);
    if(hostinformation==NULL){perror("cannot interpret the host name provided\n");exit(2);}
    
    // construct a ip_address_list variable to store all the ip_address, then we can assign
    struct in_addr ** addrList;
    addrList=(struct in_addr**)hostinformation->h_addr_list;
    if(addrList[0]==NULL){perror("cannot get the host ip address\n");exit(3);}
    
    // create a ip address variable to store any of hte ip_address, then we can passed it
    char* ip;
    if( (ip = inet_ntoa(*addrList[0])) == NULL ) {perror("inet_addr()");exit(4);}
    //printf("%s!!!!\n",ip);
    
    // create a port type
    unsigned short port;
    port = atoi(argv[2]);
    
    // create a TCP socket
    int sd=socket(AF_INET,SOCK_STREAM,0);
    
    // configure settings of the server address struct
    // the ip_address is using "ip", which is converted
    // teh port_number is using "port", which is converted
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=inet_addr(ip);
    server_addr.sin_port=htons(port);
    
    // connect the socket to the server using the address struct
    if(connect(sd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
        printf("connection error: %s (Errno:%d)\n",strerror(errno),errno);
        exit(0);
    }
    
    // sending the message
    while(1){
        char buff[129];
        memset(buff,0,129);
        fgets(buff,129,stdin);
        //scanf("%s",buff);
        //strcat(buff,"\n");
        int len;
        // send message to server
        if((len=send(sd,buff,strlen(buff),0))<0){
            printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
            exit(0);
        }
        else if(len==0){
            //printf("end of input, connection terminated\n");
            break;
        }
        //printf("len is : %d\n",len);
        //printf("len-1 position: %c\n",buff[len-1]);
    }
    return 0;
}