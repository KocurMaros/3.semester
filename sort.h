#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#define SHSIZE 1024
#define KEY 1603

class Csort{
    int lenght;
    int num;
    void char_to_int(int lenght,int num);
    void int_to_char(int lenght, int num);
}
extern Csort sort;