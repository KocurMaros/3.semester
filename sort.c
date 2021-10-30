#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#include<pthread.h>

#define SHSIZE 1024
#define KEY 1603

void actually_sorting(int *start, size_t lenght);
void merge_sort_mt(int *start, size_t len, int depth);
void *merge_sort_thread(void *pv);
void merge(int *start,int *mid,int *end);

struct Params
{
    int *start;
    size_t len;
    int depth;
};
//for synchronizing stdout from overlap
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

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
    printf("NOT SORTED:\n");
    for(int i=0;i<g;i++)
        printf("%d ", buff[i]);  
    printf("\n");

    actually_sorting(buff,g);
    printf("Sorted:\n");
    for(int i=0;i<g;i++)
        printf("%d ", buff[i]);  
    printf("\n");
    *shm = '*'; //end message
    return 0;
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
        pthread_mutex_lock(&mtx);
        printf("Starting subthread...\n");
        pthread_mutex_unlock(&mtx);

        pthread_create(&thrd, NULL, merge_sort_thread, &params);

        merge_sort_mt(start+len/2, len-len/2, depth/2);

        pthread_join(thrd, NULL);

        pthread_mutex_lock(&mtx);
        printf("Finished subthread.\n");
        pthread_mutex_unlock(&mtx);
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