//
// Created by Francesco Saletti on 07/08/24.
//

#include "ConcurrenceFunction.h"
#include "../CommonFunction/TransferFunction.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>

struct Counters *head = NULL;
struct Counters *tail = NULL;
pthread_mutex_t mutexStruct = PTHREAD_MUTEX_INITIALIZER;

void* startTread(void* socket){
    enum OPERATION operazione;
    int flag;
    int c_socket = *(int*) socket;
    size_t strlen;
    char filePath[PATH_MAX] = {0};
    struct Counters *counters;

    recv(c_socket,&operazione,sizeof(enum OPERATION),0);
    recv(c_socket, &strlen, sizeof(size_t) , 0);
    recv(c_socket,filePath,strlen,0);

    if(operazione == READ)
    {
        if((counters = GetCounters(filePath))==NULL)
        {
            flag = ERR_FILE_NOTFOUND;
            send(c_socket,&flag,sizeof(int),0);
            perror("File Non Trovato\n");
        }
        else
        {   readLock(counters);
            readFile(c_socket,filePath);
            readUnlock(counters);
        }
    }
    else if(operazione == WRITE)
    {
        counters = checkAndCreateFile(filePath);
        writeLock(counters);
        writeFile(c_socket,filePath);
        writeUnlock(counters);
    }
    else if(operazione == LIST)
    {
        if((counters = GetCounters(filePath)) == NULL)
        {
            flag = ERR_DIRECTORY_NOTFOUND;
            printf("Impossibile aprire la directory\n");
            send(c_socket,&flag,sizeof(int),0);
        }
        else
        {   readLock(counters);
            sendList(c_socket,filePath);
            readUnlock(counters);
        }
    }
    close(c_socket);

    return NULL;
}
void readLock(struct Counters *counters)
{
    pthread_mutex_lock(& counters -> mutexCounter);
    counters -> usage +=1;
    while(counters -> AcceptRequest == 0)
    {
        pthread_cond_wait(& counters -> wake, & counters -> mutexCounter);
    }
    counters -> readPending += 1;

    pthread_rwlock_rdlock(& counters->mutexFile);
    pthread_mutex_unlock(& counters  -> mutexCounter);
}

void readUnlock(struct Counters *counters)
{
    pthread_mutex_lock(& counters -> mutexCounter);

    counters -> readPending -= 1;
    counters -> usage -=1;
    counters -> readCompleted +=1;

    pthread_rwlock_unlock(& counters->mutexFile);
    pthread_mutex_unlock(& counters  -> mutexCounter);

}

void writeLock(struct Counters *counters)
{
    pthread_mutex_lock(& counters -> mutexCounter);
    counters -> usage +=1;
    while(counters->readPending > 0 || counters->readCompleted > 3)
    {
        pthread_cond_wait(& counters -> wake, & counters -> mutexCounter);
    }
    counters -> AcceptRequest = 0;

    pthread_rwlock_wrlock(& counters -> mutexFile);
    pthread_mutex_unlock(& counters  -> mutexCounter);
}

void writeUnlock(struct Counters *counters)
{
    pthread_mutex_lock(& counters -> mutexCounter);
    counters -> AcceptRequest = 1;
    counters -> usage -=1;
    counters -> readPending = 0;

    pthread_rwlock_unlock(& counters -> mutexFile);
    pthread_mutex_unlock(& counters  -> mutexCounter);
}

struct Counters* GetCounters(char* filePath)
{
    pthread_mutex_lock(&mutexStruct);
    struct stat info;
    if(stat(filePath, &info) != 0)
    {
        pthread_mutex_unlock(&mutexStruct);
        return NULL;
    }
    struct Counters* temp = head;
    while(temp != NULL)
    {
        if(temp -> fd == info.st_ino)
        {
            pthread_mutex_unlock(&mutexStruct);
            return temp;
        }
        temp = temp -> nextCounters;
    }
    temp = malloc(sizeof(struct Counters));
    temp -> fd = info.st_ino;
    temp -> AcceptRequest = 1;
    pthread_mutex_init(& temp->mutexCounter,NULL);
    pthread_rwlock_init(& temp->mutexFile, NULL);
    pthread_cond_init(& temp->wake, NULL);


    if(head == NULL)
    {
        head = temp;
        tail = temp;
    }
    else
    {
        tail -> nextCounters = temp;
        tail = temp;
    }
    pthread_mutex_unlock(&mutexStruct);
    return temp;
}

struct Counters* checkAndCreateFile(char* filePath)
{

    char *temp = filePath;
    struct Counters *counters;
    if((counters = GetCounters(filePath)) != NULL)
    {
        return counters;
    }
    do
    {
        temp = directoryName(temp);

    }
    while(access(temp,F_OK) != 0);

    counters = GetCounters(temp);

    writeLock(counters);

    create_path(directoryName(filePath));
    FILE *file = fopen(filePath,"wb");
    writeUnlock(counters);

    return GetCounters(filePath);
}
void clearCounters(struct Counters *counters)
{
    pthread_mutex_lock(&mutexStruct);
    struct Counters *previous = NULL;
    if(head -> fd == counters -> fd)
    {
        head = head -> nextCounters;
        free(counters);
        return;
    }
    struct Counters *temp = head -> nextCounters;
    previous = head;

    while(temp!=NULL || temp -> fd != counters -> fd)
    {   previous = previous -> nextCounters;
        temp = temp -> nextCounters;
    }
    previous -> nextCounters = temp -> nextCounters;
    free(temp);
    readUnlock(counters);
    pthread_mutex_unlock(&mutexStruct);
}






