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

    //ricevo le informazioni dalla client del socket, tra cui tipo di operazione, grandezza del file  path e path vero e proprio
    recv(c_socket,&operazione,sizeof(enum OPERATION),0);
    recv(c_socket, &strlen, sizeof(size_t) , 0);
    recv(c_socket,filePath,strlen,0);

    //controllo quale operazione deve essere eseguita
    if(operazione == READ)
    {
        //vado ad inizializzare un'istanza della mia struttura per la gestione della concorrenza
        if((counters = GetCounters(filePath))==NULL)
        {
            //se non si crea l'istanza, genero un errore
            flag = ERR_FILE_NOTFOUND;
            send(c_socket,&flag,sizeof(int),0);
            perror("File Non Trovato\n");
        }
        else
        {
            //lock sul file di tipo read e aggiornamento dell'istanza della struttura
            readLock(counters);

            // fase di invio del file al client
            readFile(c_socket,filePath);

            //unlock sul file di tipo read e aggiornamento dell'istanza della struttura
            readUnlock(counters);
        }
    }
    else if(operazione == WRITE)
    {
        //vado a creare il file su cui devo scrivere se non esiste, e inizializzo un'istanza della mia struttura
        counters = checkAndCreateFile(filePath);

        //lock sul file di tipo write e aggiornamento dell'istanza della struttura
        writeLock(counters);

        // fase di ricezione del file dal client e scrittura sul server
        writeFile(c_socket,filePath);

        //lock sul file di tipo write e aggiornamento dell'istanza della struttura
        writeUnlock(counters);
    }
    else if(operazione == LIST)
    {
        //vado ad inizializzare un'istanza della mia struttura per la gestione della concorrenza
        if((counters = GetCounters(filePath)) == NULL)
        {
            //se non si crea l'istanza, genero un errore
            flag = ERR_DIRECTORY_NOTFOUND;
            printf("Impossibile aprire la directory\n");
            send(c_socket,&flag,sizeof(int),0);
        }
        else
        {    //lock sul file di tipo read e aggiornamento dell'istanza della struttura
            readLock(counters);

            // fase di invio del file al client
            sendList(c_socket,filePath);

            //unlock sul file di tipo read e aggiornamento dell'istanza della struttura
            readUnlock(counters);
        }
    }
    close(c_socket);

    return NULL;
}
void readLock(struct Counters *counters)
{
    //vado a fare il lock l'istanza della struttura in modo tale ches solo un tread per volta può aggiornarla
    pthread_mutex_lock(& counters -> mutexCounter);
    counters -> usage +=1;

    //attendo che sia possibile accettare nuove richieste di lettura
    while(counters -> AcceptRequest == 0)
    {
        pthread_cond_wait(& counters -> wake, & counters -> mutexCounter);
    }
    counters -> readPending += 1;

    //chiedo il read-lock sul semaforo associato al file
    pthread_rwlock_rdlock(& counters->mutexFile);

    //faccio unlock dell'istanza della struttura
    pthread_mutex_unlock(& counters  -> mutexCounter);
}

void readUnlock(struct Counters *counters)
{
    //vado a fare il lock l'istanza della struttura in modo tale ches solo un tread per volta può aggiornarla
    pthread_mutex_lock(& counters -> mutexCounter);

    //aggiorno i contatori
    counters -> readPending -= 1;
    counters -> usage -=1;
    counters -> readCompleted +=1;

    //faccio l'unlock sul file associato
    pthread_rwlock_unlock(& counters->mutexFile);

    //faccio unlock dell'istanza della struttura
    pthread_mutex_unlock(& counters  -> mutexCounter);

}

void writeLock(struct Counters *counters)
{
    pthread_mutex_lock(& counters -> mutexCounter);
    counters -> usage +=1;

    //attendo che non ci siano richieste di lettura o che quelle completate siano maggiori di 3, prima di fare la write
    while(counters->readPending > 0 || counters->readCompleted > 3)
    {
        pthread_cond_wait(& counters -> wake, & counters -> mutexCounter);
    }
    counters -> AcceptRequest = 0;

    //faccio write-lock sul file associato
    pthread_rwlock_wrlock(& counters -> mutexFile);
    pthread_mutex_unlock(& counters  -> mutexCounter);
}

void writeUnlock(struct Counters *counters)
{
    pthread_mutex_lock(& counters -> mutexCounter);

    //resetto i contatori in modo tale da poter riiniziare ad effettuare richieste di read
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






