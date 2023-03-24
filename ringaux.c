#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <signal.h>
#include <unistd.h>



#include "ringaux.h"

void tsetHints(struct addrinfo *hints)
{
    memset(hints, 0, sizeof(*hints));
    hints->ai_family = AF_INET;       // IPv4
    hints->ai_socktype = SOCK_STREAM; // TCP socket
}

void tcsetHints(struct addrinfo *hints)
{
    memset(hints, 0, sizeof(*hints));
    hints->ai_family = AF_INET;       // IPv4
    hints->ai_socktype = SOCK_STREAM; // TCP socket
    hints->ai_flags = AI_PASSIVE;
}

void ucsetHints(struct addrinfo *hints)
{
    memset(hints,0,sizeof(*hints));
    hints->ai_family=AF_INET;//IPv4
    hints->ai_socktype=SOCK_DGRAM;//UDP socket
}

void usetHints(struct addrinfo *hints)
{
    memset(hints,0,sizeof(*hints));
    hints->ai_family=AF_INET;//IPv4
    hints->ai_socktype=SOCK_DGRAM;//UDP socket
    hints->ai_flags=AI_PASSIVE;
}


void printUI()
{
    printf("Insira a letra correspondente à ação que quer tomar.\n");
}

int createtSocket()
{
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exit(1);

    printf("createsocket fd: %d\n", fd);
    return fd;
}

int createuSocket()
{
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        exit(1);

    printf("createsocket UDP fd: %d\n", fd);
    return fd;
}

char *createSelf(int key, char *iip, char *iport)
{
    char *str, *straux;
    str = (char *) malloc(100);
    straux = (char *) malloc(10);

    if(iip == NULL){
        iip = "NULL";
    }

    sprintf(str, "SELF ");
    sprintf(straux, "%d", key);

    strcat(str, straux); 
    strcat(str, " "); 
    strcat(str, iip); 
    strcat(str, " "); 
    strcat(str, iport); 
    strcat(str, "\n");  
    free(straux);
    return str;
}

int Read(char *buffer, int fd)
{
    char *ptr;
    ssize_t nleft, nread;
    ptr = buffer;
    nleft = 100;
    while(nleft>0){
        nread = read(fd,ptr,nleft);
        if(nread==-1)
        {
            printf("Couldn't read\n");
            exit(1);
        }
        else if(nread==0){
            if(nleft == 100){
                return 1;
            } 
            break;
        }       // Para o caso em que a ligação é fechada "do outro lado"
        nleft-=nread;

        ptr+=nread;
        if((*(ptr-1)) == '\n'){
            ptr[0] = '\0';
            break;
        } 
    }
    

    return 0;
}

int ReadU(char *buffer, int fdUDP, struct sockaddr *addr, socklen_t *addrlen)
{
    char *ptr;
    ssize_t  nread;
    ptr = buffer;

    nread = recvfrom(fdUDP, ptr, 100, 0, addr, addrlen);
    ptr[nread] = '\0';

    return 0;
}

void Write(char *buffer, char *str, int fd1)
{
    char *ptr;
    ssize_t nbytes, nleft, nwritten;

    ptr = strcpy(buffer, str);

    nbytes = strlen(ptr);

    ptr[nbytes] = '\0';

    nleft = nbytes;
    while (nleft > 0)
    {
        nwritten = write(fd1, ptr, nleft);
        if (nwritten <= 0) /*error*/
            exit(1);
        nleft -= nwritten;

        ptr += nwritten;

    }
}

int WriteU(char buffer[100], char str[100], int fdUDP, struct sockaddr *addrinfo, socklen_t addrlen) 
{
    char *ptr;
    ssize_t nbytes, nleft, nwritten;

    ptr = strcpy(buffer, str);

    nbytes = strlen(ptr);

    nleft = nbytes;
    while (nleft > 0)
    {
        nwritten = sendto(fdUDP, ptr, nleft, 0, addrinfo, addrlen);

        if (nwritten <= 0) /*error*/
            exit(1);
        nleft -= nwritten;

        ptr += nwritten;

    }
    return 0;
}

char *createPred(int pkey, char *pIP, char *pPort)
{
    char *str, *straux;
    str = (char *) malloc(100);
    straux = (char *) malloc(10);

    sprintf(str, "PRED ");
    sprintf(straux, "%d", pkey);


    sprintf(str, "PRED ");
    sprintf(straux, "%d", pkey);


    strcat(str, straux); 
    strcat(str, " "); 
    strcat(str, pIP); 
    strcat(str, " "); 
    strcat(str, pPort); 
    strcat(str, "\n");  
    free(straux);
    return str;
}

int findMinFree(struct sockaddr**senderArray, int arraySize)
{
    int i;
    for(i = 1; i < arraySize; i++){
        if(senderArray[i] == NULL){
            return i;
        }
    }
    return -1;
}