#include <sys/socket.h>  
#include <netinet/in.h>  
#include <stdio.h>  
#include <string.h> 
#include <stdlib.h>  
#include <arpa/inet.h> 
#include <unistd.h> 

#define PORT 7777
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
int main(){

    int client_socket;
    struct sockaddr_in server_address;
    char buff[256];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0)
        perror("Error in connection\n");
    
    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    //server_address.sin_addr.s_addr = INADDR_ANY;

    int ret;
    int addr_len;
    addr_len = sizeof(server_address);
    ret = connect(client_socket, (struct socketaddr*)&server_address, addr_len);
    if(ret < 0){
        perror("Error in connection");
        exit(1);
    }
    while(1){
        printf("Client:\t");
        scanf("%s",&buff[0]);
        send(client_socket,buff,strlen(buff),0);
        if(strcmp(buff, ":exit") == 0){
            close(client_socket);
            exit(1);
        }
        if(recv(client_socket,buff,1024,0)<0)
            perror("Error in receiving data");
        else
            printf("Server:\t%s\n",buff);
    }
    return 0;
}
#pragma GCC diagnostic pop
