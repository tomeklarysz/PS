#include 	<sys/types.h>
#include 	<sys/socket.h>
#include 	<netdb.h>
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<unistd.h>
#include 	<string.h>
#include        <sys/time.h>    /* timeval{} for select() */
#include        <time.h>                /* timespec{} for pselect() */
#include        <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include        <arpa/inet.h>   /* inet(3) functions */
#include        <errno.h>
#include        <fcntl.h>               /* for nonblocking */
#include        <netdb.h>
#include        <signal.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include <sys/utsname.h>

#define MAXLINE 500

int
main(int argc, char *argv[])
{

	struct timeval delay;
	int	sfd, n, s, i;
	socklen_t			len;
	char				recvline[MAXLINE], str[INET6_ADDRSTRLEN+1];
	time_t				ticks;
	struct sockaddr_in	servaddr, peer_addr;
	char host[NI_MAXHOST], service[NI_MAXSERV];

	if ( (sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr,"socket error : %s\n", strerror(errno));
		return 1;
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr   = 0;
	servaddr.sin_port   = htons(13);	/* daytime server */

	if ( bind( sfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		fprintf(stderr,"bind error : %s\n", strerror(errno));
		return 1;
	}

	delay.tv_sec =6;  //opoznienie na gniezdzie
	delay.tv_usec = 0; 
	len = sizeof(delay);
	if( setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &delay, len) == -1 ){
		fprintf(stderr,"SO_RCVTIMEO setsockopt error : %s\n", strerror(errno));
		return 1;
	}		

    struct utsname sys_info;
    time_t start_time = time(NULL);
    
    struct sockaddr_storage kr_addr; 
    socklen_t kr_len;

    printf("Klient uruchomiony. Nasłuchuję przez 6 sekund...\n");

    while (time(NULL) - start_time < 6) {
        kr_len = sizeof(kr_addr);
        
        n = recvfrom(sfd, recvline, MAXLINE, MSG_DONTWAIT, (struct sockaddr *) &kr_addr, &kr_len);
        
        if (n > 0) {
            recvline[n] = 0; /* null terminate */
            printf("Odebrano czas: %s", recvline);

            if (uname(&sys_info) == 0) {
                printf("[Uname] Nazwa hosta klienta: %s\n", sys_info.nodename);
                printf("[Uname] System operacyjny: %s (Wersja: %s)\n", sys_info.sysname, sys_info.release);
                printf("--------------------------------------------------\n");
            }
        }
        usleep(10000); 
    }
    printf("Minęło 6 sekund. Koniec działania klienta.\n");
		
	exit(EXIT_SUCCESS);
}
