//
// Created by francesco-saletti on 26/07/24.
//

#ifndef TRANSFERFUNCTION_H
#define TRANSFERFUNCTION_H
#define ERR_MEMORY_FULL 3
#define ERR_DIRECTORY_NOTFOUND 2
#define ERR_FILE_NOTFOUND 1
#define CONFIRM 0
#define BUFFER_SIZE 51200
#include <stdio.h>
enum OPERATION{WRITE,READ,LIST};

void readFile(int socket,char *path);
void writeFile(int socket, char *path);
char *directoryName(const char *path);
long getFileSize(const char *filename);
int checkMemory(const size_t size);
void sendList(int socket, char *path);
void receiveList(int socket);
int create_path(const char *path);

#endif //TRANSFERFUNCTION_H
