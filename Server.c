#include <stdio.h>#include <stdlib.h>#include <sys/socket.h>#include <netinet/in.h>#include <arpa/inet.h>#include <unistd.h>#include "ServerFunction/StartServer.h"#include "ServerFunction/ConcurrenceFunction.h"#include "Server.h"#include <string.h>#include <pthread.h>#include "CommonFunction/TransferFunction.h"int main(int argc, char *argv[]){    char *indirizzo,*porta,*directory;    if(argc != 7 || argv[1][0] != '-' || argv[3][0] != '-' || argv[5][0] != '-') {        perror("Formato non valido\n");        exit(EXIT_FAILURE);    }    for(int i = 1; i < argc; i = i + 2)    {        if(strcmp(argv[i],"-a") == 0)        {indirizzo = argv[i+1];}        if(strcmp(argv[i],"-p") == 0)        {porta = argv[i+1];}        if(strcmp(argv[i],"-d") == 0)        {directory = argv[i+1];}    }    workDirectory(directory);    char *check;    struct sockaddr_in address;    socklen_t addresslen = sizeof(address);    address.sin_family=AF_INET;    u_long portaInt = strtoll(porta,&check,10);    if(*check != '\0'){        perror("Porta non valida\n");        exit(EXIT_FAILURE);    }    address.sin_port=htons(portaInt);    if(inet_pton(AF_INET,indirizzo,&address.sin_addr) <=0)    {        perror("Indirizzo Ip non valido\n");        exit(EXIT_FAILURE);    }    int new_socket = startSocket(address);    int c_socket;    pthread_t thread;    while(1){        if((c_socket = accept(new_socket,(struct sockaddr*)&address,&addresslen)) < 0)        {            perror("Richiesta non accettata\n");            exit(EXIT_FAILURE);        }        else        {            if (pthread_create(&thread, NULL, startTread, &c_socket) != 0) {                perror("Errore nella creazione del thread\n");                return EXIT_FAILURE;            }        }    }    close(new_socket);}