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
//#include <linux/limits.h>
#include <limits.h>

void* startTread(void* socket){
    enum OPERATION operazione;
    int c_socket = *(int*) socket;
    size_t strlen;
    char filePath[PATH_MAX] = {0};

    recv(c_socket,&operazione,sizeof(enum OPERATION),0);
    recv(c_socket, &strlen, sizeof(size_t) , 0);
    recv(c_socket,filePath,strlen,0);

    if(operazione == READ)
    {
        readFile(c_socket,filePath);
    }
    else if(operazione == WRITE)
    {
        writeFile(c_socket,filePath);
    }
    else if(operazione == LIST)
    {
        sendList(c_socket,filePath);
    }
    close(c_socket);

    return NULL;
}