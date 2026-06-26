#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 1024

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE], recvline[MAXLINE];

    if (argc != 3) {
        printf("Użycie: %s <Adres_IP_Serwera> <Port>\n", argv[0]);
        exit(1);
    }
 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;  
    servaddr.sin_port = htons(atoi(argv[2]));  
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {  
        perror("Błąd połączenia connect");
        exit(1);
    }

    printf("Połączono z serwerem echo. Wpisz tekst (Ctrl+D aby wyjść):\n");  

    int n_bytes;
    
    // czytanie z klawiatury linia po linii
    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        write(sockfd, sendline, strlen(sendline));

        // zamiast zwykłego read, zapisujemy ile bajtów faktycznie przyszło
        if ((n_bytes = read(sockfd, recvline, MAXLINE - 1)) == 0) {
            printf("Serwer przedwcześnie zamknął połączenie.\n");
            break;
        } else if (n_bytes < 0) {
            perror("Błąd read");
            break;
        }

        // dopasowujemy koniec stringu do odebranych danych
        recvline[n_bytes] = '\0'; 
        printf("Echo: %s", recvline);
    }

    close(sockfd);  
    return 0;
}