#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//constantes
#define TAMBUFFER 1024
#define PORTA 1028
#define LOCAL_SERVER_PORT 5000
#define MAX_MSG 100

typedef struct pacote {
    int numsequencial; //NUMERO SEQUENCIAL DO PACOTE
    long int check_sum; //CHEKSUN DO PACOTE
    int dimensao; //DIMENSAO QUE CONTEM A VARIÁVEL DE DADOS
    char palavra[90]; //VETOR DE DADOS
} pacote;

typedef struct resposta {
    int status; //SUCESSO=1, FALHA=0
    int num_sequencial; //NUMERO SEQUENCIAL DO PACOTE
} resposta;

//FUNÇÃO QUE CALCULA O CHEKSUN DO PACOTE DE DADOS

long int checksum(char palavra[], int dimensao) {
    long int soma;
    int asc, i;
    soma = 0; //INICIALIZA VARIAVEL QUE ARMAZENA O CALCULO DO CHEKSUN
    for (i = 0; i < dimensao; i++) { //PERCORRE O VETOR DE DADOS PARA CALCULAR O CHEKSUN
        asc = (int) palavra[i]; //OBTEM O VALOR INTEITO CORRESPONDENTE AO CARACTER NA TABELA ASCII
        soma += asc; //ADICIONA O VALOR INTEIRO NA VARIAVEL "SOMA"
    }
    return soma; //RETORNA O VALOR DA VARIAVEL "SOMA" QUE SERÁ O CHEKSUN DO PACOTE
}


//Função para pesquisar IP que tem o arquivo a ser pesquisado

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


/////////////////////Funções/////////////////////////

//função para inicializar conexão

void funcInicio() {
    struct sockaddr_in server_sock, client_sock; //estrututra para programar socket

    int sock; //número do socket
    int slen = sizeof (client_sock); //tamanho da estrutura de socket
    int binder; //define o bind

    char buffer[TAMBUFFER]; //mensagem

    /*
     cria um ponto de comunicação
    socket(int domain, int type, int protocol)
     int domain -> dominino da comunicação (tipo da comunicação)
     type -> tipo de socket
     protocol -> procolo
     */
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    //verifica se o socket foi aberto corretamente
    if (sock <= 0) {
        printf("Erro na abertura do socket: %s\n", strerror(errno));
        exit(1);
    }

    memset((char *) &server_sock, 0, sizeof (server_sock)); //define um buffer (Destino, caracter, tamanho)
    server_sock.sin_family = AF_INET; //familia de endereços
    server_sock.sin_port = htons(PORTA); //define a porta


    /*
     INADDR_ANY ->no serivor é argumento para o bind que fala o socket para ouvir todas as interfaces disponíveis
        server_sock.sin_addr.s_addr -> endereço IP
     */
    server_sock.sin_addr.s_addr = htonl(INADDR_ANY);


    /*
    Faz o bind do socket com a porta.
    bind -> associa o socket criado a porta local do sistema operacional. Será desta associação
    (porta) que o programa receberá dados (bytes) de outros programas
    bind(int sockfd, const struct sockaddr, *my_addr, socklen_t addrlen)
     */
    binder = bind(sock, (struct sockaddr*) &server_sock, sizeof (server_sock));
    //verifica se deu erro no bind
    if (binder < 0) {
        printf("Erro no Bind: %s\n", strerror(errno));
        exit(1);
    }

    //espera por dados
    while (strcmp(buffer, "fim") != 0) {
        printf("Esperando por dados\n");
        fflush(stdout);
        memset(buffer, '\0', TAMBUFFER);

        /*
        recvfrom -> função recvfrom é utilizada para receber (ler) uma mensagem de um socket.
         * deve ser usado com a função sento (UDP)

        recvfrom(int s, void * buffer, size_t len, int flags, struct sockaddr * from, socklem_t * fromlen)
        s -> socket
        buffer -> mensagem lida
        len -> tamanho da mensagem lida (Buffer)
        flags
        from -> endereço do cliente
        socklem_t -> tamanho do socket cliente

        Tenta receber algum dado do cliente
         */
        if (recvfrom(sock, buffer, TAMBUFFER, 0, (struct sockaddr *) &client_sock,
                (socklen_t*) & slen) == -1) {
            //terminar("recvfrom()");
        }

        //imprime os dados do cliente
        /*
        inet_ntoa -> a partir de um valor binário (estrutura) ela retorna o endereço em formato string
        (inclusive com pontos).
        inet_ntoa(client_sock.sin_addr) -> pega o ip do cliente e converte em formato string

        ntohs(client_sock.sin_port)) -> pega a porta do cliente e converte em formato string
         */

        printf("Origem do cliente %s:%d\n", inet_ntoa(client_sock.sin_addr),
                ntohs(client_sock.sin_port));
        printf("mensagem: %s\n", buffer); //imprime a mensagem

         //chama função para pesquisar o nome do arquivo. Será retornado o IP do cliente que possui arquivo
        // funcPesquisaArquivo(buffer);


        //retorna o cliente com a mensagem
        if (sendto(sock, buffer, strlen(buffer), 0,
                (struct sockaddr*) &client_sock, slen) == -1) {
            //terminar("sendto()");
        }


    }


}

//FUNCAO PRINCIPAL

int main(int argc, char *argv[]) {

    int sock, rc, n, cliLen;
    long int checar;
    struct sockaddr_in cliAddr, servAddr;
    char msg[MAX_MSG];
    FILE *arquivo;
    pacote pct;
    resposta rsp;
    //WSADATA wsaData;
    //WSAStartup(MAKEWORD(2, 1), &wsaData); // INICIALIZA A DLL DE SOCKETS PARA O WINDOWS
    printf("DIGITE O NOME DO ARQUIVO: "); //SOLICITA NOME DO ARQUIVO QUE SERÁ RECEBIDO
    scanf("%s", msg);
    arquivo = fopen(msg, "wb"); //ABRE O ARQUIVO EM FORMATO BINARIO
    if (!arquivo) //VERIFICA SE O ARQUIVO FOI ABERTO
    {
        printf("\n<<ERRO>> NAO FOI POSSIVEL ABRIR O ARQUIVO! EXECUCAO FINALIZADA!\n\n");
        return 1;
    }
    sock = socket(AF_INET, SOCK_DGRAM, 0); //CRIAÇÃO DE SOCKET
    if (sock < 0) {
        printf("\n<<ERRO>> NAO FOI POSSIVEL ABRIR O SOCKET. \n");
        exit(1);
    }
    servAddr.sin_family = AF_INET; //LIGANDO PORTA LOCAL DO SERVIDOR
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(LOCAL_SERVER_PORT);
    rc = bind(sock, (struct sockaddr *) &servAddr, sizeof (servAddr));
    if (rc < 0) {
        printf("<<ERRO>> NAO FOI POSSIVEL UTILIZAR A PORTA DE NUMERO %d!\n", LOCAL_SERVER_PORT);
        exit(1);
    }
    printf("\n<<INFO>>A ESPERA DE DADOS NA PORTA UDP %u...\n", LOCAL_SERVER_PORT);
    while (strcmp(pct.palavra, "EXIT")) //RECEBIMENTO DO ARQUIVO, FINALIZA QUANDO RECEBE UM PACOTE COM O DADO "EXIT"
    {
        memset(&pct, 0x0, MAX_MSG); //INICIA O BUFFER
        cliLen = sizeof (cliAddr); //RECEBE MENSAGEM
        n = recvfrom(sock, &pct, sizeof (pacote) + 1, 0, (struct sockaddr *) &cliAddr, &cliLen); //RECEBE O PACOTE E ARMAZENA UM VALOR POSITIVO EM N CASO HAJA SUCESSO
        if (!(strcmp(pct.palavra, "EXIT"))) //FINALIZA A EXECUÇÃO SE OS DADOS DO PACOTE CONTER "EXIT"
            break;
        if (n < 0) //VERIFICA SE "N" ESTÁ NEGATIVO PARA SOLICITAR REENVIO DO PACOTE
        {
            printf("<<ERRO>> PACOTE RECEBIDO COM ERRO, SOLICITANDO REENVIO...\n");
            rsp.num_sequencial = pct.numsequencial; // GERAR RESPOSTA DO SERVIDOR
            rsp.status = 0;
            rc = -1; //ENVIAR RESPOSTA AO SERVIDOR
            while (rc < 0) {
                rc = sendto(sock, &rsp, sizeof (resposta), 0, (struct sockaddr *) &cliAddr, sizeof (cliAddr));
                if (rc < 0) //VERIFICAR SE ENVIO DA RESPOSTA FOI REALIZADO COM SUCESSO.
                {
                    printf("<<ERRO>> FALHA AO ENTREGAR NEGATIVA DE RECEBIMENTO DO PACOTE. REENVIANDO..\n");
                }
            }
        } else {
            printf("<<INFO>> PACOTE %d RECEBIDO DO CLIENTE..\n", pct.numsequencial);
            checar = checksum(pct.palavra, pct.dimensao); //CALCULAR CHEKSUN
            if (checar == pct.check_sum) //COM O CHEKSUN VERIFICA SE O PACOTE CHEGOU CORRETAMENTE
            {
                printf("<<INFO>> CHEKSUN OK! ADICIONANDO DADOS AO ARQUIVO..\n"); //INFORMA AO USUÁRIO SE O PACOTE FOI RECEBIDO COM SUCESSO.
                fwrite(pct.palavra, sizeof (char), pct.dimensao, arquivo); //ADICIONA DADOS DO PACOTE AO ARQUIVO
                rsp.num_sequencial = pct.numsequencial;
                rsp.status = 1; // GERAR RESPOSTA DO SERVIDOR
                rc = -1; //ENVIAR RESPOSTA AO SERVIDOR
                while (rc < 0) {
                    rc = sendto(sock, &rsp, sizeof (resposta), 0, (struct sockaddr *) &cliAddr, sizeof (cliAddr));
                    if (rc < 0) //VERIFICAR SE ENVIO DA RESPOSTA FOI REALIZADO COM SUCESSO.
                    {
                        printf("<<ERRO>> FALHA AO ENTREGAR POSITIVA DE RECEBIMENTO DO PACOTE. REENVIANDO..\n");
                    }
                }
            } else {
                printf("<<ERRO>> CHECKSUN NAO CONFERE. SOLICITANDO REENVIO DO PACOTE..\n"); //INFORMA AO USUÁRIO QUE CHEKSUN NÃO CONFERE
                rsp.num_sequencial = pct.numsequencial; // GERAR RESPOSTA DO SERVIDOR
                rsp.status = 0;
                rc = -1; //ENVIAR RESPOSTA AO SERVIDOR
                while (rc < 0) {
                    rc = sendto(sock, &rsp, sizeof (resposta), 0, (struct sockaddr *) &cliAddr, sizeof (cliAddr));
                    if (rc < 0) {
                        printf("<<ERRO>> FALHA AO ENTREGAR NEGATIVA DE RECEBIMENTO DO PACOTE. REENVIANDO..\n");
                    }
                }
            }

        }

    }
    if (!strcmp(pct.palavra, "EXIT"))
        printf("\n<<INFO>> ARQUIVO RECEBIDO COM SUCESSO!\n");
    fclose(arquivo);
    return 0;
}








/*
int main() {

    funcInicio();

    return 0;

}
 */


/*
 Referencias

 * https://docs.microsoft.com/en-us/windows-hardware/drivers/network/af-inet
 * https://www.vivaolinux.com.br/artigo/Datagramas
 * https://gist.github.com/jonhoo/7780260
 * https://www.binarytides.com/raw-udp-sockets-c-linux/
 * http://penta2.ufrgs.br/Esmilda/fmtoudp.html
 */
