#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#define SHSIZE 1024
#define KEY 1603

int main(){
    int shmid;
    key_t key;
    key = KEY;
    char *shm;
    char *s;
    printf("From shared memory:\t");
    shmid = shmget(key, SHSIZE, 0666);
    if(shmid < 0){
        perror("shmget");
        exit(1);
    }
    shm = shmat(shmid, NULL, 0);

    if(shm == (char *) -1){
        perror("shmat");
        exit(1);
    }

    char temp[6];
    //make thread for calc numbers and than allocate
    int buff[512];
    int c=0,g=0;
    int lenght = strlen(shm);
    for(int i=1;i<lenght;i++){
        c=0;
        while(shm[i+c]!=','&&shm[i+c]!=']')
            c++;
        int j;
        for(j=0;j<c;j++)
            temp[j]=shm[i+j];
        temp[j]='\0';
        i+=c;
        buff[g++] = atoi(temp); 
    }
    
    for(int i=0;i<g;i++)
        printf("%d ", buff[i]);
    printf("\n");


    *shm = '*'; //end message
    return 0;
}