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

// ./exec (-a adresse) (-p port)
int main(int nbargs, char *targs[]) {
 
 	char* addresse = NULL;
	int port = 0;
	
    // Argument de l'application
	int i;
    for (i = 1; i < nbargs; ++i) {
        char* s = targs[i];
        char c;
        if (s[0] == '-') 
            c = s[1];
        switch (c) {
            case 'a':
                addresse = targs[++i];
                break;
            case 'p':
                sscanf(targs[++i],"%d",&port);
                break;
        }
    }
    
    // Adresse et port par defaut
    if (addresse == NULL)
    	addresse = LOCAL_ADDR;
    if (port == 0) 
    	port = PORT;
 
    int sock;
    struct sockaddr_in sin;
    char bufferIn[TCHAINE] = "";
    char bufferOut[TCHAINE] = "";

    // Creation de la socket coté client
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
    	perror("Creation socket");
    	exit(-1);
    }
    printf("Socket %d creee\n", sock);

    // Configuration de la connexion (sockaddr_in)
    sin.sin_addr.s_addr = inet_addr(addresse);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port); // htons : Ordonnancement des bits de local a réseau

    // Connection au serveur
    if(connect(sock, (struct sockaddr*)&sin, sizeof(sin)) == -1) {
    	perror("Connection au serveur");
    	exit(-2);
    }

    printf("Connection au serveur reussie\n");
    
    // On recoit et envoie une reponse jusqu'a sortie (/exit)
    int boolean = 1;
    while (boolean) {
        // Reception
		if(recv(sock, bufferIn, TCHAINE, 0) == -1)
			perror("Reception");

        // Demande de sortie par le serveur
		if (strcmp(bufferIn,"exit0") == 0) {
			boolean = 0;
			continue;
		}
        // Affichage
		printf("%s", bufferIn);
		
        // Reponse ("%[^\n]*c" scan de plusieurs mots)
	   	scanf("%[^\n]*c",bufferOut);
	    
        // On vide le buffer
		while ((getchar()) != '\n'); 
	   	
        // Envoi reponse
		if(send(sock, bufferOut, TCHAINE, 0) == -1)
        		perror("Envoi");
    }
    
    // On ferme la socket
    close(sock);
    printf("Vous etes deconnecte du serveur\n");
 
    return EXIT_SUCCESS;
}
