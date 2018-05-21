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
#define PORTA 5000


//Função para pesquisar IP que tem o arquivo a ser pesquisado
char* funcPesquisaArquivo(char* pesquisaArquivo){	

	char  resultadoArquivo[51];//resultado do arquivo pesquisado no servidor
        char* resultadoIp = (char*) calloc (16, sizeof(char));//armazena o resultado do ip achado no arquivo de pesquisa
        
	FILE *ptarq;//ponteiro para manipular arquivo

	//abre arquivo em modo de leitura
	ptarq = fopen("arquivos.txt", "r");
	
        //verifica se deu erro ao abrir o arquivo
	if(ptarq == NULL)
	{
		printf("\n\nErro ao abrir arquivo\n\n");
		exit(0);
	}
	
        //pesquisa todo o arquivo de dados do servidor
	while((fscanf(ptarq,"%s %s\n", resultadoArquivo, resultadoIp))!=EOF )
	 //!feof(ptarq))//Enquanto não chegar no final do arquivo
	{
		
		//se encontrou o arquivo pesquisado
		if(strcmp(resultadoArquivo, pesquisaArquivo) == 0){
			printf("O arquivo %s esta localizado no IP %s\n", resultadoArquivo, resultadoIp);
		}
		else{//senão encontrou
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
void funcInicio(){
    struct sockaddr_in server_sock, client_sock;//estrututra para programar socket
   
    int sock;//número do socket
    int slen = sizeof(client_sock);//tamanho da estrutura de socket
    int binder;//define o bind
    
    char buffer[TAMBUFFER];//mensagem 
 
    /*
     cria um ponto de comunicação
    socket(int domain, int type, int protocol)
     int domain -> dominino da comunicação (tipo da comunicação)
     type -> tipo de socket
     protocol -> procolo
    */    
    sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    //verifica se o socket foi aberto corretamente
    if(sock <= 0){
	printf("Erro na abertura do socket: %s\n", strerror(errno));
	exit(1);
    }            

    memset((char *) &server_sock, 0, sizeof(server_sock));//define um buffer (Destino, caracter, tamanho)
    server_sock.sin_family = AF_INET;//familia de endereços
    server_sock.sin_port = htons(PORTA);//define a porta
    
    
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
    binder = bind(sock , (struct sockaddr*)&server_sock, sizeof(server_sock));
    //verifica se deu erro no bind 
    if(binder < 0){
	printf("Erro no Bind: %s\n", strerror(errno));
	exit(1);
    }
    
    //espera por dados
    while(strcmp(buffer,"fim")!=0){
        printf("Esperando por dados\n");
        fflush(stdout);
        memset(buffer,'\0',TAMBUFFER);
        
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
                (socklen_t*)&slen) == -1){
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
        printf("mensagem: %s\n" , buffer);//imprime a mensagem
        
        //chama função para pesquisar o nome do arquivo. Será retornado o IP do cliente que possui arquivo
        funcPesquisaArquivo(buffer);
        

        //retorna o cliente com a mensagem
        if (sendto(sock, buffer, strlen(buffer), 0, 
                (struct sockaddr*) &client_sock, slen) == -1){
            //terminar("sendto()");
        }
 
 
    }
    
    
}


int main(){

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
