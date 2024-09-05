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

    //vado a creare la socket del mio server, in modo tale che i client possano comunicare con il server
    if ((new_socket = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        //nel caso non riesce a crearlo, genera un errore
        perror("Fallimento creazione socket\n");
        exit(EXIT_FAILURE);
    }
    //vado ad impostare le opzioni della mia socket server
    if(setsockopt(new_socket,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT,&opt,sizeof(opt)))
    {
        //nel caso non riesce a impostare le opzioni, genera un errore
        perror("Fallimento impostazioni socket\n");
        exit(EXIT_FAILURE);
    }

    //collego la socket del server, all`Indirizzo Ip e alla Porta contenuti nella struct sockaddr
    if(bind(new_socket,(struct sockaddr*)&address,sizeof(address)) < 0 )
    {
        //nel caso il collegamento non va a buon fine, genero un errore
        perror("Binding fallito\n");
        exit(EXIT_FAILURE);
    }

    //metto il server in ascolto, pronto a ricevere connessioni in entrata
    if(listen(new_socket,SOMAXCONN) < 0)
    {
        //se non riesce ad entrare in ascolto, genera un errore
        perror("Fallimento nell'ascolto di richieste\n");
        exit(EXIT_FAILURE);
    }
    return new_socket;
}

void workDirectory(char *pathDirectory)
{
     DIR* directory = opendir(pathDirectory);

     //controllo se la directory esiste o meno, nel caso la creo.
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
     //imposto la cartella come cartella di lavoro
     chdir(pathDirectory);
}