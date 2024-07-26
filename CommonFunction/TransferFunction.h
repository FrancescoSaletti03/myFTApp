//
// Created by francesco-saletti on 26/07/24.
//

#ifndef TRANSFERFUNCTION_H
#define TRANSFERFUNCTION_H
#define ERR_DIRECTORY_NOTFOUND 2
#define ERR_FILE_NOTFOUND 1
#define CONFIRM 0
#define BUFFER_SIZE 51200

void read(int socket,char *path);
void write(int socket, char *path);
char *directoryName(const char *path);
long getFileSize(const char *filename);

#endif //TRANSFERFUNCTION_H
