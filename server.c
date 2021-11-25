#include <sys/socket.h>  
#include <netinet/in.h>  
#include <stdio.h>  
#include <stdbool.h>
#include <string.h> 
#include <stdlib.h>  
#include <arpa/inet.h> 
#include <unistd.h> 

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <time.h>
#include <pthread.h>
#include <signal.h>

#define PORT 7777

void actually_sorting(int *, size_t);
void merge_sort_mt(int *, size_t, int);
void *merge_sort_thread(void *);
void merge(int *,int *,int *);
timer_t created_timer(int);
void start_timer(timer_t, int);
bool shutdown_sginal = false;

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
int p;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
void *sorting(void *pv);
void response_from_client();
void *merge_sort_thread(void *pv);
void sort();
void *incrementation(){
    for(int i=0;i<1000000;i++){
        pthread_mutex_lock(&mtx);
        p++;
        pthread_mutex_unlock(&mtx);
    }
}
void *up(int a){
    printf("Nobody connected last 10 sec\n");
}
void *end(){
    int a;
    scanf("%d",&a);
    if(a == 69)
       exit(1); 
}
int main(){
    int server_socket;
    struct sockaddr_in server_address;

   
    struct sockaddr_in new_address;
    socklen_t addr_size;
    
    char buff[1024];
    char welcome_messagge[128] = "Welcome on sorting algoritmus server,if you want to sort number please insert numbers in [] and separate with ','";
    pid_t pid;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
        perror("Error in connection\n");
    printf("[+] Socket is created\n");

    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    timer_t timer;
    if(signal(SIGINT, up) == SIG_ERR)
        printf("cant catch sigint\n");
    timer = created_timer(SIGINT);
    pthread_t end_tid;
    pthread_create(&end_tid, NULL, end, NULL);
    int new_socket;
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
        timer = created_timer(SIGINT);
        start_timer(timer,10);
        new_socket = accept(server_socket,(struct sockaddr*)&new_address, &addr_len); 
        timer_delete(timer);
        if(new_socket < 0 ){
            perror("Socket create error");
            exit(1);
        }
        printf("Connection accept from %s:%d\n",inet_ntoa(new_address.sin_addr),ntohs(new_address.sin_port));
        send(new_socket,welcome_messagge,128,0);
        if((pid = fork()) == 0){  //precesy
            close(server_socket);
            int len;
            while(1){
                bzero(buff,sizeof(buff));
                len=recv(new_socket, &buff, 1024, 0);
                if(strcmp(buff, ":e") == 0){
                    close(new_socket);
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
    bzero(inbuff, 1024);
    clock_t t;
    if(buff[0] == '[' && buff[lenght-1]==']'){
        struct Parameters parameters = {buff, lenght};
        pthread_t tid1,tid2,tid3;
        t = clock();
        pthread_mutex_init(&mtx,NULL);
        pthread_create(&tid1, NULL, sorting, &parameters);
        pthread_join(tid1,NULL);
        printf("2000000 = %d\n",p);
        buff[lenght-1]=']';
        buff[lenght]='\0';
        send(socket, buff,lenght,0);  
        t = clock()-t;
        double time_taken = ((double)t)/CLOCKS_PER_SEC;
        printf("Sort took %f sec\n",time_taken);
        write(fd[1],buff,lenght);  //pipes 4fun
        
        read(fd[0],inbuff,lenght+1);
        int i=0;
        while(inbuff[i]!='\0')
            printf("%c",inbuff[i++]);
        printf("\n");
        close(fd[0]);
        close(fd[1]);
        pthread_mutex_destroy(&mtx);
    }
    else{
        printf("Client:\t%s\n",buff);
        pthread_t tid2,tid3;
        p=0;
        pthread_create(&tid2, NULL, incrementation, NULL);
        pthread_create(&tid3, NULL, incrementation, NULL);
        pthread_join(tid2,NULL);
        pthread_join(tid3,NULL);
        printf("2000000 = %d\n",p);
        send(socket,"done\0",5,0);
    }
    
}
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

        pthread_create(&thrd, NULL, merge_sort_thread, &params);
        
        merge_sort_mt(start+len/2, len-len/2, depth/2);

        pthread_join(thrd, NULL);
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

timer_t created_timer(int signal){
    struct sigevent sig;
    sig.sigev_notify = SIGEV_SIGNAL;
    sig.sigev_signo = signal;

    timer_t timer; 
    timer_create(CLOCK_REALTIME, &sig, &timer);
    return(timer);
}
void start_timer(timer_t timer, int sec){
    struct itimerspec tim;
    tim.it_value.tv_sec = sec;
    tim.it_value.tv_nsec = 0;
    tim.it_interval.tv_sec = sec;
    tim.it_interval.tv_nsec = 0;
    timer_settime(timer, CLOCK_REALTIME, &tim,  NULL);
    printf("timer started\n");
}
