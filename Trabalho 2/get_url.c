#include "header.h"
//Este eh um parser que recebe um 'HTTP request' em texto e retorna uma URL referente ao host que esta
//contido como uma das informações deste request
char * get_url(char * http_text){

	char * get = NULL;
	char * line = NULL;
	char host[6];
	
	//usei malloc pois não estava conseguindo fazer 'return url' de outra maneira
	char *url = (char *) malloc(sizeof(char) * 1025);

	get = strtok(http_text, " ");

	//so busca 'Host:' se for HTTP 'GET'
	if(strcmp(get, "GET") == 0){

		line = strtok(NULL, "\n");
		//a primeira chamada à 'strtok' eh para desperdicar o resto da primeira linha, não utilizada na chamada inicial
		line = strtok(NULL, "\n");

		//pesquisa no texto, linha a linha, procurando por 'Host:'
		while(line != NULL){	
			/*'sscanf()' formata o conteudo contido em 'line' separando suas
				substrings por espaço e colocando-as respectivamente nas strings dadas como parametros*/

			sscanf(line, "%s %s", host, url);
			//printf("host: %s, url: %s\n", host, url);

			if(strcmp(host, "Host:") == 0){
				return url;
			}
			
			//proxima linha...
			line = strtok(NULL, "\n");
		}
	}
	else{
		printf("This is not a 'GET' request! Not going to be treated. Exiting...\n");
		//exit(1);
	}

	return NULL;
}
