//
// Created by francesco-saletti on 26/07/24.
//

#include "TransferFunction.h"
#include "sys/socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

void readFile(int socket,char *path)
{
    int *flag = malloc(sizeof(int));
    unsigned char buffer[BUFFER_SIZE];

    FILE *file = fopen(path,"rb");

    if(file == NULL)
    {
        *flag = ERR_FILE_NOTFOUND;
        send(socket,flag,sizeof(int),0);
        perror("\nFile Non Trovato\n");
        return;
    }

    *flag = CONFIRM;
    send(socket,flag,sizeof(int),0);

    recv(socket,flag,sizeof(int),0);
    if(*flag == ERR_DIRECTORY_NOTFOUND)
    {
        printf("\nDirectory Non Esistente\n");
        return;
    }

    long size = getFileSize(path);
    send(socket,&size,sizeof(long),0);

    size_t n;
    while((n = fread(buffer,sizeof(char),BUFFER_SIZE,file)) > 0 )
    {
        send(socket,buffer,n,0);
        bzero(buffer,BUFFER_SIZE);
    }

    recv(socket,flag,sizeof(int),0);
    if(*flag == ERR_MEMORY_FULL)
    {
        printf("\nMemoria Piena\n");
        return;
    }

    printf("\ntrasmissione avvenuta\n");
    fclose(file);
}

void writeFile(int socket, char *path)
{
    int *flag = malloc(sizeof(int));
    unsigned char buffer[BUFFER_SIZE];
    char *dir =directoryName(path);
    struct stat info;
    recv(socket,flag,sizeof(int),0);
    if(*flag == ERR_FILE_NOTFOUND)
    {
        printf("\nFile Non Trovato\n");
        return;
    }

    if(stat(dir,&info) != 0)
    {
        *flag = ERR_DIRECTORY_NOTFOUND;
        send(socket,flag,sizeof(int),0);
        printf("\nDirectory Non Trovata\n");
        return;
    }

    *flag = CONFIRM;
    send(socket,flag,sizeof(int),0);

    FILE *file = fopen(path,"wb");
    if(file == NULL)
    {
        printf("Errore creazione file");
        return;
    }
    long size;
    recv(socket,&size,sizeof(long),0);
    if(checkMemory(size) == 0)
    {
        *flag = ERR_MEMORY_FULL;
        send(socket,flag,sizeof(int),0);
        printf("\nMemoria Piena\n");
        return;
    }
    size_t n = 0,temp;
    do
    {
        temp = recv(socket,buffer,BUFFER_SIZE,0);
        fwrite(buffer,sizeof(char),temp,file);
        n+=temp;
        bzero(buffer,BUFFER_SIZE);
    }
    while(n < size);
    fclose(file);

    printf("\nRicezione Avvenuta\n");
    return;

}

char *directoryName(const char *path)
{
    char path_copy[strlen(path)];

    // Copia del percorso perch+ dirname modifica la stringa passata
    strncpy(path_copy,path,sizeof(path_copy));
    path_copy[sizeof(path_copy)-1] = '\0'; //Mi Assicuro che la stringa sia null-terminated

    //Ottengo la directory del percorso
    char *dir = dirname(path_copy);
    if(strcmp(dir,".") == 0)
    {
        dir = "./";
    }
    return  dir;
}

long getFileSize(const char *filename)
{
    FILE *file = fopen(filename,"rb");
    long filesize = 0;
    if(file != NULL)
    {
        fseek(file,0,SEEK_END); //Mi sposto alla fine del file
        filesize = ftell(file);//Ottengo la posizione corrente del file
        fclose(file);
    }
    return  filesize;
}

int checkMemory(const size_t size)
{
    const char *path = "./"; //percorso corrente del server
    struct statvfs buf;
    if(statvfs(path,&buf) == 0)
    {
        unsigned long free_space = buf.f_bavail * buf.f_frsize; //spazio libero disponibile
        if(free_space < size)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    return 0;
}