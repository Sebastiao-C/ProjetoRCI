#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <signal.h>
#include <unistd.h>



#include "ringaux.h"

void setHints(struct addrinfo *hints)
{
    memset(hints, 0, sizeof *hints);
    hints->ai_family = AF_INET;       // IPv4
    hints->ai_socktype = SOCK_STREAM; // TCP socket
}

void fsetHints(struct addrinfo *hints)
{
    memset(hints, 0, sizeof *hints);
    hints->ai_family = AF_INET;       // IPv4
    hints->ai_socktype = SOCK_STREAM; // TCP socket
    hints->ai_flags = AI_PASSIVE;
}

void usetHints(struct addrinfo *hints)
{
    memset(&hints,0,sizeof hints);
    hints->ai_family=AF_INET;//IPv4
    hints->ai_socktype=SOCK_DGRAM;//UDP socket
    hints->ai_flags=AI_PASSIVE;
}


void printUI()
{
    printf("Insira a letra correspondente à ação que quer tomar.\n");
    printf("Menu:\n");
    printf("n: Criação de um anel contendo apenas o nó.\n");
    printf("p: Entrada do nó no anel sabendo que o seu predecessor será o nó pred com endereço IP pred.IP e porto pred.port.\n");
    printf("l: Saída do nó do anel atual.\n");    
    printf("x: Fecho da aplicação.\n");
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
    // VOLTAR A PÔR ISTO BEM. O PROBLEMA ERA SÓ DAR STRCAT A UM NULL, QUE É O IIP...
    char *str, *straux;
    str = (char *) malloc(100);
    straux = (char *) malloc(10);

    if(iip == NULL){
        iip = "NULL";
    }

    sprintf(str, "SELF ");
    sprintf(straux, "%d", key);

    printf("%s", iip);
    printf("%s", iport);
    strcat(str, straux); 
    strcat(str, " "); 
    strcat(str, iip); 
    strcat(str, " "); 
    strcat(str, iport); 
    strcat(str, "\n");  
    printf("%s", str);
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
        printf("nread = %ld\n", nread);
        if(nread==-1)
        {
            printf("couldn't read\n");
            exit(1);
        }
        else if(nread==0){
            if(nleft == 100){
                return 1;
            } 
            break;
        }//closed by peer
        nleft-=nread;

        ptr+=nread;
        if((*(ptr-1)) == '\n') break;
    }
    return 0;
}

int ReadU(char *buffer, int fdUDP)
{
    char *ptr;
    ssize_t nleft, nread;
    ptr = buffer;
    nleft = 100;
    struct sockaddr addr;
    socklen_t addrlen;
    
    addrlen = sizeof(addr);     // ??


    while(nleft>0){
        nread = recvfrom(fdUDP, ptr, 100, 0, &addr, &addrlen);
        printf("nread = %ld\n", nread);

        if(nread==-1)
        {
            printf("couldn't read\n");
            exit(1);
        }
        
        else if(nread==0){
            /*
            if(nleft == 100){
                return 1;
            } 
            */
            break;
        }
        nleft-=nread;

        ptr+=nread;
        if((*(ptr-1)) == '\n') break;
    }
    return 0;
}

void Write(char *buffer, char *str, int fd1)
{
    char *ptr;
    ssize_t nbytes, nleft, nwritten, nread;

    ptr = strcpy(buffer, str);
        printf("yup3\n");

    nbytes = strlen(ptr);
    //printf("strlen is %d\n", nbytes);
    ptr[nbytes] = '\0';     // Em princípio não faz nada

    nleft = nbytes;
    while (nleft > 0)
    {
        nwritten = write(fd1, ptr, nleft);
        if (nwritten <= 0) /*error*/
            exit(1);
        nleft -= nwritten;
        //printf("nleft is %d\n", nleft);

        ptr += nwritten;

    }
}

int WriteU(char *buffer, char *str, int fdUDP)
{

}

char *createPred(int pkey, char *pIP, char *pPort)
{
    char *str, *straux;
    str = (char *) malloc(100);
    straux = (char *) malloc(10);

    sprintf(str, "PRED ");
    sprintf(straux, "%d", pkey);

    // TRATAR DISTO, PÔR IGUAL AO SELF

    sprintf(str, "PRED ");
    sprintf(straux, "%d", pkey);

    printf("%s", pIP);
    printf("%s", pPort);
    strcat(str, straux); 
    strcat(str, " "); 
    strcat(str, pIP); 
    strcat(str, " "); 
    strcat(str, pPort); 
    strcat(str, "\n");  
    printf("%s", str);
    free(straux);
    return str;
}