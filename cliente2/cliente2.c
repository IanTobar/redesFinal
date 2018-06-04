#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include <errno.h>
#include<arpa/inet.h>

typedef struct pacote {
    int numSeq; //número de sequência do pacote
    long int checksum; //Numero de verificação do pacote
    int tamDados; // tamanho da palavra
    char dados[90]; //vetor de dados
} pacote;

typedef struct ak {
    int status; // status 1 = ACK 2 = NACK
    int numSeq; ////número de sequência do pacote
};

long int checksum() {
    long int soma;
    int asc, i;
    soma = 0;

}

