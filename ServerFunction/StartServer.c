//
// Created by francesco-saletti on 21/07/24.
//

#include "StartServer.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

int startSocket(struct sockaddr_in address)
{

    const int opt = 1;
    int new_socket;

    if ((new_socket = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("fallimento creazione socket");
        exit(EXIT_FAILURE);
    }
    /*if(setsockopt(new_socket,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT,&opt,sizeof(opt)))
    {
        perror("fallimento impostazioni socket");
        exit(EXIT_FAILURE);
    }*/
    if(setsockopt(new_socket,SOL_SOCKET,SO_REUSEPORT,&opt,sizeof(opt)))
    {
        perror("fallimento impostazioni socket");
        exit(EXIT_FAILURE);
    }

    if(bind(new_socket,(struct sockaddr*)&address,sizeof(address)) < 0 )
    {
        perror("binding fallito");
        exit(EXIT_FAILURE);
    }

    if(listen(new_socket,SOMAXCONN) < 0)
    {
        perror("fallimento nell'ascolto di richieste");
        exit(EXIT_FAILURE);
    }
    return new_socket;
}

void workDirectory(char *pathDirectory)
{
 DIR* directory = opendir(pathDirectory);
 if(ENOENT == errno)
 {
     mkdir(pathDirectory,0777);
 }
 else
 {
     closedir(directory);
 }
    chdir(pathDirectory);
}