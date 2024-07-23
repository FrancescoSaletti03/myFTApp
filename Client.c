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
#define PORT 8080
int main(int argx, char *argv[])
{
    int client_socket,stato;
    char stringa[] = "Ciao";
    struct sockaddr_in sAddress;

    socklen_t aAddresslen = sizeof(sAddress);
    sAddress.sin_family=AF_INET;
    sAddress.sin_port=htons(PORT);
    if(inet_pton(AF_INET,"127.0.0.1",&sAddress.sin_addr) <=0)
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
    send(client_socket,stringa,sizeof(char)*1024,0);
    close(client_socket);
}