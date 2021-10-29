#include <sys/socket.h>  
#include <netinet/in.h>  
#include <stdio.h>  
#include <string.h> 
#include <stdlib.h>  
#include <arpa/inet.h> 
#include <unistd.h> 

#include<time.h>
#include<pthread.h>

#define PORT 7777

struct parameters{
    int *start;
    size_t len;
    int depth;
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void response_from_client();
void *merge_sort_thread(void *pv);

int main(){
    int server_socket;
    struct sockaddr_in server_address;
    
    int new_socket;
    struct sockaddr_in new_address;

    socklen_t addr_size;
    
    char buff[1024];
    pid_t childpid;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
        perror("Error in connection\n");
    printf("[+] Socket is created\n");

    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = bind(server_socket,(struct sockaddr*)&server_address,sizeof(server_address));
    if(ret<0)
        perror("Error in binding");
    printf("[+] Bind to port: %d\n",ntohs(server_address.sin_port));
    if(listen(server_socket,10)==0)
        printf("[+]Listening...\n");
    else
        perror("error in binding");
    int addr_len;
    while(1){
        addr_len = sizeof(new_address);
        new_socket = accept(server_socket,(struct sockaddr*)&new_address, &addr_len);
        if(new_socket < 0 ){
            perror("Socket create error");
            exit(1);
        }
        printf("Connection accept from %s:%d\n",inet_ntoa(new_address.sin_addr),ntohs(new_address.sin_port));
        if((childpid = fork()) == 0){  //precesy
            close(server_socket);
            int len;
            while(1){
                len=recv(new_socket, &buff, 1024, 0);
                if(strcmp(buff, ":exit") == 0){
                    printf("Disconnected from %s:%d\n",inet_ntoa(new_address.sin_addr),ntohs(new_address.sin_port));
                    break;
                }
                else{
                    response_from_client(new_socket, buff,len);
                    bzero(buff,sizeof(buff));
                }
            }
        }
    }
    close(new_socket);
    return 0;
}
void response_from_client(int socket,char buff[],int lenght){ //zde pouzijem thready na sort a pipy na vypis || shared memory 
    buff[lenght]='\0';
    if(buff[0] == '[' && buff[lenght-1]==']'){
        sorting_algoritm(buff, lenght);
        printf("ssad");
    }
    else
        printf("Client:\t%s\n",buff);
    send(socket,buff,strlen(buff),0);
}
