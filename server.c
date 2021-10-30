#include <sys/socket.h>  
#include <netinet/in.h>  
#include <stdio.h>  
#include <string.h> 
#include <stdlib.h>  
#include <arpa/inet.h> 
#include <unistd.h> 

#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#include<time.h>
#include<pthread.h>

#define PORT 7777

void response_from_client();
void *merge_sort_thread(void *pv);

int main(){
    int server_socket;
    struct sockaddr_in server_address;
    
    int new_socket;
    struct sockaddr_in new_address;
    socklen_t addr_size;
    
    char buff[1024];
    char welcome_messagge[128] = "Welcome on sorting algoritmus server, if you want to sort number please insert numbers in [] and separate wit ','";
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
        send(new_socket,welcome_messagge,128,0);
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
        sort(buff,lenght);
    }
    else
        printf("Client:\t%s\n",buff);
    send(socket,buff,strlen(buff),0);
}

#define SHSIZE 1024
#define KEY 1603

void sort(char buff[],int lenght){ //shared memory
    int shmid;
    key_t key;
    key = KEY;
    char *shm;
    char *s;

    shmid = shmget(key, SHSIZE, IPC_CREAT | 0666);
    if(shmid < 0){
        perror("shmget");
        exit(1);
    }
    shm = shmat(shmid, NULL, 0);

    if(shm == (char *) -1){
        perror("shmat");
        exit(1);
    }
    memcpy(shm, buff, lenght+1);
    system("./sort");
    s = shm;
    s += 11;

    *s = 0;
    while(*shm != '*')
        sleep(1);
}