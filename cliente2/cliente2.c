#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include <errno.h>
#include<arpa/inet.h>

typedef struct pacote {
    int num;
    long int checksum;
    char dados[90];
} pacote;

