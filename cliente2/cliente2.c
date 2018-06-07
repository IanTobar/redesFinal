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

int criaSocket(struct sockaddr_in *cliAddr, struct timeval *time_wait) {
    int sock, opt;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock <= 0) {
        printf("Erro ao iniciar socket\n");
        exit(1);
    }

    time_wait->tv_sec = 0;
    time_wait->tv_usec = 400000;
    opt = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void*) time_wait, sizeof (*time_wait));
    if (opt < 0) {
        printf("Erro ao definir temporizador de Resposta\n");
    }
    vincularPorta();

    return sock;
}

void vincularPorta(struct sockaddr_in cliAddr) {
    memset((char *) &cliAddr, 0, sizeof (cliAddr));
    cliAddr.sin_family = AF_INET;
    cliAddr.sin_port = htons(PORTA);
}

FILE * abrir_arquivo(char *arquivo) {
    FILE *arq;
    arq = fopen(arquivo, "rb");
    if (!arquivo) //VERIFICA SE O ARQUIVO FOI ABERTO
    {
        printf("Não foi Possivel Abrir o Arquivo Programa Finalizado\n");
        exit(1);
    }
    return arq;
}

pacote geraPacote(char mensagem[], int numerosequencia, int *indice) {
    pacote pac;
    int i = 0;
    for (i = 0; i < (*indice); i++) //ADICIONA OS DADOS AO PACOTE
        pac.dados[i] = mensagem[i];
    pac.checksum = checksum(pac.dados, *indice); //Chama checksum
    pac.tamDados = *indice; //Define Dimensão (Número de Bytes)
    pac.numSeq = numerosequencia; //Define Número de Sequência do Pacote
    return pac;
}

void vincularPorta(struct sockaddr_in cliAddr) {
    memset((char *) &cliAddr, 0, sizeof (cliAddr)); //define um buffer (Destino, caracter, tamanho)
    cliAddr.sin_family = AF_INET;
    cliAddr.sin_port = htons(PORTA);
}

FILE * abrir_arquivo(char *arquivo) {
    FILE *arq;
    arq = fopen(arquivo, "rb");
    if (!arquivo) //VERIFICA SE O ARQUIVO FOI ABERTO
    {
        printf("Não foi Possivel Abrir o Arquivo Execução Finalizada\n");
        exit(1);
    }
    return arq;
}

pacote geraPacote(char mensagem[], int numerosequencia, int *indice) {
    pacote pac;
    int i = 0;
    for (i = 0; i < (*indice); i++) //Adiciona dados ao pacote
        pac.dados[i] = mensagem[i];
    pac.checksum = checksum(pac.dados, *indice); //Chama checksum
    pac.tamDados = *indice; //Define tamanho de dados (Número de Bytes)
    pac.numSeq = numerosequencia; //Define Número de Sequência do Pacote
    return pac;
}

int main() {
    int sock, numSeq = 0;
    struct sockaddr_in cliAddr;
    struct timeval timeWait;
    char dados[90];
    pacote pac;
    FILE *arquivo;
    sock = criaSocket(&cliAddr, &timeWait);
    arquivo = abrir_arquivo(arquivo);

    pac = geraPacote();


}

