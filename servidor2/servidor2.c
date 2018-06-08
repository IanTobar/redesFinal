#include <stdio.h>
#include <netdb.h>// definições para operações de banco de dados de rede. Pode disponibilizar o tipo in_port_t e o tipo in_addr_t
#include <errno.h>//números de erro do sistema
#include <stdlib.h>
#include <string.h>
#include <unistd.h>//define diversas constantes e tipos simbólicos e declara funções diversas.
#include <arpa/inet.h>//definições para operações da internet.  disponibiliza o tipo in_port_t e o tipo in_addr_t. disponibiliza a estrutura in_addr
#include <sys/types.h>//tipos de dados
#include <sys/socket.h>//cabeçalho dos sockets principais
#include <netinet/in.h>//Família de protocolos da Internet

#define porta 5000
#define buffer 100

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

int criaSocket(struct sockaddr_in *addr, struct timeval *time_wait) {
    int sock, opt;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock <= 0) {
        printf("Erro na abertura do socket: %s\n", strerror(errno)); //mensagem de erro ao abrir socket 
        exit(1); //sai do programa 
    }

    time_wait->tv_sec = 0;
    time_wait->tv_usec = 400000;
    opt = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void*) time_wait, sizeof (*time_wait));
    if (opt < 0) {
        printf("Erro ao definir temporizador de Resposta\n");
    }


    return sock;
}

void bindSocket(struct sockaddr_in *servAddr, int sock) {
    int rc;
    memset((char *) &servAddr, 0, sizeof (servAddr)); //define um buffer (Destino, caracter, tamanho) 
    servAddr->sin_family = AF_INET; //familia de endereços 
    servAddr->sin_port = htons(porta); //define a porta
    servAddr->sin_addr.s_addr = htonl(INADDR_ANY);
    rc = bind(sock, (struct sockaddr *) &servAddr, sizeof (servAddr));
    if (rc < 0) { //verifica se o bind do socket a portafuncionou corretamente

        printf("Não foi possivel vincular porta\n");
        exit(1);
    }
}

pacote geraPacote(char *mensagem, int numerosequencia, int *indice) {
    pacote pac;
    int i = 0;
    // for (i = 0; i < (*indice); i++) //adiciona os dados ao pacote
    //pac.dados[i] = mensagem[i];
    strcpy(pac.dados, mensagem);
    pac.checksum = checksum(pac.dados, *indice); //Chama checksum
    pac.tamDados = *indice; //Define Dimensão (Número de Bytes)
    pac.numSeq = numerosequencia; //Define Número de Sequência do Pacote
    return pac;
}

char* funcPesquisaArquivo(char* pesquisaArquivo) {

    char resultadoArquivo[51]; //resultado do arquivo pesquisado no servidor 
    char* resultadoIp = (char*) calloc(16, sizeof (char)); //armazena o resultado do ip achado no arquivo de pesquisa 

    FILE *ptarq; //ponteiro para manipular arquivo 

    //abre arquivo em modo de leitura 
    ptarq = fopen("arquivos.txt", "r");

    //verifica se deu erro ao abrir o arquivo 
    if (ptarq == NULL) {
        printf("\n\nErro ao abrir arquivo\n\n");
        exit(0);
    }

    //pesquisa todo o arquivo de dados do servidor 
    while ((fscanf(ptarq, "%s %s\n", resultadoArquivo, resultadoIp)) != EOF)
        //!feof(ptarq))//Enquanto não chegar no final do arquivo 
    {

        //se encontrou o arquivo pesquisado 
        if (strcmp(resultadoArquivo, pesquisaArquivo) == 0) {
            printf("O arquivo %s esta localizado no IP %s\n", resultadoArquivo, resultadoIp);
        } else {//senão encontrou 
            printf("Arquivo nao encontrado!!!\n");
            exit(0);
        }

        //fecha arquivo 
        fclose(ptarq);

        //retorna o ip da máquina que contém arquivo 
        return resultadoIp;

    }

}

int main() {
    struct sockaddr_in servAddr, cliAddr;
    pacote pctRec, pctEnv;
    int sock, numSeq = 0;
    struct timeval tempo;
    char resultadoIP[16];
    //CRIA SOCKET NO SERVIDOR E FAZ SEU BIND PARA RECEBER DADOS ATRAVÉS DELE
    sock = criaSocket(&servAddr, &tempo);
    bindSocket(&servAddr, sock);

    //RECEBE SOLICITAÇÃO DO CLIENTE
    recvfrom(sock, &pctRec, sizeof (pctRec), 0, (struct sockaddr *) &servAddr, sizeof (servAddr));

    //PRINTA INFORMAÇÕES PARA TESTE
    printf("Origem do cliente %s\n", inet_ntoa(cliAddr.sin_addr));
    printf("mensagem: %s\n", pctRec.dados); //imprime a mensagem

    //chama função para pesquisar o nome do arquivo. Será retornado o IP do cliente que possui arquivo
    strcpy(resultadoIP, funcPesquisaArquivo(pctRec.dados));

    pctEnv = geraPacote(resultadoIP, numSeq, 16);

    sendto(sock, &pctEnv, sizeof (pacote), 0, (struct sockaddr *) &cliAddr, sizeof (cliAddr));

    return 0;
}