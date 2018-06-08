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
    int status; // status 0= ACK 1 = NACK
    int numSeq; ////número de sequência do pacote
} ck;

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

int enviaArquivo(FILE *arquivo, char *dados, struct sockaddr_in *servAddr, int sock, int numSeq, int *tamanho) {
    ck resp;
    int aux, n, i = 0;
    char vet[90];
    memset(&resp, 0x0, sizeof (resp));
    aux = sizeof (*servAddr);
    n = recvfrom(sock, &resp, sizeof (ck), 0, (struct sockaddr*) servAddr, &aux);
    if (n < 0) { //VERIFICA RESPOSTA DO SERVIDOR
        printf(" RESPOSTA DO SERVIDOR NAO RECEBIDA!\n");
    } else {
        if (resp.status) {
            printf("pacote %d entregue ao servidor\n", numSeq);
            *tamanho = fread(&vet, sizeof (char), 90, arquivo);
            if (tamanho == 0) {
                free(dados);
                dados = (char*) calloc(3, sizeof (char));
                dados[0] = 'F';
                dados[1] = 'I';
                dados[2] = 'M';
            } else {
                free(dados);
                dados = (char*) calloc(*tamanho, sizeof (char));
                for (i = 0; i < (*tamanho); i++) {
                    dados[i] = vet[i];
                }
            }
            return numSeq + 1;
        } else {

            printf("PACOTE %d COM ERRO DETECTADO . Reenvio em andamento..\n", numSeq);
            return numSeq;
        }
    }
}

int main(int argc, char** argv) {
    int sock, cliTam, status, rc, numSeq = 0, op, tamanho, i;
    char pesquisaArquivo[51], ipArq[16], vet[90], *dados;
    struct sockaddr_in cliAddr, servAddr;
    struct hostent * host;
    long int check;
    struct timeval tempo;
    FILE *arquivo;
    pacote pct, pct2;
    ck resp;
    cliTam = sizeof (cliAddr);
    printf("1-Baixar Arquivo\n2-Seeder");
    scanf("%d", op);
    switch (op) {
        case 1:
            //CRIA SOCKET PARA RECEBER DADOS
            sock = criaSocket(&cliAddr, &tempo); //Cria Socket que  será utilizado pelo cliente para receber o arquivo
            bindSocket(&cliAddr, sock); //Faz o bind do socket a uma porta local para recebimento de dados

            //INTERAGE COM O USUÁRIO PARA OBTER NOME DO ARQUIVO QUE SE DESEJA RECEBER
            printf("Digite o nome do arquivo que deseja baixar\n"); //solicita ao usuário nome do arquivo que se deseja baixar
            gets(pesquisaArquivo); //armazena esse nome na variável pesquisa arquivo

            //CONECTA AO SERVIDOR PARA RECEBER IP DA MAQUINA QUE CONTÉM O ARQUIVO DESEJADO
            host = gethostbyname(argv[1]);
            if (host == NULL) {
                printf("impossivel conectar ao host '%s', Finalizando Programa... \n", argv[1]);
                exit(1);
            }
            servAddr.sin_family = host->h_addrtype;
            memcpy((char *) &servAddr.sin_addr.s_addr, host.h_addr_list[0], host->h_length); //COPIANDO DADOS DO HOST PARA O END. SERVIDOR
            servAddr.sin_port = htons(porta);

            // COMUNICA COM SERVIDOR PARA PESQUISAR O ARQUIVO E SOLICITAR O IP DO SEEDER
            pct2 = geraPacote(pesquisaArquivo, numSeq, strlen(pesquisaArquivo)); //gera pacote que será enviado ao servidor
            numSeq++;
            sendto(sock, &pct2, sizeof (pacote), 0, (struct sockaddr *) &servAddr, sizeof (servAddr));

            memset(&pct, 0x0, buffer); //inicia o Buffer

            status = recvfrom(sock, &pct, sizeof (pacote), 0, (struct sockaddr *) &cliAddr, &cliTam); //recebe pacote e armazena valor do status,positivo = sucesso 

            check = checksum(pct.dados, pct.tamDados); //Calcula Checksum do pacote recebido

            if ((check == pct.checksum) && (pct.dados != 0)) {
                strcpy(ipArq, pct.dados); //copia os dados do pacote de resposta do servidor para a variável ipArq 
            } else {
                printf("Arquivo solicitado não foi encontrado\n");
                exit(0);
            }


            //CONECTA AO CLIENTE QUE POSSUI O ARQUIVO PARA SOLICITAR O ENVIO DO ARQUIVO DESEJADO
            host = gethostbyname(ipArq);
            if (host == NULL) {
                printf("impossivel conectar ao host '%s', Finalizando Programa... \n", ipArq);
                exit(1);
            }
            servAddr.sin_family = host->h_addrtype;
            memcpy((char *) &servAddr.sin_addr.s_addr/*PARA QUEM VAO OS DADOS*/,
                    host.h_addr_list[0] /*DE ONDE VAO OS DADOS*/, host.h_length /*numero de bytes*/); //COPIANDO DADOS DO HOST PARA O END. SERVIDOR
            servAddr.sin_port = htons(porta);


            //CRIAÇÃO DO ARQUIVO FINAL ONDE OS DADOS RECEBIDOS SERÃO PERSISTIDOS
            arquivo = fopen(pesquisaArquivo, "wb"); //Cria o arquivo no cliente (arquivo que serão salvados os dados)
            //Arquivo é criado de forma binária já que estamos tratando arquivos de audio MP3
            if (!arquivo) //Verifica se foi possível criar o arquivo
            {
                printf("Não foi Possível criar o Arquivo para Recebimento dos dados\n"); //mensagem de erro
                return 1; //encerra o programa
            }


            //INTERAGE COM CLIENTE QUE POSSUI ARQUIVO PARA SOLICITAR ENVIO DE DADOS
            pct2 = geraPacote(pesquisaArquivo, numSeq, strlen(pesquisaArquivo)); //gera pacote que será enviado ao servidor
            sendto(sock, &pct2, sizeof (pacote), 0, (struct sockaddr *) &servAddr, sizeof (servAddr));
            numSeq++;

            pct2 = geraPacote(ipArq, numSeq, strlen(ipArq)); //gera pacote que será enviado ao servidor
            sendto(sock, &pct2, sizeof (pacote), 0, (struct sockaddr *) &servAddr, sizeof (servAddr));
            numSeq++;



            //LOOP PARA RECEBER PACOTES COM OS DADOS
            while (strcmp(pct.dados, "FIM")) //loop para receber o arquivo, finaliza ao receber Palavra FIM
            {
                memset(&pct, 0x0, buffer); //inicia o Buffer
                cliTam = sizeof (cliAddr); //recebe mensagem
                status = recvfrom(sock, &pct, sizeof (pacote), 0, (struct sockaddr *) &cliAddr, &cliTam); //recebe pacote e armazena valor do status,positivo = sucesso 
                if (!(strcmp(pct.dados, "FIM"))) //finaliza execução ao receber FIM nos dados do pacote 

                    break;
                if (status < 0) //se pacote foi recebido com erro
                {
                    printf("Pacote recebido com erro solicitando reenvio\n");
                    resp.numSeq = pct.numSeq; //popula os campos da strutura ck para enviar o NACK ao cliente  
                    resp.status = 0;
                    rc = -1; //ENVIAR RESPOSTA AO SERVIDOR
                    while (rc < 0) {
                        rc = sendto(sock, &resp, sizeof (ck), 0, (struct sockaddr *) &cliAddr, sizeof (cliAddr));
                        if (rc < 0) //VERIFICAR SE ENVIO DA RESPOSTA FOI REALIZADO COM SUCESSO.
                        {
                            printf("Falha ao enviar NACK ao cliente, Reenviando..\n");
                        }
                    }
                } else {
                    printf("Pacote %d Recebido\n", pct.numSeq);
                    check = checksum(pct.dados, pct.tamDados); //Calcula Checksum do pacote recebido
                    if (check == pct.checksum) //verifica se o checksum está correto
                    {
                        printf("Pacote %d integro, adicionando dados ao arquivo\n", pct.numSeq); //informa ao usuário que o pacote foi recebido integro
                        fwrite(pct.dados, sizeof (char), pct.tamDados, arquivo); //escreve dado recebidos no arquivo
                        resp.numSeq = pct.numSeq; //Começa a popular dados da resposta
                        resp.status = 1;
                        rc = -1; //Prepara variável para loop de envio de resposta ao cliente
                        while (rc < 0) { //loop para envio de resposta ao cliente
                            rc = sendto(sock, &resp, sizeof (ck), 0, (struct sockaddr *) &cliAddr, sizeof (cliAddr)); //realiza o envio de resposta ao cliente
                            if (rc < 0) //Verifica se resposta foi recebida com sucesso
                            {
                                printf("Falha ao entregar ACK do pacote, Reenciando..\n");
                            }
                        }
                    } else {
                        printf("Checksum não confere, solicitando Reenvio do Pacote..\n"); //Informa ao usuário que o pacote não chegou integro
                        resp.numSeq = pct.numSeq; //Começa a popular dados da resposta
                        resp.status = 0;
                        rc = -1; //Prepara variável para loop de envio de resposta ao cliente
                        while (rc < 0) {//loop para envio de resposta ao cliente
                            rc = sendto(sock, &resp, sizeof (ck), 0, (struct sockaddr *) &cliAddr, sizeof (cliAddr)); //realiza o envio de resposta ao cliente
                            if (rc < 0) {
                                printf("Falha ao enviar NACK ao cliente, Reenviando..\n");
                            }
                        }
                    }

                }

            }
            if (!strcmp(pct.dados, "FIM")) //Verifica se o envio de pacotes acabou
                printf("Arquivo Recebido com sucesso\n"); //se sim informa ao usuário que o arquivo foi recebido com sucesso
            fclose(arquivo); //Fecha arquivo 
            break;
        case 2:
            //CRIA SOCKET PARA RECEBER DADOS
            sock = criaSocket(&cliAddr, &tempo); //Cria Socket que  será utilizado pelo cliente para receber receber solicitações
            bindSocket(&cliAddr, sock); //Faz o bind do socket a uma porta local para recebimento de dados
            memset(&pct, 0x0, buffer); //inicia o Buffer
            cliTam = sizeof (cliAddr);
            status = recvfrom(sock, &pct, sizeof (pacote), 0, (struct sockaddr *) &cliAddr, &cliTam);
            strcpy(pesquisaArquivo, pct.dados);
            arquivo = fopen(pesquisaArquivo, "rb");
            memset(&pct, 0x0, buffer); //inicia o Buffer
            status = recvfrom(sock, &pct, sizeof (pacote), 0, (struct sockaddr *) &cliAddr, &cliTam);
            strcpy(ipArq, pct.dados);

            //CONECTA AO CLIENTE QUE IRA RECEBER O ARQUIVO
            host = gethostbyname(ipArq);
            if (host == NULL) {
                printf("impossivel conectar ao host '%s', Finalizando Programa... \n", ipArq);
                exit(1);
            }
            servAddr.sin_family = host.h_addrtype;
            memcpy((char *) &servAddr.sin_addr.s_addr, host.h_addr_list[0], host.h_length); //COPIANDO DADOS DO HOST PARA O END. SERVIDOR
            servAddr.sin_port = htons(porta);

            //INICIA O ENVIO DE DADOS
            tamanho = fread(&vet, sizeof (char), 90, arquivo);
            if (tamanho == 0) {
                strcpy(vet, "FIM");
            } else {
                dados = (char*) calloc(tamanho, sizeof (char));
            }
            for (i = 0; i < tamanho; i++) {
                dados[i] = vet[i];
            }

            while (strcmp(dados, "FIM")) {
                pct2 = geraPacote(dados, numSeq, &tamanho);
                rc = -1;
                while (rc < 0) {
                    rc = sendto(sock, &pct2, sizeof (pacote), 0, (struct sockaddr*) &servAddr, sizeof (servAddr));
                    if (rc < 0)
                        printf("Falha no envio do pacote %d.. \n", pct2.numSeq);
                }
            }

            break;

    }
    return 0; //Finaliza o program

}