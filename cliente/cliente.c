#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include <errno.h>
#include<arpa/inet.h>

//constantes
#define SERVER "127.0.0.1"
#define TAMBUFFER 1024  //Tamanho maximo do buffer
#define PORTA 5000   //A porta -na qual sera enviado os dados
#define REMOTE_SERVER_PORT 5000

typedef struct pacote {
    int numsequencial; //Número de sequência do pacote
    long int check_sum; //Soma de Verificação do Pacote
    int dimensao; //DIMENSAO QUE CONTEM A VARIÁVEL DE DADOS
    char palavra[90]; //vetor de dados a serem enviados
} pacote;


typedef struct resposta {
    int status; //SUCESSO=1, FALHA=0
    int num_sequencial; //Número de Sequência do Pacote
} resposta;


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

struct hostent * conectarHost(char *dadoshost) {
    struct hostent * host;
    host = gethostbyname(dadoshost);
    if (host == NULL) {
        printf("\n<<ERRO>> Host Invalido '%s' \n", dadoshost);
        exit(1);
    }
    printf(" Preparando Trasferencia dos Dados\n"); //Mensagem de Inicio ao usuário
    return host;
}






//FUNÇÃO CRIAR SOCKET

int criaSocket(struct sockaddr_in *cliAddr, struct timeval *tempo) {
    int sock, opt;

    /*
     cria um ponto de comunicação
    socket(int domain, int type, int protocol)
     int domain -> dominino da comunicação (tipo da comunicação)
     type -> tipo de socket
     protocol -> procolo
     */

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock <= 0) {
        printf("Erro na abertura do socket: %s\n", strerror(errno)); //mensagem de erro ao abrir socket
        exit(1); //sai do programa
    }
    //Temporizador
    tempo->tv_sec = 0;
    tempo->tv_usec = 400000;
    //Definir o temporizador de recepção
    opt = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void *) tempo, sizeof (*tempo));
    //verificar a definição do temporizador 0 = Ok, -1 = Falha
    if (opt < 0)
        printf("Definir temporizador de recepção falhou\n");
    vincularPorta(cliAddr); //Vincula Porta
    return sock;
}

void vincularPorta(struct sockaddr_in serv_addr) {
    memset((char *) &serv_addr, 0, sizeof (serv_addr)); //define um buffer (Destino, caracter, tamanho)
    serv_addr.sin_family = AF_INET; //familia de endereços
    serv_addr.sin_port = htons(PORTA); //define a porta

}




//Gera Pacote para enviar

pacote geraPacote(char *mensagem, int numerosequencia, int *indice) {
    pacote pac;
    int i = 0;
    for (i = 0; i < (*indice); i++) //ADICIONA OS DADOS AO PACOTE
        pac.palavra[i] = mensagem[i];
    pac.check_sum = checksum(pac.palavra, *indice); //Chama checksum
    pac.dimensao = *indice; //Define Dimensão (Número de Bytes)
    pac.numsequencial = numerosequencia; //Define Número de Sequência do Pacote
    return pac;
}



//Envia Pacote ao Servidor

int enviar_pacote(int sock, pacote *pac, struct sockaddr_in remoteServAddr) {
    int resultado;
    //resultado = sendto(sock, pac, sizeof (pacote) + 1, 0, (struct sockaddr *) &remoteServAddr, sizeof (remoteServAddr)); //ENVIA PACOTE
    resultado = sendto(sock, pac, sizeof (pacote), 0, (struct sockaddr *) &remoteServAddr, sizeof (remoteServAddr));
    if (resultado < 0) //VERIFICA SE O ENVIO FOI REALIZADO
        printf("\n: Falha ao Enviar Pacote %d\n", pac->numsequencial);
    return resultado;
}

int resposta_servidor(FILE * arquivo, char *mensagem, struct sockaddr_in *remoteServAddr, int sock, int numerosequencia, int *indice) {
    resposta rsp;
    int aux, n, i = 0;
    char vetor[90];
    memset(&rsp, 0x0, sizeof (rsp)); //Inicia Buffer
    aux = sizeof (*remoteServAddr); //Recebe resposta
    n = recvfrom(sock, &rsp, sizeof (resposta), 0, (struct sockaddr *) remoteServAddr, &aux);
    if (n < 0) //Verifica Resposta
        printf("Resposta do Servidor nao Recebida!\n");
    else {
        if (rsp.status) //Verifica status da Resposta
        {
            printf("Pacote %d Entregue ao Servidor..\n", numerosequencia);
            *indice = fread(&vetor, sizeof (char), 90, arquivo); //Lê Proxima sequencia de Dados a serem enviadas e adiciona ao 'INDICE' a quantidade de bytes carregados
            if (*indice == 0) { //Se 'INDICE'=0 ADICIONA A FLAG 'EXIT' a Variavel de dados do Pacote para o Servidor encerrar conexão
                free(mensagem);
                mensagem = (char*) calloc(4, sizeof (char));
                mensagem[0] = 'E';
                mensagem[1] = 'X';
                mensagem[2] = 'I';
                mensagem[3] = 'T';
            } else {
                free(mensagem);
                mensagem = (char*) calloc(*indice, sizeof (char)); //SE 'INDICE'>0 cria vetor com tamanho adequado de bytes
                for (i = 0; i < (*indice); i++) {
                    mensagem[i] = vetor[i];
                }
            }
            return numerosequencia + 1;
        } else {
            printf("Pacote %d com erro detectado pelo servidor. Reenvio em andamento\n", numerosequencia);

            return numerosequencia;
        }
    }
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

    /*         * função para enviar mensagem para outro socket
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

FILE * abrir_arquivo() {
    FILE *arquivo;
    char *caminho;
    caminho = calloc(40, sizeof (char));
    printf("DIGITE O NOME DO ARQUIVO: "); //SOLICITA NOME DO ARQUIVO QUE SERA RECEBIDO
    scanf("%[^\n]", caminho);
    arquivo = fopen(caminho, "rb");
    if (!arquivo) //VERIFICA SE O ARQUIVO FOI ABERTO
    {

        printf("\nNAO FOI POSSIVEL ABRIR O ARQUIVO! EXECUCAO FINALIZADA!\n\n");
        exit(1);
    }
    return arquivo;
}

void disponibilidade_da_porta(int sock, struct sockaddr_in cliAddr) {
    int rc;
    rc = bind(sock, (struct sockaddr *) &cliAddr, sizeof (cliAddr)); //TENTA CONECTAR NA PORTA
    if (rc < 0) { //VERIFICA SE A CONEXAO OCORREU COM SUCESSO

        printf("<<ERRO>> NAO FOI POSSIVEL VINCULAR PORTA.\n");
        exit(1);
    }
}





//FUNCAO PRINCIPAL

int main(int numparametros, char *listaparametros[]) {
    int sock, rc, numerosequencia;
    struct sockaddr_in cliAddr, remoteServAddr;
    struct hostent *host;
    struct timeval tempo;
    char vetor[90];
    char *mensagem;
    int indice, i = 0;
    FILE *arquivo;
    pacote envio;
    
    // WSADATA wsaData;
    //WSAStartup(MAKEWORD(2, 1), &wsaData); // INICIALIZA A DLL DE SOCKETS PARA O WINDOWS
    if (numparametros < 2) { //VERIFICA NUMERO DE PARAMETROS PASSADOS (NOME DA FUNCAO E IP)
        printf("\n<<INFO>>INFOME O SERVIDOR! EX: 'cliente_arquivo.exe 127.0.0.1'.\n");
        exit(1);
    }
    arquivo = abrir_arquivo(); //ABRIR ARQUIVO DE TEXTO
    host = conectarHost(listaparametros[1]); //OBTER ENDEREÇO IP DO SERVIDOR, SEM VERIFICAR SE A ENTRADA É O ENDEREÇO IP OU O NOME DNS
    //preparaConexao(host, &remoteServAddr); //PREPARAR A CONEXAO COM O SERVIDOR
    remoteServAddr.sin_family = AF_INET;
    remoteServAddr.sin_port = htons(REMOTE_SERVER_PORT); //define a porta
    sock = criaSocket(&cliAddr, &tempo); //CRIAR SOCKET E VINCULAR PORTA DO CLIENTE
    //disponibilidade_da_porta(sock, cliAddr); //CRIAR DISPONIBILIDADE DA PORTA
    /* INICIO DO ENVIO DE DADOS AO SERVIDOR */
    numerosequencia = 1; //INICIA VARIAVEL DE CONTROLE SEQUENCIAL DOS PACOTES
    indice = fread(&vetor, sizeof (char), 90, arquivo); //LÊ A PRIMEIRA SEQUENCIA DE DADOS DO ARQUIVO E RETORNA A QUANTIDADE DE BYTES CARREGADOS PARA A VARIÁVEL INDICE
    if (indice == 0) //VERIFICA SE O ARQUIVO NÃO ESTAVA VAZIO
        strcpy(vetor, "EXIT"); //CASO O ARQUIVO ESTEJA VAZIO ADICIONA O FLAG 'EXIT' A VARIAVEL DE DADOS PARA QUE O SERVIDOR ENCERRE A TRANSFERÊNCIA
    else {
        mensagem = (char*) calloc(indice, sizeof (char)); //CASO HAJA DADOS NO ARQUIVO GERA UM VETOR COM O TAMANHO EXATO DA QUANTIDADE DE BYTES
    }
    for (i = 0; i < indice; i++) { //COPIA OS DADOS PARA O VETOR DE TAMANHO ADEQUADO.
        mensagem[i] = vetor[i];
    }
    while (strcmp(mensagem, "EXIT")) { //LOOP PARA ENVIO DOS PACOTES DE DADOS
        envio = geraPacote(mensagem, numerosequencia, &indice); //GERA O PRIMEIRO PACOTE A SER ENVIADO
        rc = -1; //VARIAVÉL DE CONTROLE PARA REENVIO DE PACOTES COM ERRO DETECTADO PELO SERVIDOR
        while (rc < 0)
            rc = enviar_pacote(sock, &envio, remoteServAddr);
        numerosequencia = resposta_servidor(arquivo, mensagem, &remoteServAddr, sock, numerosequencia, &indice);
    }
    strcpy(envio.palavra, "EXIT"); //ENVIO DE MENSAGEM DE TERMINO AO SERVIDOR
    rc = sendto(sock, &envio, sizeof (pacote) + 1, 0, (struct sockaddr *) &remoteServAddr, sizeof (remoteServAddr));
    printf("\n<<INFO>>TRANSFERENCIA CONCLUIDA COM SUCESSO!\n\n"); //MENSAGEM AO USUÁRIO DE TERMINO DA TRANFERÊNCIA
    fclose(arquivo);
    exit(0);
}



/*int main() {
    funcInicio();

    return 0;
}*/

/*
 Referencias

 * https://docs.microsoft.com/en-us/windows-hardware/drivers/network/af-inet
 * https://www.vivaolinux.com.br/artigo/Datagramas
 * https://gist.github.com/jonhoo/7780260
 * https://www.binarytides.com/raw-udp-sockets-c-linux/
 * http://penta2.ufrgs.br/Esmilda/fmtoudp.html
 */
