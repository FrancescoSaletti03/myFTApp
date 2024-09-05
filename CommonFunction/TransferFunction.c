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
#include <dirent.h>
#include <errno.h>
#include <limits.h>

void readFile(int socket,char *path)
{
    int flag;
    unsigned char buffer[BUFFER_SIZE];

    FILE *file = fopen(path,"rb");

    if(file == NULL)
    {
        flag = ERR_FILE_NOTFOUND;
        send(socket,&flag,sizeof(int),0);
        perror("File Non Trovato\n");
        return;
    }

    flag = CONFIRM;
    send(socket,&flag,sizeof(int),0);

    recv(socket,&flag,sizeof(int),0);
    if(flag == ERR_DIRECTORY_NOTFOUND)
    {
        printf("Directory Non Esistente\n");
        return;
    }

    recv(socket,&flag,sizeof(int),0);
    if(flag == ERR_FILE_NOTFOUND)
    {
        printf("Errore creazione file");
        return;
    }

    long size = getFileSize(path);
    send(socket,&size,sizeof(long),0);

    recv(socket,&flag,sizeof(int),0);
    if(flag == ERR_MEMORY_FULL)
    {
        printf("Memoria Piena\n");
        return;
    }

    size_t n;
    while((n = fread(buffer,sizeof(char),BUFFER_SIZE,file)) > 0 )
    {
        send(socket,buffer,n,0);
        bzero(buffer,BUFFER_SIZE);
    }

    printf("Trasmissione avvenuta\n");
    fclose(file);
    return;
}

void writeFile(int socket, char *path)
{
    int flag;
    unsigned char buffer[BUFFER_SIZE];
    char *dir =directoryName(path);
    struct stat info;
    recv(socket,&flag,sizeof(int),0);
    if(flag == ERR_FILE_NOTFOUND)
    {
        printf("File Non Trovato\n");
        return;
    }

    if(create_path(dir)!= 0)
    {
        flag = ERR_DIRECTORY_NOTFOUND;
        send(socket,&flag,sizeof(int),0);
        printf("Impossibile Creare la directory\n");
        return;
    }

    flag = CONFIRM;
    send(socket,&flag,sizeof(int),0);

    FILE *file = fopen(path,"wb");
    if(file == NULL)
    {
        flag = ERR_FILE_NOTFOUND;
        printf("Errore creazione file");
        send(socket,&flag,sizeof(int),0);
        return;
    }
    flag = CONFIRM;
    send(socket,&flag,sizeof(int),0);
    long size;
    recv(socket,&size,sizeof(long),0);
    if(checkMemory(size) == 0)
    {
        flag = ERR_MEMORY_FULL;
        send(socket,&flag,sizeof(int),0);
        printf("Memoria Piena\n");
        return;
    }
    flag = CONFIRM;
    send(socket,&flag,sizeof(int),0);

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

    printf("Ricezione Avvenuta\n");
    return;

}

void sendList(int socket, char *path)
{
    struct dirent *de;// Dichiarazione di un puntatore per il contenuto della directory

    DIR *dr= opendir(path);
    int flag;
    size_t len;


    if(dr == NULL)
    {
        flag = ERR_DIRECTORY_NOTFOUND;
        printf("Impossibile aprire la directory\n");
        send(socket,&flag,sizeof(int),0);
        return;
    }

    flag = CONFIRM;
    send(socket,&flag,sizeof(int),0);

    //usa readdir() per leggere il contenuto della directory
    while((de = readdir(dr)) != NULL)
    {
        len = strlen(de->d_name);
        send(socket,&len,sizeof(size_t),0);
        send(socket,de->d_name,strlen(de->d_name),0);
    }

    closedir(dr);
    return;
}

void receiveList(int socket)
{
    char stringa[FILENAME_MAX];
    size_t len = 0;
    int flag;
    recv(socket,&flag,sizeof(int),0);
    if(flag == ERR_DIRECTORY_NOTFOUND)
    {
        printf("Impossibile aprire la directory\n");
        return;
    }

    while (recv(socket,&len,sizeof(size_t),0) > 0)
    {
        bzero(stringa,FILENAME_MAX);
        recv(socket,stringa,len,0);
        printf("%s\n",stringa);
    }

    return;
}

char *directoryName(const char *path)
{
    char path_copy[PATH_MAX];

    // Copia del percorso perchè dirname modifica la stringa passata
    strncpy(path_copy,path,PATH_MAX);
    path_copy[PATH_MAX-1] = '\0'; //Mi Assicuro che la stringa sia null-terminated

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

int create_path(const char *path)
{
    int result = 0;

    //controllo se la directory esiste
    if(access(path,F_OK)!=0)
    {
        char *fdir = malloc(PATH_MAX);
        strcpy(fdir,path);

        if(strcmp(path,".")!=0)
        {
            //creo la directory se non esiste
            if(create_path(dirname(fdir)) == -1)
            {
                result = -1;
            }
        }

        //provo a crare la directory se fallisce, esce con un errore
        if(mkdir(path,S_IRWXU | S_IRWXG) == -1 && errno != EEXIST)
        {
            perror("Errore creazione Directory\n");
            result = -1;
        }
        free(fdir);
    }
    return result;
}
