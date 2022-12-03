/* 
 * echoservert.c - A concurrent echo server using threads
 */
/* $begin echoservertmain */
#include "csapp.h"

/* Rishabh Singhvi : rsinghvi@umass.edu
 * Xinyu Cao:  xinyucao@umass.edu
 */
#define NUM_CONNECTIONS 20 //server serves a maximum of 20 connections

struct clientfd_map{ //maps names to connfd
    char name[60];
    int connfd;
};

const char* delim = " ";

struct clientfd_map users[NUM_CONNECTIONS];
int cursor = 0;
pthread_mutex_t lock; // mutex to make sure threads don't add and remove elements in users array at the same time

void *thread_routine(void *vargp);
int removeConnfd(int);
int addConnfd(char*, int);
void listUsers();
void handleClient(int);
int findConnfd(char*);
char* getName(int);


int main(int argc, char** argv)
{
    int listenfd, *connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr, servaddr;
    pthread_t tid;
    rio_t rio;
    char name[30];

    int i;

    if(pthread_mutex_init(&lock, NULL)!=0)
    {
        fprintf(stderr, "Mutex init failed.\n");
        exit(0);
    }

    if(argc!=2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);

    while(1)
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Malloc(sizeof(int));
        *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Rio_readinitb(&rio, *connfd);
        recv(*connfd, name, 30, MSG_WAITALL);
        printf("@%s joined\n", name);

        pthread_mutex_lock(&lock); // lock the mutex
        if((addConnfd(name, *connfd)==-1))
        {
            printf("Error adding connection to the users list:\n");
            pthread_mutex_unlock(&lock); // unlock mutex and return so that socket could be closed
        }

        /*for(i=0; i<cursor; i++)
        {
            printf("%s, %d\n", users[i].name, users[i].connfd);
        }*/


        pthread_mutex_unlock(&lock);

        Pthread_create(&tid, NULL, thread_routine, connfd);
    }
    pthread_mutex_destroy(&lock);
    return 0;
}


/*
 *
 * function: removeConnfd(int)
 * "Removes" connfd from the users array after a user has disconnected
 * Shifts everything to the left to make sure no "holes" in the array
 * 
*/ 
int removeConnfd(int _connfd)
{
    int i, p;
    for(i = 0; i < cursor; i++)
    {
        if(users[i].connfd == _connfd)
        {
            p = i;
            break;
        }
    }
    if(i == cursor) // if i reaches cursor value, no such connfd in the array
    {
        return -1;
    }
    if(p==(cursor-1)) // if p is the last value in the array, just decrease cursor and return
    {
        cursor--;
        return 0;
    }
    for(i = p+1; i < cursor; i++) // shift every element in the array after the "found" element to the left
    {
        users[i-1] = users[i];
    }
    cursor--; //decrease cursor
    return 0;
}

/*
 *
 * function: addConnfd(int)
 * Adds connfd to the users array after a user has connected
 * 
 * 
*/ 
int addConnfd(char* _name, int _connfd)
{
    if(cursor == NUM_CONNECTIONS) //if cursor equal to NUM_CONNECTIONS, reached the maximum amount of connections possible
    {
        return -1;
    }
    strcpy(users[cursor].name, _name);
    users[cursor].connfd = _connfd;
    cursor++;
    return 0;
}


/*
 *
 * function: findConnfd(int)
 * Given a name, finds the connfd of the user. Names are case sensitive.
 * 
 * 
*/ 
int findConnfd(char* _name)
{
    int i;
    for(i = 0; i < cursor; i++)
    {
        if(strcmp(users[i].name, _name)==0)
        {
            return users[i].connfd;
        }
    }
    return -1;
}

void* thread_routine(void *vargp)
{
    int connfd = (*(int*)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    handleClient(connfd);
    Close(connfd);
    return NULL;
}



void handleClient(int connfd)
{
    int n;
    char buf[MAXLINE];
    char name[30];
    char sendbuf[MAXLINE];
    char* tempbuf[MAXLINE], *token, *msgbuf[MAXLINE]; 
    
    rio_t rio;

    Rio_readinitb(&rio, connfd);

    //Rio_readlineb(&rio, name, MAXLINE); // client sends name of the user to server
    //printf("@%s joined\n", name); // comment this out later
    
    
    //while((n = Rio_readlineb(&rio, buf, MAXLINE))!=0)
    while(1)
    {
	recv(connfd, buf, MAXLINE, NULL);
        if(buf==NULL)
        {
            continue;
        }

        if(strcmp(buf, "list-users\n")==0) //if user typed list-users
        {
            listUsers(msgbuf);
	    send(connfd, msgbuf, MAXLINE, NULL);
            continue;
        }
        else if(strcmp(buf, "quit\n")==0) //if user typed quit
        {
            strcpy(msgbuf, "Goodbye!");
            //Rio_writen(connfd, buf, strlen(buf));
	    send(connfd, msgbuf, MAXLINE, NULL); 
            pthread_mutex_lock(&lock);
            if((removeConnfd(connfd)==-1)) // "remove" connfd from the array
            {
                printf("Error removing connection to the users list:\n");
                pthread_mutex_unlock(&lock);
                exit(0);
            }
            pthread_mutex_unlock(&lock);
            //printf("@%s left\n", name);
            return;    
        }
        else
	{
	
	    
		
	    strcpy(tempbuf, buf); // copy data into a new buffer because buf will lose data
            int msgto; // variable to hold the connfd of the person to receive this message

		
            token = strtok(buf, delim);
	
		
            if(token[0]!='@')
            {
                continue;

            }
		
            if((msgto = findConnfd(&token[1]))==-1) // if no such connfd exists
            {
                continue;
            }

            
            strcpy(msgbuf, "@");
            strcat(msgbuf, getName(connfd));
	    strcat(msgbuf, delim);
            token = strtok(tempbuf, delim);
	    while(token!=NULL)
            {
                if(token[0]!='@')
                {
                    strcat(msgbuf, token);
                    strcat(msgbuf, delim);
                }
                token = strtok(NULL, delim);
            }

          
            send(msgto, msgbuf, MAXLINE, NULL);
	    
            
        }
    }

}

char* getName(int connfd)
{
    int i;
    for(i = 0; i < cursor; i++)
    {
        if(users[i].connfd == connfd)
        {
            return users[i].name;
        }
    }
    return NULL;
}
void listUsers(char* buf)
{
    int i;
    strcpy(buf, "\0");
    
    for(i = 0; i < cursor; i++)
    {
    //    printf("@%s\n", users[i].name);
    	strcat(buf, "@");
	strcat(buf, users[i].name);
	strcat(buf, "\n");
    }
}






/*void *thread(void *vargp);

void echo(int connfd)
{
    int n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", n);
        Rio_writen(connfd, buf, n);
    }
}

int main(int argc, char **argv) 
{
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid; 

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }
    listenfd = Open_listenfd(argv[1]);

    while (1) {
        clientlen=sizeof(struct sockaddr_storage);
	connfdp = Malloc(sizeof(int)); //line:conc:echoservert:beginmalloc
	*connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen); //line:conc:echoservert:endmalloc
	Pthread_create(&tid, NULL, thread, connfdp);
    }
}

/* Thread routine */
/*void *thread(void *vargp) 
{  
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self()); //line:conc:echoservert:detach
    Free(vargp);                    //line:conc:echoservert:free
    echo(connfd);
    Close(connfd);
    return NULL;
}
/* $end echoservertmain */
