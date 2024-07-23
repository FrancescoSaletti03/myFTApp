//
// Created by francesco-saletti on 21/07/24.
//

#ifndef STARTSOCKET_H
#define STARTSOCKET_H
#include <netinet/in.h>
int startSocket(struct sockaddr_in address);
void workDirectory(char *pathDirectory);
#endif //STARTSOCKET_H
