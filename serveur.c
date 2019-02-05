#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 5000
#define TCHAINE 512
#define LOCAL_ADDR "127.0.0.1"

// Commande recue du client de forme : "/commande"
int command(char* c) {
	if (strcmp(c,"/exit") == 0) {
		return 0;
	} if (strcmp(c,"/reverse") == 0) {
		return 1;
	} else
		return -1;
}

// Algo d'inversion de chaine de caracteres
char *strrev(char *str) {
    if (!str || ! *str)
        return str;

    int i, j;
    char ch;
    for (i=strlen(str)-1,j=0;i>j;i--,j++) {
    	ch = str[i];
        str[i] = str[j];
        str[j] = ch;
    }
    return str;
}

// Suite a /reverse, service d'inverstion de chaine
void reverseF(char* buffer, int cSock) {
	char ret[TCHAINE] = "";

	int tmp = 1;
	while (tmp) {
		tmp = 0;
		strcpy(buffer,"Entrez un chaine a inverser : ");
		if(send(cSock, buffer, TCHAINE, 0) == -1)
			perror("Envoi");
		if(recv(cSock, buffer, TCHAINE, 0) == -1)
			perror("Reception");
		strcpy(ret, buffer);
		strrev(ret);
		sprintf(buffer,"Chaine inversee ! '%s'\nVoulez-vous inverser une autre chaine ? y / n : ",ret);
		// Demande pour continuer d'inverser des chaines
		if(send(cSock, buffer, TCHAINE, 0) == -1)
			perror("Envoi");
		
		if(recv(cSock, buffer, TCHAINE, 0) == -1)
			perror("Reception");
		if (strcmp(buffer,"y") != 0)
			strcpy(buffer,"");
			break;
	}
}

// ./exec (-a adresse) (-p port)
int main(int nbargs, char *targs[]) {
	
	int port = 0;

	// Arguments de l'application
	int i;
	for (i = 1; i < nbargs; ++i) {
	   char* s = targs[i];
	   char c;
	   if (s[0] == '-') 
		  c = s[1];
	   switch (c) {
		  case 'p':
		      sscanf(targs[++i],"%d",&port);
		      break;
	   }
	}
    
    // Port par defaut
	if (port == 0) 
		port = PORT;
	
	int sock;
	struct sockaddr_in sin;
	int cSock;
	struct sockaddr_in cSin;
	char buffer[TCHAINE] = "";
	socklen_t recsize = sizeof(cSin);
	int sock_err;

	// Creation de la socket coté client
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		perror("Creation socket");
		exit(-1);
	}
	printf("Socket %d creee\n", sock);

	// Configuration de la connexion (sockaddr_in)
	sin.sin_addr.s_addr    = htonl(INADDR_ANY);
	sin.sin_family         = AF_INET;
	sin.sin_port           = htons(port);

    // Bind de la socket serveur
	if(bind(sock, (struct sockaddr*)&sin, sizeof(sin)) == -1) {
		perror("Bind de la socket serveur");
		exit(-3);
	}

	// Position de la socket serveur en ecoute
	if(listen(sock, 5) == -1) {
		perror("Listen de la socket serveur");
		exit(-4);
	}

	// Serveur en attente de conneion
	printf("Socket en ecoute passive sur le port : %d\n", port);        
	
	while (1) {
		// Connexion d'un client
		cSock = accept(sock, (struct sockaddr*)&cSin, &recsize);
		printf("Un client s est connecte\n");
		
		if (fork() == 0) {
			int boolean = 1;
			strcpy(buffer,"-- ");
			while (boolean) {
				if(send(cSock, buffer, TCHAINE, 0) == -1)
					perror("Envoi");

				if(recv(cSock, buffer, TCHAINE, 0) == -1)
					perror("Reception");

				// On regarde si le client a saisi une commande
				if (buffer[0] == '/') {
					// Traitement de la chaine par la fonction command(char*)
					switch (command(buffer)) {
					case 0 :
						boolean = 0;
						break;
					case 1 :
						reverseF(buffer, cSock);
						break;
					default :
						strcpy(buffer, "Commande inconnue");
						break;
					}
				}
				strcat(buffer,"\n-- ");
				continue;
			}
			// On dit au client de se terminer
			strcpy(buffer,"exit0");
			if(send(cSock, buffer, TCHAINE, 0) == -1)
				perror("Envoi");
			shutdown(cSock, 2);

			printf("Un client est parti\n");
			exit(0);
		}
	}

    // On ferme les sockets
    shutdown(cSock, 2);
    close(sock);
    
    printf("Fermeture du serveur terminee\n");
 
    return EXIT_SUCCESS;
}
