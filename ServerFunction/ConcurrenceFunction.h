//
// Created by Francesco Saletti on 07/08/24.
//

#ifndef MYFTA_CONCURRENCEFUNCTION_H
#define MYFTA_CONCURRENCEFUNCTION_H
#include <pthread.h>

void* startTread(void* c_socket);
struct Counters* GetCounters(char* filePath);
struct Counters* checkAndCreateFile(char* filePath);
void readLock(struct Counters *counters);
void readUnlock(struct Counters *counters);
void writeLock(struct Counters *counters);
void writeUnlock(struct Counters *counters);
void clearCounters(struct Counters *counters);

struct Counters
{
    int fd;
    int readPending;
    int readCompleted;
    int usage;
    int AcceptRequest;
    struct Counters *nextCounters;
    pthread_mutex_t mutexCounter;
    pthread_rwlock_t mutexFile;
    pthread_cond_t wake;

};
#endif //MYFTA_CONCURRENCEFUNCTION_H
