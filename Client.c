//
// Created by francesco-saletti on 21/07/24.
//

#include "Client.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>

#include "CommonFunction/TransferFunction.h"
int main(int argc, char *argv[])
{
    enum OPERATION operazione;
    char *indirizzoIP = NULL, *porta = NULL, *from = NULL, *on = NULL;
    if(!(argc == 10 || argc == 8))
    {
        printf("formato non valido \n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < argc; i = i + 1)
    {
        if(strcmp(argv[i],"-w") == 0 && (i+1 >= argc || argv[i+1][0] == '-')) { operazione = WRITE; }
        else if (strcmp(argv[i],"-r") == 0 && (i+1 >= argc || argv[i+1][0] == '-')) { operazione = READ; }
        else if (strcmp(argv[i],"-l") == 0 && (i+1 >= argc || argv[i+1][0] == '-')) { operazione = LIST; }
        else if (strcmp(argv[i],"-a") == 0 && i+1 < argc && argv[i+1][0] != '-') { indirizzoIP = argv[i+1]; }
        else if (strcmp(argv[i],"-p") == 0 && i+1 < argc && argv[i+1][0] != '-') { porta = argv[i+1]; }
        else if (strcmp(argv[i],"-f") == 0 && i+1 < argc && argv[i+1][0] != '-') { from = argv[i+1]; }
        else if (strcmp(argv[i],"-o") == 0 && i+1 < argc && argv[i+1][0] != '-') { on = argv[i+1]; }
        else if(argv[i][0] != '-' && (i+1 >= argc || argv[i+1][0] == '-')) {}
        else
        {
            printf("formato non valido \n");
            exit(EXIT_FAILURE);
        }

    }

    if(on == NULL)
    {
        on = from;
    }


    int client_socket,stato;
    struct sockaddr_in sAddress;

    socklen_t aAddresslen = sizeof(sAddress);
    sAddress.sin_family=AF_INET;
    char *check;
    u_long portaInt = strtoll(porta,&check,10);
    if(*check != '\0'){
        perror("porta non valida");
        exit(EXIT_FAILURE);
    }

    sAddress.sin_port=htons(portaInt);

    if(inet_pton(AF_INET,indirizzoIP,&sAddress.sin_addr) <=0)
    {
        printf("ip non valido");
        return -1;
    }

    if ((client_socket = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        printf("\nfallimento creazione socket\n ");
        return  -1;
    }
    if((stato = connect(client_socket,(struct sockaddr*)&sAddress,aAddresslen)) < 0)
    {
        printf("\nfallimento connessione\n ");
        return  -1;
    }

    send(client_socket,&operazione,sizeof(enum OPERATION),0);

    if(operazione == READ)
    {
        size_t size = strlen(from);
        send(client_socket, &size, sizeof(size_t),0);
        send(client_socket,from,strlen(from),0);
        writeFile(client_socket,on);
    }
    else if (operazione == WRITE)
    {
        size_t size = strlen(on);
        send(client_socket, &size, sizeof(size_t),0);
        send(client_socket,on, strlen(on),0);
        readFile(client_socket,from);
    }
    else if(operazione == LIST)
    {
        size_t size = strlen(from);
        send(client_socket, &size, sizeof(size_t),0);
        send(client_socket,from,strlen(from),0);
        receiveList(client_socket);
    }
    close(client_socket);
}