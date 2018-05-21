#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include <errno.h>

//constantes
#define SERVER "127.0.0.1"
#define TAMBUFFER 1024  //Tamanho maximo do buffer
#define PORTA 5000   //A porta na qual sera enviado os dados

/////////////////////Funções////////////////////////

long int checksum(char palavra[], int dimensao) {
    long int soma;
    int asc, i;
    soma = 0; //Variável que irá armazenar o cálculo do checksum
    for (i = 0; i < dimensao; i++) { //percorre vetor de dados para cálcular o checksum
        asc = (int) palavra[i]; //transforma palavra em valor inteiro seguindo a tabela ascii
        soma += asc; //adiciona a soma
    }
    return soma; //retorna a soma (checksum)
}


//função para inicializar conexão

void funcInicio() {
    struct sockaddr_in serv_addr; //estrututra para programar socket
    int sock; //numero do socket
    int slen = sizeof (serv_addr); //pega o tamanho da struct
    char buffer[TAMBUFFER]; //buffer
    char pesquisaArquivo[51], pesquisaIp[31];

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
        printf("Erro na abertura do socket: %s\n", strerror(errno)); //mensagem de erro ao abrir socket
        exit(1); //sai do programa
    }

    memset((char *) &serv_addr, 0, sizeof (serv_addr)); //define um buffer (Destino, caracter, tamanho)
    serv_addr.sin_family = AF_INET; //familia de endereços
    serv_addr.sin_port = htons(PORTA); //define a porta
    /*
    converte e retorna o endereço passado como parâmetro para um ordenamento de byte significativo. Retorna
     * o na ordem de bytes da rede.
     */

    /*
    Converte o endereço passado (inclusive com pontos) para uma estrutura de endereços (binário)
    válido. Retorna um valor maior que zero se a conversão ocorreu ou zero se houve algum erro.
     */
    if (inet_aton(SERVER, &serv_addr.sin_addr) == 0)//o endereço passado foi o do servidor
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }


    while (strcmp(pesquisaArquivo, "fim") != 0) {

        //usuário digita o nome do arquivo que será pesquisado no servidor
        printf("Digite o nome do arquivo que deseja baixar\n");
        gets(pesquisaArquivo);

        /*
         * função para enviar mensagem para outro socket
         * sendto é usado em UDP
         * sendto(socket, mensagem_a_enviar, tamanho da mensagem, flags, endereço pra quem vai enviar,
         tamanho socket
         */
        if (sendto(sock, pesquisaArquivo, strlen(pesquisaArquivo), 0,
                (struct sockaddr *) &serv_addr, slen) == -1) {
            //terminar("sendto()");
        }

        //Recebe dados do servidor
        if (recvfrom(sock, buffer, TAMBUFFER, 0, (struct sockaddr *) &serv_addr,
                (socklen_t *) & slen) == -1) {
            printf("Erro ao enviar pacote");
        }

        //printa os dados recebidos
        printf("%s\n", buffer);
    }

}

int main() {

    funcInicio();
    return 0;
}

/*
 Referencias
 
 * https://docs.microsoft.com/en-us/windows-hardware/drivers/network/af-inet
 * https://www.vivaolinux.com.br/artigo/Datagramas
 * https://gist.github.com/jonhoo/7780260
 * https://www.binarytides.com/raw-udp-sockets-c-linux/
 * http://penta2.ufrgs.br/Esmilda/fmtoudp.html
 */