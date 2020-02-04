#include "header.h"

//---Realiza o 'hard work' do proxy de dialogar com o servidor remoto e com o cliente
void *dialogue(void *sock){

	char text[32765];	
	char *url = NULL;	
	int sockRemoteServer;
	int closing;
	struct sockaddr_in proxy_addr;
	struct hostent *hostname;

	//------------------------------//
	char textServer[3276500];


	//---recupera o socket previamente inicializado---//
	//int newsockfd = (int) sock; em C

	//int newsockfd = static_cast<int>(reinterpret_cast<std::uintptr_t>(sock));
	int newsockfd = *((int*)sock);
	memset(text, 0, sizeof(text));

	//man recv
	if(recv(newsockfd, text, sizeof(text), 0) < 0) {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(1);
	}
	
	printf("Mensagem Navegador:\n%s\n", text);

	/*'get_url()' retorna uma string referente a URL caso seja uma requisição GET e NULL caso contrário*/
	url = get_url(text);
	//printf("URL: %s\n",url);
	FILE *arq;
	arq = fopen(url, "wr");  // Cria um arquivo texto para gravação
	if (arq == NULL){ // Se não conseguiu criar	
		printf("Problemas na CRIACAO do arquivo\n");
		exit(1);
	}

	//TODO: send request to remote server --- In Progress ---

	if ((sockRemoteServer = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
   		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(1);
	}
	
	//hostname apropriado (caso a url retornada seja a correta)
	hostname = gethostbyname(url);	

	memset(&proxy_addr, 0, sizeof(proxy_addr));
	proxy_addr.sin_family = AF_INET;
	memcpy(&proxy_addr.sin_addr.s_addr, hostname->h_addr, hostname->h_length);
	proxy_addr.sin_port = htons(80);
	
	if(connect(sockRemoteServer, (struct sockaddr*) &proxy_addr, sizeof(proxy_addr)) < 0) {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		exit(1);
	}

	char message[32765];
	//message = "GET / HTTP/1.1\r\nHost: "+url"\r\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:57.0) Gecko/20100101 Firefox/57.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: pt-BR,pt;q=0.8,en-US;q=0.5,en;q=0.3\r\n\r\n";

	strcpy (message,"GET / HTTP/1.1\r\nHost: ");
    strcat (message,url);
    strcat (message,"\r\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:57.0) Gecko/20100101 Firefox/57.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: pt-BR,pt;q=0.8,en-US;q=0.5,en;q=0.3\r\n\r\n");

	puts(message);
	//while(1){
		//Send-Recv to Remote Server
		if(send(sockRemoteServer, message, strlen(message), 0) < 0) {
			fprintf(stderr, "ERROR1: %s\n", strerror(errno));
			exit(1);
		}

		memset(textServer, 0, sizeof(textServer));



		//Recebendo e montando em um arquivo------------------------
		size_t datasize=1, totalDataSize=0;
		char text3[1024];
		while(datasize > 0){
			datasize = recv(sockRemoteServer, text3, sizeof(text), 0);
			totalDataSize += datasize;
			fwrite(&text3, sizeof(char), datasize, arq);
			memset(text3, 0, 1024);
		}
		fclose(arq);
		//----------------------------------------------------------
		closing = close(sockRemoteServer);

		//printf("Mensagem servidor: %s\n",textServer);

		//Sending back to navigator
		arq = fopen(url, "rb");
		char text2[1024];

		int bytes_read;
		while(!feof(arq)){
			if ((bytes_read = fread(&text2, 1, totalDataSize, arq)) > 0)
       			send(newsockfd, text2, bytes_read, 0);

		}
		fclose(arq);

		
	//}
	
	close(newsockfd);
	//----------------------------------FIM TODO--------------------------------------//
}
