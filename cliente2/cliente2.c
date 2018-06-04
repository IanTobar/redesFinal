#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include <errno.h>
#include<arpa/inet.h>


#define PORTA 5000

typedef struct pacote {
    int numSeq; //número de sequência do pacote
    long int checksum; //Numero de verificação do pacote
    int tamDados; // tamanho da palavra
    char dados[90]; //vetor de dados
} pacote;

typedef struct ck {
    int status; // status 1 = ACK 2 = NACK
    int numSeq; ////número de sequência do pacote
};

long int checksum(char dados[], int tamdados) {
    long int soma;
    int asc, i;
    soma = 0;
    for (i = 0; i < tamdados; i++) { //percorre vetor de dados para cálcular o checksum
        asc = (int) dados[i]; //transforma palavra em valor inteiro seguindo a tabela ascii
        soma += asc; //adiciona a soma
    }
    return soma; //retorna a soma (checksum)
}

int criaSocket(struct sockaddtr_in *cliAddr, struct timeval time_wait) {
    int sock, opt;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock <= 0) {
        printf("Erro ao iniciar socket\n");
        exit(1);
    }

    time_wait->tv_sec = 0;
    time_wait->tv_usec = 400000;
    opt = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void *) time_wait, sizeof (*time_wait));
    if (opt < 0) {
        printf("Erro ao definir temporizador de Resposta\n");
    }
    vincularPorta();

    return sock;
}

void vincularPorta(struct sockaddr_in cliAddr) {

    cliAddr.sin_family = AF_INET;
    cliAddr.sin_port = htons(PORTA);
}

FILE * abrir_arquivo(char *arquivo[]) {
    FILE *arq;
    arq = fopen(caminho, "rb");
    if (!arquivo) //VERIFICA SE O ARQUIVO FOI ABERTO
    {
        printf("Não foi Possivel Abrir o Arquivo Execução Finalizada\n");
        exit(1);
    }
    return arquivo;
}



