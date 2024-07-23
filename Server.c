#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Common/SocketFunction.h"
#include "Server.h"
#include <string.h>
#define PORT 8080

int main(int argc, char *argv[])
{
    char *indirizzo,*porta,*directory;

    if(argc != 7 || argv[1][0] != '-' || argv[3][0] != '-' || argv[5][0] != '-') {
        perror("formato non valido");
        exit(EXIT_FAILURE);
    }
    for(int i = 1; i < argc; i = i + 2)
    {
        if(strcmp(argv[i],"-a") == 0)
        {indirizzo = argv[i+1];}
        if(strcmp(argv[i],"-p") == 0)
        {porta = argv[i+1];}
        if(strcmp(argv[i],"-d") == 0)
        {directory = argv[i+1];}

    }
    char stringa[1024];
    char *check;
    struct sockaddr_in address;
    socklen_t addresslen = sizeof(address);
    address.sin_family=AF_INET;

    u_long portaInt = strtoll(porta,&check,10);
    if(*check != '\0'){
        perror("porta non valida");
        exit(EXIT_FAILURE);
    }

    address.sin_port=htons(portaInt);

    if(inet_pton(AF_INET,indirizzo,&address.sin_addr) <=0)
    {
        perror("ip non valido");
        exit(EXIT_FAILURE);
    }

    int new_socket = startSocket(address);
    int c_socket;
    if((c_socket = accept(new_socket,(struct sockaddr*)&address,&addresslen)) < 0)
    {   
        perror("richiesta non accettata");
        exit(EXIT_FAILURE);
    }

    if(read(c_socket,stringa,sizeof(char)*1024) < 0)
    {
        perror("ricezione non avvenuta");
        exit(EXIT_FAILURE);
    }

    printf("%s \n",stringa);
    close(c_socket);
    close(new_socket);
}
