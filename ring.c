#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>
#include <unistd.h>
#include <signal.h>

#include "ringaux.h"

#define max(A,B) ((A)>=(B)?(A):(B))

// Variáveis globais que contêm o estado atual do nó

int key;
char IP[100];
char Port[100];

int sKey;
char sIP[100];
char sPort[100];

int pKey;
char pIP[100];
char pPort[100];

int fdTCP;
int fdSuc;
int fdPred;
int fdUDP;

int cKey;
char cIP[100];
char cPort[100];
struct sockaddr *caddr;
socklen_t clen;


int inARing;
int maxfd;
int currentN;
int fndCameFromUser;



int ReadTimeout(int fd, int sec)
{
    fd_set rset;
    struct timeval tv;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    tv.tv_sec = sec;
    tv.tv_usec = 0;

    return (select(fd + 1, &rset, NULL, NULL, &tv));
        /* > 0 if descriptor is readable */
}

void setToZero()
{
    sKey = 0;
    strcpy(sIP, "");
    strcpy(sPort, "");

    pKey = 0;
    strcpy(pIP, "");
    strcpy(pPort, "");

    cKey = 0;
    strcpy(cIP, "");
    strcpy(cPort, "");

    caddr = NULL;
    clen = 0;

    fdSuc = 0;
    fdPred = 0;

    inARing = 0;
}

int getMaxFD(){
    int fdmax;
    fdmax = max(fdPred, fdSuc);
    fdmax = max(fdmax, fdUDP);
    return max(fdmax, fdTCP);
}

void showInfo()
{
    /*
    printf("fdTCP: %d\n", fdTCP);
    printf("fdSuc: %d\n", fdSuc);
    printf("fdPred %d\n", fdPred);
    */
    printf("Key: %d\n", key);
    printf("IP: %s\n", IP);
    printf("Port: %s\n\n", Port);    
    printf("sKey: %d\n", sKey);
    printf("sIP: %s\n", sIP);
    printf("sPort: %s\n\n", sPort);
    printf("pKey: %d\n", pKey);
    printf("pIP: %s\n", pIP);
    printf("pPort: %s\n\n", pPort);
    if(cKey != 0){
        printf("cKey: %d\n", cKey);
        printf("cIP: %s\n", cIP);
        printf("cPort: %s\n", cPort);
    }
}

int createRing()
{   

    // O nó passa a estar ligado a si próprio

    sKey = key;
    strcpy(sIP, IP);
    strcpy(sPort, Port);

    pKey = key;
    strcpy(pIP, IP);
    strcpy(pPort, Port);

    //printf("Anel criado com sucesso. Sucessor deste nó é o próprio.\n");

    return 0;
}

int pEntry(int predkey, char *predIP, char *predPort){

    struct addrinfo hints, *res;
    int fd1, errcode;

    ssize_t n;

    struct sigaction act;

    char buffer[128 + 1];

    fd1 = createtSocket(); // Se houver um erro, o programa termina

    tcsetHints(&hints);

    if(!strcmp(predIP, "NULL")){
        if ((errcode = getaddrinfo(NULL, predPort, &hints, &res)) != 0){ /*error*/
            printf("Não consegui receber addrinfo.\n");
            return 1;
        }
    }
    else if ((errcode = getaddrinfo(predIP, predPort, &hints, &res)) != 0){ /*error*/
        printf("Não consegui receber addrinfo.\n");
        return 1;
    }

    n = connect(fd1, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/
    {
        printf("Não conseguir fazer connect\n");
        return 1;
    }

    memset(&act, 0, sizeof act);

    act.sa_handler = SIG_IGN;

    if (sigaction(SIGPIPE, &act, NULL) == -1) /*error*/
        exit(1);

    char *str;
    str = createSelf(key, IP, Port);

    Write(buffer, str, fd1);


    // Atualização do estado do nó

    fdPred = fd1;
    if(fdPred > maxfd) maxfd = fdPred;
    pKey = predkey;
    strcpy(pIP, predIP);
    strcpy(pPort, predPort);

    return 0;

}

// Cria os dois servidores, TCP e UDP, e guarda os fds correspondentes nas variáveis globais

int createServer(struct addrinfo *hints, struct addrinfo **res){     
    int errcode;
    
    fdTCP = createtSocket(); 
    maxfd = fdTCP;
    tsetHints(hints);

    if(!strcmp(IP, "NULL"))
    {
        if ((errcode = getaddrinfo(NULL, Port, hints, res)) != 0){ /*error*/
            printf("Não consegui receber addrinfo.\n");
            return 1;
        }
    }
    else if ((errcode = getaddrinfo(IP, Port, hints, res)) != 0){ /*error*/
        printf("Não consegui receber addrinfo.\n");
        return 1;
    }

    if (bind(fdTCP, (*res)->ai_addr, (*res)->ai_addrlen) == -1){ /*error*/
        printf("Não consegui fazer bind.\n");
        return 1;
    }
    if (listen(fdTCP, 5) == -1){ /*error*/
        printf("Não consegui fazer listen.\n");
        return 1;
    }

    fdUDP = createuSocket();
    if(fdUDP > maxfd) maxfd = fdUDP;
    usetHints(hints);

    if(!strcmp(IP, "NULL"))
    {
        if ((errcode = getaddrinfo(NULL, Port, hints, res)) != 0)
        { /*error*/
            printf("Não consegui receber addrinfo.\n");
            return 1;
        }
    }
    else if ((errcode = getaddrinfo(IP, Port, hints, res)) != 0){ /*error*/
        printf("Não consegui receber addrinfo.\n");
        return 1;
    } 
    
    if(bind(fdUDP,(*res)->ai_addr, (*res)->ai_addrlen)==-1)/*error*/
    {
        printf("Não consegui fazer bind.\n");
    }

    return 0;
}


void leave()
{
    if(fdSuc != 0) close(fdSuc);
    if(fdPred != 0) close(fdPred);
    setToZero();
}

void exitProgram()
{
    leave();
    close(fdTCP);
    close(fdUDP);
    exit(0);
}


void checkInput(int isThereChar)
{
    char input;
    int cont = 1;
    int auxkey = 0;
    char params[100], auxip[100], auxport[100], buffer[200];
    char *str;
    struct addrinfo hints, *res;
    char auxIP[20], auxPort[20];
    char auxstr[100];
    int errcode;
    int i;

    while (cont)
    {
        if(isThereChar)
        {

            input = fgetc(stdin);
        }
        else{
            printUI();
            input = fgetc(stdin);
        }
        if(input == '\n') return;
        else fgets(params, 100, stdin);

        switch (input)
        {
        case 'n':

            if((sKey != 0) || (pKey != 0)){
                printf("O nó já está num anel.\n");
                return;
            }
            else if(!createRing()){           
                inARing = 1;
                return;
            }
            else{
                printf("Erro na função createRing\n");
                exit(0);
            }
            return;

        case 'p':
            if((sKey != 0) || (pKey != 0)){
                printf("O nó já está num anel.\n");
                return;
            }
            if(sscanf(params, "%d %s %s", &auxkey, auxip, auxport) != 3)
            {
                printf("Formato errado.\n");
                return;
            }
            
            pEntry(auxkey, auxip, auxport);

            inARing = 1;
            return;
        case 'x':
            if((strcmp(sPort, "") != 0 ) && (strcmp(sPort, Port) != 0)) 
            {
                str = createPred(pKey, pIP, pPort);
                Write(buffer, str, fdSuc); 
            } 
            exitProgram();
        case 'l':
            if((sKey == 0) && (pKey == 0)){
                printf("O nó não está num anel.\n");
                return;
            }
            if((strcmp(sPort, "") != 0 ) && (strcmp(sPort, Port) != 0))
            {
                str = createPred(pKey, pIP, pPort);
                Write(buffer, str, fdSuc); 
            } 
            leave();
            return;
        case 'f':
            if((sKey == 0) && (pKey == 0)){
                printf("O nó não está num anel.\n");
                return;
            }
            int findk;
            if(sscanf(params, "%d", &findk) != 1)
            {
                printf("Formato errado.\n");
                return;
            }
            printf("findk: %d\n", findk);

            if(((findk >= key) && (findk <= sKey)) || ((sKey < key) && ((findk > key) || (findk < sKey)))) 
            {
                printf("A chave pertence a %d %s %s\n", key, IP, Port);

            }
            else{

                if((cKey == 0) || (((findk >= key) && (findk <= cKey)) || ((cKey < key) && ((findk > key) || (findk < cKey)))))
                {
                    sprintf(params, "%s %d %d %d %s %s\n", "FND", findk, 1, key, IP, Port);
                    Write(buffer, params, fdSuc);
                }
                else
                {
                    sprintf(params, "%s %d %d %d %s %s", "FND", findk, 1, key, IP, Port);
                    WriteU(buffer, params, fdUDP, caddr, clen);
                    for(i = 0; i < 3; i++){
                        if(ReadTimeout(fdUDP, 2) > 0){
                            ReadU(buffer, fdUDP, caddr, &clen);
                            return;
                        }
                        else printf("Ainda não recebi...\n");
                    }
                    printf("Não recebi um ACK, não sei se chegou... A enviar um FND para o sucessor\n");
                    sprintf(params, "%s %d %d %d %s %s\n", "FND", findk, 1, key, IP, Port);
                    Write(buffer, params, fdSuc);
                }
                
            }

            return;
        case 'b':
            if((sKey != 0) || (pKey != 0)){
                printf("O nó já está num anel.\n");
                return;
            }
            if(sscanf(params, "%d %s %s", &auxkey, auxIP, auxPort) != 3)
            {
                printf("Formato errado.\n");
                return;
            }
            sprintf(auxstr, "EFND %d", key);
            ucsetHints(&hints);

            if(!strcmp(auxIP, "NULL"))
            {
                if ((errcode = getaddrinfo(NULL, auxPort, &hints, &res)) != 0)
                { /*error*/
                    printf("Não consegui receber addrinfo.\n");
                    exit(1);
                }
            }
            else if ((errcode = getaddrinfo(auxIP, auxPort, &hints, &res)) != 0){ /*error*/
                    printf("Não consegui receber addrinfo.\n");
                    exit(1);
            } 
            WriteU(buffer, auxstr, fdUDP, (res)->ai_addr, (res)->ai_addrlen);
            
            for(i = 0; i < 3; i++){
                if(ReadTimeout(fdUDP, 2) > 0){
                    ReadU(buffer, fdUDP, (res)->ai_addr, &(res)->ai_addrlen);
                    return;
                }
                else printf("Ainda não recebi...\n");
            }
            printf("Não recebi um ACK, não sei se chegou...\n");

            return;
        case 'c':
            if((sKey == 0) && (pKey == 0)){
                printf("O nó não está num anel.\n");
                return;
            }
            sscanf(params, "%d %s %s", &auxkey, auxip, auxport);

            cKey = auxkey;
            strcpy(cIP, auxip);
            strcpy(cPort, auxPort);

            ucsetHints(&hints);

            if(!strcmp(auxip, "NULL"))
            {
                if ((errcode = getaddrinfo(NULL, auxport, &hints, &res)) != 0)
                { /*error*/
                    printf("Não consegui receber addrinfo.\n");
                    exit(1);
                }
            }
            else if ((errcode = getaddrinfo(auxip, auxport, &hints, &res)) != 0){ /*error*/
                    printf("Não consegui receber addrinfo.\n");
                    exit(1);
            } 
            caddr = res->ai_addr;
            clen = res->ai_addrlen;
            return;
        case 'e':
            if((sKey == 0) && (pKey == 0)){
                printf("O nó não está num anel.\n");
                return;
            }
            else if(cKey == 0)
            {
                printf("Não existe atalho.\n");
                return;
            }
            cKey = 0;
            strcpy(cIP, "");
            strcpy(cPort, "");
            caddr = NULL;
            clen = 0;
            return;
        case 's':
            showInfo();
            return;
        default:
            printf("Não é um input válido.\n");
            return;
        }
    }
}

int main(int argc, char **argv)
{
    setToZero();

    // Declarações

    maxfd = 0;
    struct sockaddr *addresses[99];
    socklen_t lens[99];
    currentN = 0;
    int i;
    char auxstr[100];
    int sent = 0;

    
    for(i = 0; i < 99; i++){
        addresses[i] = NULL;
        lens[i] = 0;
    }
    

    fd_set rfds;

    struct addrinfo hints, *res;
    int newfd, errcode;

    ssize_t n;
    struct sockaddr addr;
    socklen_t addrlen;

    struct sigaction act;

    char buffer[128 + 1];
    char buffer2[200];

    int x;
    char fst[20], scd[20], thd[20], frt[20], fft[20], sxt[20];
    char *str;

    if ((argc < 4) || (argc > 4))
    {
        printf("Não há argumentos suficientes.\n");
        exit(EXIT_FAILURE);
    }

    key = atoi(argv[1]); 
    strcpy(IP, argv[2]);
    strcpy(Port, argv[3]);


    createServer(&hints, &res); 

    inARing = 0;

    checkInput(0);
    int alreadyRead = 0;

    FD_ZERO(&rfds);    FD_SET(0, &rfds);   FD_SET(fdTCP, &rfds);   FD_SET(fdPred, &rfds);


    while (1)
    {   
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);   FD_SET(fdSuc, &rfds);   FD_SET(fdPred, &rfds);   FD_SET(fdUDP, &rfds);  FD_SET(fdTCP, &rfds);  
        maxfd = getMaxFD();   

        select(maxfd + 1, &rfds, NULL, NULL, NULL);  

        alreadyRead = 0;
        addrlen = sizeof(addr);   
        if((x = FD_ISSET(0, &rfds)) != 0)
        {

            checkInput(1);
            FD_CLR(0, &rfds);
        }      

        if ((x = FD_ISSET(fdSuc, &rfds)) != 0)
        {
            if(Read(buffer, fdSuc) == 1){ 
                if(fdSuc != fdPred)
                {
                    sKey = 0;                   
                    strcpy(sIP, "");
                    strcpy(sPort, "");
                    FD_CLR(fdSuc, &rfds);    
                }
                else
                {
                    sKey = key;                  
                    strcpy(sIP, IP);
                    strcpy(sPort, Port);
                    pKey = key;
                    strcpy(pIP, IP);
                    strcpy(pPort, Port);
                    fdPred = 0;
                }
                close(fdSuc);
                fdSuc = 0;
                continue;
            }
            alreadyRead = 1;        // Porque fdPred e fdSuc podem ser iguais!

        }

        if ((x = FD_ISSET(fdPred, &rfds)) != 0)
        {

            if(alreadyRead == 1)
            {
            }
            else
            {
                if(Read(buffer, fdPred) == 1){
                    close(fdPred);
                    pKey = 0;
                    strcpy(pIP, "");
                    strcpy(pPort, "");
                    fdPred = 0;
                    FD_CLR(fdPred, &rfds);
                    continue;
                }
            }

            sscanf(buffer, "%s", fst);

            if(!strcmp(fst, "SELF"))        // Para o caso em que ainda temos só 2 nós e o outro quer informar-nos disso
            {
                sscanf(buffer, "%s %s %s %s", fst, scd, thd, frt);
                sKey = atoi(scd);
                strcpy(sIP, thd);
                strcpy(sPort, frt);
                fdSuc = fdPred;
            }
            if(!strcmp(fst, "PRED"))        
            {

                sscanf(buffer, "%s %s %s %s", fst, scd, thd, frt);
                
                int fdPAux;
                fdPAux = createtSocket();
                tcsetHints(&hints); 
                if(strcmp(thd,"NULL") == 0){
                    if ((errcode = getaddrinfo(NULL, frt, &hints, &res)) != 0) /*error*/
                    exit(1);
                }
                else if ((errcode = getaddrinfo(thd, frt, &hints, &res)) != 0) /*error*/
                    exit(1);

                n = connect(fdPAux, res->ai_addr, res->ai_addrlen);
                if (n == -1) /*error*/
                {
                    printf("Não consegui fazer connect\n");
                    exit(1);
                }

                memset(&act, 0, sizeof act);

                act.sa_handler = SIG_IGN;


                if (sigaction(SIGPIPE, &act, NULL) == -1) /*error*/
                    exit(1);

                str = createSelf(key, IP, Port);
                Write(buffer, str, fdPAux);
                if(fdSuc != fdPred) close(fdPred);
                fdPred = fdPAux;
                pKey = atoi(scd);
                strcpy(pIP, thd);
                strcpy(pPort, frt);

            }
            if(!strcmp(fst, "FND"))        
            {
                sscanf(buffer, "%s %s %s %s %s %s", fst, scd, thd, frt, fft, sxt);
                if(((atoi(scd) >= key) && (atoi(scd) <= sKey)) || ((sKey < key) && ((atoi(scd) > key) || (atoi(scd) < sKey))))  
                {
                    strcpy(scd, frt);

                    if((cKey == 0) || (((atoi(scd) >= key) && (atoi(scd) <= cKey)) || ((cKey < key) && ((atoi(scd) > key) || (atoi(scd) < cKey)))))
                    {
                        sprintf(auxstr, "RSP %s %s %d %s %s\n", scd, thd, key, IP, Port);
                        Write(buffer, auxstr, fdSuc);
                    }
                    else
                    {
                        sprintf(auxstr, "RSP %s %s %d %s %s", scd, thd, key, IP, Port);
                        WriteU(buffer, auxstr, fdUDP, caddr, clen);
                        sent = 0;
                        for(i = 0; i < 3; i++){
                            if(ReadTimeout(fdUDP, 2) > 0){
                                ReadU(buffer, fdUDP, caddr, &clen);
                                sent = 1;
                                break;
                            }
                            else printf("Ainda não recebi...\n");
                        }
                        if(sent == 0)
                        {
                            printf("Não recebi um ACK, não sei se chegou... A enviar um FND para o sucessor\n");
                            sprintf(auxstr, "RSP %s %s %d %s %s\n", scd, thd, key, IP, Port);
                            Write(buffer, auxstr, fdSuc);
                        }

                    }
                }
                else{

                    if((cKey == 0) || (((atoi(scd) >= key) && (atoi(scd) <= cKey)) || ((cKey < key) && ((atoi(scd) > key) || (atoi(scd) < cKey)))))
                    {
                        strcpy(auxstr, buffer);
                        Write(buffer, auxstr, fdSuc);
                    }
                    else
                    {
                        sprintf(auxstr, "%s %s %s %s %s %s", fst, scd, thd, frt, fft, sxt);
                        WriteU(buffer, auxstr, fdUDP, caddr, clen);
                        sent = 0;
                        for(i = 0; i < 3; i++){
                            if(ReadTimeout(fdUDP, 2) > 0){
                                ReadU(buffer, fdUDP, caddr, &clen);
                                sent = 1;
                                break;
                            }
                            else printf("Ainda não recebi...\n");
                        }
                        if(sent == 0)
                        {
                            printf("Não recebi um ACK, não sei se chegou... A enviar um FND para o sucessor\n");
                            sprintf(auxstr, "%s %s %s %s %s %s\n", fst, scd, thd, frt, fft, sxt);
                            Write(buffer, auxstr, fdSuc);
                        }

                    }
                    
                }

            }
            else if(!strcmp(fst, "RSP"))
            {
                sscanf(buffer, "%s %s %s %s %s %s", fst, scd, thd, frt, fft, sxt);
                if(atoi(scd) == key)
                {
                    if(strcmp(thd, "1") == 0){
                        printf("A chave pertence a: %s, %s, %s\n", frt, fft, sxt);       
                    }
                    else{
                        int n;
                        n = atoi(thd);
                        strcpy(scd, frt);
                        strcpy(thd, fft);
                        strcpy(frt, sxt);
                        sprintf(auxstr, "EPRED %s %s %s", scd, thd, frt);
                        WriteU(buffer, auxstr, fdUDP, addresses[n-1], lens[n-1]);
                        ReadU(buffer, fdUDP, addresses[n-1], &(lens[n-1]));
                        addresses[n-1] = NULL;
                        lens[n-1] = 0;
                    }
                }
                else{
                    if((cKey == 0) || (((atoi(scd) >= key) && (atoi(scd) <= cKey)) || ((cKey < key) && ((atoi(scd) > key) || (atoi(scd) < cKey)))))
                    {
                        strcpy(auxstr, buffer);
                        Write(buffer, auxstr, fdSuc);
                    }
                    else
                    {
                        sprintf(auxstr, "%s %s %s %s %s %s", fst, scd, thd, frt, fft, sxt);
                        WriteU(buffer, auxstr, fdUDP, caddr, clen);
                        sent = 0;
                        for(i = 0; i < 3; i++){
                            if(ReadTimeout(fdUDP, 2) > 0){
                                ReadU(buffer, fdUDP, caddr, &clen);
                                sent = 1;
                                break;
                            }
                            else printf("Ainda não recebi...\n");
                        }
                        if(sent == 0)
                        {
                            printf("Não recebi um ACK, não sei se chegou... A enviar um FND para o sucessor\n");
                            sprintf(auxstr, "%s %s %s %s %s %s\n", fst, scd, thd, frt, fft, sxt);
                            Write(buffer, auxstr, fdSuc);
                        }
                        
                    }
                }
            }


            FD_CLR(fdPred, &rfds);
        }

        if ((x = FD_ISSET(fdUDP, &rfds)) != 0)
        {

            ReadU(buffer, fdUDP, &addr, &addrlen);
            char *strack = "ACK";
            WriteU(buffer2, strack, fdUDP, &addr, addrlen);

            sscanf(buffer, "%s", fst);

            if(!strcmp(fst, "FND"))      
            {
                sscanf(buffer, "%s %s %s %s %s %s", fst, scd, thd, frt, fft, sxt);
                if(((atoi(scd) >= key) && (atoi(scd) <= sKey)) || ((sKey < key) && ((atoi(scd) > key) || (atoi(scd) < sKey))))
                {
                    
                    if((cKey == 0) || (((atoi(scd) >= key) && (atoi(scd) <= cKey)) || ((cKey < key) && ((atoi(scd) > key) || (atoi(scd) < cKey)))))
                    {
                        strcpy(scd, frt);
                        sprintf(auxstr, "RSP %s %s %d %s %s\n", scd, thd, key, IP, Port);
                        Write(buffer, auxstr, fdSuc);
                    }
                    else
                    {
                        strcpy(scd, frt);
                        sprintf(auxstr, "RSP %s %s %d %s %s", scd, thd, key, IP, Port);
                        WriteU(buffer, auxstr, fdUDP, caddr, clen);
                        sent = 0;
                        for(i = 0; i < 3; i++){
                            if(ReadTimeout(fdUDP, 2) > 0){
                                ReadU(buffer, fdUDP, caddr, &clen);
                                sent = 1;
                                break;
                            }
                            else printf("Ainda não recebi...\n");
                        }
                        if(sent == 0)
                        {
                            printf("Não recebi um ACK, não sei se chegou... A enviar um FND para o sucessor\n");
                            strcpy(scd, frt);
                            sprintf(auxstr, "RSP %s %s %d %s %s\n", scd, thd, key, IP, Port);
                            Write(buffer, auxstr, fdSuc);
                        }


                    }
                    
                }
                else{
                    if((cKey == 0) || (((atoi(scd) >= key) && (atoi(scd) <= cKey)) || ((cKey < key) && ((atoi(scd) > key) || (atoi(scd) < cKey)))))
                    {
                        strcpy(auxstr, buffer);
                        strcat(auxstr, "\n");
                        Write(buffer, auxstr, fdSuc);
                    }
                    else
                    {
                        strcpy(auxstr, buffer);
                        WriteU(buffer, auxstr, fdUDP, caddr, clen);
                        sent = 0;
                        for(i = 0; i < 3; i++){
                            if(ReadTimeout(fdUDP, 2) > 0){
                                ReadU(buffer, fdUDP, caddr, &clen);
                                sent = 1;
                                break;
                            }
                            else printf("Ainda naõ recebi...\n");
                        }
                        if(sent == 0)
                        {
                            printf("Não recebi um ACK, não sei se chegou... A enviar um FND para o sucessor\n");
                            strcpy(auxstr, buffer);
                            strcat(auxstr, "\n");
                            Write(buffer, auxstr, fdSuc);
                        }
                        

                    }
                }

            }
            else if(!strcmp(fst, "RSP"))
            {
                sscanf(buffer, "%s %s %s %s %s %s", fst, scd, thd, frt, fft, sxt);
                if(atoi(scd) == key)
                {
                    if(strcmp(thd, "1") == 0){
                        printf("A chave pertence a: %s, %s, %s\n", frt, fft, sxt);       
                    }
                    else{
                        int n;
                        n = atoi(thd);
                        strcpy(scd, frt);
                        strcpy(thd, fft);
                        strcpy(frt, sxt);
                        sprintf(auxstr, "EPRED %s %s %s", scd, thd, frt);
                        WriteU(buffer, auxstr, fdUDP, addresses[n-1], lens[n-1]); 
                        ReadU(buffer, fdUDP, addresses[n-1], &(lens[n-1]));

                        addresses[n-1] = NULL;
                        lens[n-1] = 0;
                    }
                }
                else{
                    if((cKey == 0) || (((atoi(scd) >= key) && (atoi(scd) <= cKey)) || ((cKey < key) && ((atoi(scd) > key) || (atoi(scd) < cKey)))))
                    {
                        strcpy(auxstr, buffer);
                        strcat(auxstr, "\n");
                        Write(buffer, auxstr, fdSuc);
                    }
                    else
                    {
                        strcpy(auxstr, buffer);
                        WriteU(buffer, auxstr, fdUDP, caddr, clen);
                        sent = 0;
                        for(i = 0; i < 3; i++){
                            if(ReadTimeout(fdUDP, 2) > 0){
                                ReadU(buffer, fdUDP, caddr, &clen);
                                sent = 1;
                                break;
                            }
                            else printf("Ainda não recebi...\n");
                        }
                        if(sent == 0)
                        {
                            printf("Não recebi um ACK, não sei se chegou... A enviar um FND para o sucessor\n");
                            strcpy(auxstr, buffer);
                            strcat(auxstr, "\n");
                            Write(buffer, auxstr, fdSuc);
                        }


                    }
                }
            }
            if(!strcmp(fst, "EFND"))      
            {
                sscanf(buffer, "%s %s", fst, scd);

                if((!(strcmp(IP, sIP) || strcmp(Port,sPort) || strcmp(IP, pIP) || strcmp(Port, pPort))) || 
                (((atoi(scd) >= key) && (atoi(scd) <= sKey)) || ((sKey < key) && ((atoi(scd) > key) || (atoi(scd) < sKey)))))
                {
                    sprintf(auxstr, "EPRED %d %s %s", key, IP, Port);
                    WriteU(buffer, auxstr, fdUDP, &addr, addrlen);
                    sent = 0;
                    for(i = 0; i < 3; i++){
                        if(ReadTimeout(fdUDP, 2) > 0){
                            ReadU(buffer, fdUDP, &addr, &addrlen);
                            sent = 1;
                            break;
                        }
                        else printf("Ainda não recebi...\n");
                    }
                    if(sent == 0)
                    {
                        printf("Não recebi um ACK, não sei se chegou...\n");
                    }
                }
                else{       
                    int a;
                    a = findMinFree(addresses, 99);
                    a++;
                    addresses[a-1] = &addr;
                    lens[a-1] = addrlen;
                    if((cKey == 0) || (((atoi(scd) >= key) && (atoi(scd) <= cKey)) || ((cKey < key) && ((atoi(scd) > key) || (atoi(scd) < cKey)))))
                    {
                        sprintf(auxstr, "FND %s %d %d %s %s\n", scd, a, key, IP, Port);
                        Write(buffer, auxstr, fdSuc);
                    }
                    else
                    {
                        sprintf(auxstr, "FND %s %d %d %s %s", scd, a, key, IP, Port);
                        WriteU(buffer, auxstr, fdUDP, caddr, clen);
                        sent = 0;
                        for(i = 0; i < 3; i++){
                            if(ReadTimeout(fdUDP, 2) > 0){
                                ReadU(buffer, fdUDP, caddr, &clen);
                                sent = 1;
                                break;
                            }
                            else printf("Ainda não recebi...\n");
                        }
                        if(sent == 0)
                        {
                            printf("Não recebi um ACK, não sei se chegou... A enviar um FND para o sucessor\n");
                            sprintf(auxstr, "FND %s %d %d %s %s\n", scd, a, key, IP, Port);
                            Write(buffer, auxstr, fdSuc);
                        }


                    }
                }

            }
            if(!strcmp(fst, "EPRED"))      
            {

                sscanf(buffer, "%s %s %s %s", fst, scd, thd, frt);
                int fdPAux;
                fdPAux = createtSocket();
                tcsetHints(&hints); 
                if(strcmp(thd,"NULL") == 0){
                    if ((errcode = getaddrinfo(NULL, frt, &hints, &res)) != 0) /*error*/
                    exit(1);
                }
                else if ((errcode = getaddrinfo(thd, frt, &hints, &res)) != 0) /*error*/
                    exit(1);

                n = connect(fdPAux, res->ai_addr, res->ai_addrlen);
                if (n == -1) /*error*/
                {
                    printf("Não consegui fazer connect\n");
                    exit(1);
                }

                memset(&act, 0, sizeof act);

                act.sa_handler = SIG_IGN;


                if (sigaction(SIGPIPE, &act, NULL) == -1) /*error*/
                    exit(1);

                str = createSelf(key, IP, Port);
                Write(buffer, str, fdPAux);
                if(fdPred != fdSuc) close(fdPred);
                fdPred = fdPAux;
                pKey = atoi(scd);
                strcpy(pIP, thd);
                strcpy(pPort, frt);

            }

            FD_CLR(fdUDP, &rfds);
        }

        if((x = FD_ISSET(fdTCP, &rfds)) != 0)
        {

            if ((newfd = accept(fdTCP, &addr, &addrlen)) == -1)
            {   
                printf("Não consegui fazer accept.\n");
                /*error*/ exit(1);                              
            }

            Read(buffer, newfd);

            sscanf(buffer, "%s %s %s %s", fst, scd, thd, frt);

            if(!strcmp(fst, "SELF")){
                if(strcmp(IP, sIP) || strcmp(Port,sPort) || strcmp(IP, pIP) || strcmp(Port, pPort)) // O anel tem mais do que um nó
                {
                    // Informar o sucessor desta entrada
                    if(!strcmp(sIP,"")){
                    }
                    else{
                        
                        str = createPred(atoi(scd), thd, frt); 
                        Write(buffer, str, fdSuc);
                        if(fdPred != fdSuc) close(fdSuc);
                    }
                    sKey = atoi(scd);
                    strcpy(sIP, thd);
                    strcpy(sPort, frt);
                    fdSuc = newfd;

                }
                else
                {                           // O anel só tem um nó e vai passar a ter dois
                    sKey = atoi(scd);
                    strcpy(sIP, thd);
                    strcpy(sPort, frt);
                    fdSuc = newfd;

                    pKey = sKey;
                    strcpy(pIP, sIP);
                    strcpy(pPort, sPort);
                    fdPred = newfd;
                    printf("fdPred: %d\n", fdPred);
                    // Informar o outro de que estava sozinho
                    char *self;

                    self = createSelf(key, IP, Port);

                    Write(buffer, self, fdPred);

                }

            }


        }
    }
    exit(0);
}