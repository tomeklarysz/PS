#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <syslog.h>
#include <errno.h>

#define MAXLINE 1024

// obsługa sygnału SIGCHLD - automatyczne oczyszczanie procesów zombie
void obsluga_sigchld(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// obsługa sygnału SIGPIPE - łapiemy moment, gdy klient chamsko zerwie połączenie 
void obsluga_sigpipe(int signo) {
    syslog(LOG_LOCAL7 | LOG_WARNING, "Wykryto nagłe zerwanie potoku (SIGPIPE) przez klienta!");  
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    char buf[MAXLINE];
    int n;

    if (argc != 2) {
        printf("Użycie: %s <numer_portu>\n", argv[0]);
        exit(1);
    }

    // otwarcie połączenia z demonem syslog
    openlog("SerwerEcho", LOG_PID | LOG_CONS, LOG_LOCAL7); 

    listenfd = socket(AF_INET, SOCK_STREAM, 0); 
    
    // zabezpieczenie przed stanem TIME_WAIT dzieki SO_REUSEADDR
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); 

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(atoi(argv[1])); 

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)); 

    listen(listenfd, 5); 

    // rejestracja funkcji obsługi sygnałów systemowych 
    signal(SIGCHLD, obsluga_sigchld); 
    signal(SIGPIPE, obsluga_sigpipe);

    syslog(LOG_LOCAL7 | LOG_INFO, "Serwer Echo pomyślnie uruchomiony na porcie %s", argv[1]);

    for ( ; ; ) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen); 

        if (connfd < 0) {
            if (errno == EINTR) continue; // obsługa przerwania przez sygnał systemowy
            else { perror("Błąd funkcji accept"); exit(1); }
        }

        syslog(LOG_LOCAL7 | LOG_NOTICE, "Nowe połączenie od: %s", inet_ntoa(cliaddr.sin_addr));

        // tworzenie procesu potomnego do obsługi tego konkretnego klienta 
        if ( (childpid = fork()) == 0) { 
            close(listenfd); // proces potomny zamyka gniazdo nasłuchujące

            while ( (n = read(connfd, buf, MAXLINE)) > 0) { 
                buf[n] = '\0';
                syslog(LOG_LOCAL7 | LOG_DEBUG, "Odebrano wiadomość: %s", buf);
                write(connfd, buf, n);
            }

            syslog(LOG_LOCAL7 | LOG_NOTICE, "Klient rozłączył się prawidłowo.");
            close(connfd); 
            exit(0); // poprawne zakończenie procesu potomnego 
        }
        close(connfd); // proces macierzysty zamyka gniazdo połączone
    }
}