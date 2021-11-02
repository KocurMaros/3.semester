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

void actually_sorting(int *start, size_t lenght);
void merge_sort_mt(int *start, size_t len, int depth);
void *merge_sort_thread(void *pv);
void merge(int *start,int *mid,int *end);

struct Parameters //for threading 
{
    char *start;
    size_t len;
};
struct Params //for sorting
{
    int *start;
    size_t len;
    int depth;
};
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
void *sorting(void *pv);
void response_from_client();
void *merge_sort_thread(void *pv);
void sort();
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
                bzero(buff,sizeof(buff));
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

void response_from_client(int socket,char *buff,int lenght){ //zde pouzijem thready na sort a pipy na vypis || shared memory 
    buff[lenght]='\0';
    int fd[2];
    if(pipe(fd)<0){
        perror("pipe");
        exit(1);
    }
    char inbuff[1024];
    if(buff[0] == '[' && buff[lenght-1]==']'){
        struct Parameters parameters = {buff, lenght};
        pthread_t thrd;

        pthread_create(&thrd, NULL, sorting, &parameters);
        
        pthread_join(thrd,NULL);
        
        buff[lenght-1]=']';
        send(socket, buff,lenght,0);  //pipes 4fun
        write(fd[1],buff,lenght);
        // for(int i=0;i<lenght;i++)
        //     printf("%c",buff[i]);
        pthread_mutex_lock(&mtx);
            printf("\n[+]mutex done \n");
        pthread_mutex_unlock(&mtx);
        read(fd[0],inbuff,1024);
        printf("Pipes for live: %s\n",inbuff);
        
    }
    else{
        printf("Client:\t%s\n",buff);
        send(socket,"done\0",5,0);
    }
    
}
/*
#define SHSIZE 1024
#define KEY 1603

void sort(char buff[],int lenght,int socket){ //shared memory
    int shmid;
    key_t key;
    key = KEY;
    char *shm;

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
    sleep(1);  //zeby stihlo sortnut srandy
    strcpy(buff,shm);
    buff[0]='[';
    send(socket,buff,strlen(buff),0);
    // shm = shmat(shmid, NULL, 0);
    
    printf("execute done\n");
    while(*shm != '*'){
        sleep(1);
    }
}

void *sorting(void *pv){
    struct Parameters *parameters = pv;
    int shmid;
    key_t key;
    key = KEY;
    char *shm;

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
    memcpy(shm, parameters->start, parameters->len);
    
    system("./sort");
    while(*shm != '*'){
        sleep(1);
    }
    char buff[1024];
    int i;
    for(i=0;i<parameters->len;i++){
        buff[i]=shm[i];
        printf("%c",buff[i]);
    }
    buff[i]='\0';
    *shm = NULL;
    send(parameters->socket,buff,parameters->len,0);
}
*/
void *sorting(void *pv){
    struct Parameters *parameters = pv;
    //pipe for vypis 
    int buff[512];
    char temp[5]; 
    int c,g=0;
    char *s = parameters->start;
    for(int i=1;i<parameters->len;i++){
        c=0;
        while(s[i+c]!=','&&s[i+c]!=']')
            c++;
        int j;
        for(j=0;j<c;j++)
            temp[j]=s[i+j];
        temp[j]='\0';
        i+=c;
        buff[g++] = atoi(temp); 
    }
    actually_sorting(buff,g);
    strcpy(s, "[");
    for(int i=0;i<(sizeof(buff)/sizeof(int))-1;i++)
        sprintf(&s[strlen(s)],"%d,",buff[i]);
    sprintf(&s[strlen(s)],"%d",buff[g]);
    strcat(s,"]");
    strcat(s,"\0");
}
void actually_sorting(int *start, size_t lenght){
    merge_sort_mt(start,lenght,4); // ll use 7 threads
}
void merge_sort_mt(int *start, size_t len, int depth){

    if(len < 2)
        return;
    if(depth <=0 || len < 4){
        merge_sort_mt(start, len/2, 0);
        merge_sort_mt(start+len/2, len-len/2, 0);
    }
    else{
        struct Params params = {start, len/2, depth/2};
        pthread_t thrd;
        // pthread_mutex_lock(&mtx);
        // printf("Starting subthread...\n");
        // pthread_mutex_unlock(&mtx);

        pthread_create(&thrd, NULL, merge_sort_thread, &params);
        
        merge_sort_mt(start+len/2, len-len/2, depth/2);

        pthread_join(thrd, NULL);

        // pthread_mutex_lock(&mtx);
        // printf("Finished subthread.\n");
        // pthread_mutex_unlock(&mtx);
    }
    merge(start, start+len/2, start+len);
}

void *merge_sort_thread(void *pv){
    struct Params *params = pv;
    merge_sort_mt(params->start, params->len, params->depth);
    return pv;
}
void merge(int *start, int *mid, int *end){
    int *res = malloc((end - start)*sizeof(*res));
    int *lhs = start, *rhs = mid, *dst = res;

    while(lhs != mid && rhs != end)
        *dst++ = (*lhs < *rhs) ? *lhs++ : *rhs++;
    while(lhs != mid)
        *dst++ = *lhs++;

    memcpy(start, res, (rhs-start)*sizeof *res);
    free(res);
}