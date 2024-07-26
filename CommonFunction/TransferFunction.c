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
#include <dirent.h>
#include <errno.h>

void read(int socket,char *path)
{
    int *flag = malloc(sizeof(int));
    unsigned char buffer[BUFFER_SIZE];

    FILE *file = fopen(path,"rb");

    if(file == NULL)
    {
        *flag = ERR_FILE_NOTFOUND;
        send(socket,flag,sizeof(int),0);
        printf("\nFile Non Trovato\n");
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

    long *size = malloc(sizeof(long));
    *size = getFileSize(path);
    send(socket,size,sizeof(long),0);

    size_t n;
    do
    {
        n = fread(buffer,sizeof(char),BUFFER_SIZE,file);
        send(socket,buffer,BUFFER_SIZE,0);
    }
    while(n != BUFFER_SIZE);
    fclose(file);
    return;
}

void write(int socket, char *path)
{
    int *flag = malloc(sizeof(int));
    unsigned char buffer[BUFFER_SIZE];
    char *dir =directoryName(path);

    recv(socket,flag,sizeof(int),0);
    if(*flag == ERR_FILE_NOTFOUND)
    {
        printf("\nFile Non Trovato\n");
        return;
    }

    opendir(dir);
    if(ENOENT == errno)
    {
        *flag = ERR_DIRECTORY_NOTFOUND;
        send(socket,flag,sizeof(int),0);
    }

    FILE *file = fopen(path,"wb");

    long *size = malloc(sizeof(long));
    recv(socket,size,sizeof(long),0);

    long n = 0;
    do
    {
        recv(socket,buffer,BUFFER_SIZE,0);
        n = n + fwrite(buffer,sizeof(char),BUFFER_SIZE,file);
    }
    while(n < *size);
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

    //Ottengo la directory cdel percorso
    char *dir = dirname(path_copy);
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