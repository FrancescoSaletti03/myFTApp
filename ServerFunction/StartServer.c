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
        perror("Fallimento creazione socket\n");
        exit(EXIT_FAILURE);
    }
    if(setsockopt(new_socket,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT,&opt,sizeof(opt)))
    {
        perror("Fallimento impostazioni socket\n");
        exit(EXIT_FAILURE);
    }
    /*if(setsockopt(new_socket,SOL_SOCKET,SO_REUSEPORT,&opt,sizeof(opt)))
    {
        perror("fallimento impostazioni socket");
        exit(EXIT_FAILURE);
    }*/

    if(bind(new_socket,(struct sockaddr*)&address,sizeof(address)) < 0 )
    {
        perror("Binding fallito\n");
        exit(EXIT_FAILURE);
    }

    if(listen(new_socket,SOMAXCONN) < 0)
    {
        perror("Fallimento nell'ascolto di richieste\n");
        exit(EXIT_FAILURE);
    }
    return new_socket;
}

void workDirectory(char *pathDirectory)
{
     DIR* directory = opendir(pathDirectory);
     if(ENOENT == errno)
     {
         if(mkdir(pathDirectory,0777) < 0)
         {
             perror("Path per la creazione della cartella non valido\n");
             exit(EXIT_FAILURE);
         }
     }
     else
     {
         closedir(directory);
     }
     chdir(pathDirectory);
}