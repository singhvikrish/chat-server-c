/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */


/* Rishabh Singhvi : rsinghvi@umass.edu
 * Xinyu Cao: xinyucao@umass.edu
*/

#include "csapp.h"

int EXITFLAG = 0;

void *receive(void*);

int main(int argc, char **argv) 
{
    int clientfd;
    char *host, *port, *name, buf[MAXLINE];
    rio_t rio, rio2;
    pthread_t tid;

    if (argc != 4) 
    {
	    fprintf(stderr, "usage: %s <host> <port> <name>\n", argv[0]);
	    exit(0);
    }
    host = argv[1];
    port = argv[2];
    //name = argv[3];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);
    Rio_readinitb(&rio2, clientfd);

    //Rio_writen(clientfd, argv[3], strlen(argv[3])); // send name to server before beginning communication

    send(clientfd, argv[3], 30, NULL);

    //while (Fgets(buf, MAXLINE, stdin) != NULL) {
	//Rio_writen(clientfd, buf, strlen(buf));
	//Rio_readlineb(&rio, buf, MAXLINE);
	//Fputs(buf, stdout);
    //}
    Pthread_create(&tid, NULL, receive, &clientfd);

    while(EXITFLAG == 0)
    {
        printf("> ");
        Fgets(buf, MAXLINE, stdin);
        //Rio_writen(clientfd, buf, strlen(buf));
        send(clientfd, buf, MAXLINE, NULL);
    }
    
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}
/* $end echoclientmain */


void *receive(void* vargp)
{
    //rio_t rio2 = (*(rio_t*)vargp);
    int cfd = (*(int*)vargp);
    Pthread_detach(pthread_self());
    //Free(vargp);
    char buf[MAXLINE];
    while(EXITFLAG == 0)
    {
        //Rio_readlineb(&rio2, buf, MAXLINE);
        recv(cfd, buf, MAXLINE, NULL);
        printf("\b%s\n",buf);
        if(strcmp(buf, "Goodbye!")==0)
        {
            EXITFLAG = 1;
        }
    }
}
/*int main(int argc, char **argv) 
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) {
	fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
	exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
	Rio_writen(clientfd, buf, strlen(buf));
	Rio_readlineb(&rio, buf, MAXLINE);
	Fputs(buf, stdout);
    }
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}*/
