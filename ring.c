#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>
#include <unistd.h>
#include <signal.h>

#include "ringaux.h"


// Todas necessárias? Alguma?
int key;
char IP[100];
char Port[100];

int sKey;
char sIP[100];
char sPort[100];

int pKey;
char pIP[100];
char pPort[100];

int fd;
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


void setToZero()
{
    printf("here in zero\n");
    //key = 0;
    //strcpy(IP, "");
    //strcpy(Port, "");

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

    //fd = 0;
    fdSuc = 0;
    fdPred = 0;
    fdUDP = 0;

    inARing = 0;
    printf("here in zero end\n");
}

void showInfo()
{
    printf("fd: %d\n", fd);
    printf("fdSuc: %d\n", fdSuc);
    printf("fdPred %d\n", fdPred);
    printf("Key: %d\n", key);
    printf("IP: %s\n", IP);
    printf("Port: %s\n", Port);    
    printf("sKey: %d\n", sKey);
    printf("sIP: %s\n", sIP);
    printf("sPort: %s\n", sPort);
    printf("pKey: %d\n", pKey);
    printf("pIP: %s\n", pIP);
    printf("pPort: %s\n", pPort);
}

int createRing()
{   

    // É SUPOSTO HAVER UMA VERIFICAÇÃO DE SE O i.IP É EQUIVALENTE AO IP DO NÓ QUE CRIA A APLICAÇÃO?

   if(inARing)
   {
       // Avisar que vai sair do anel atual, talvez fazer um scanf para pedir confirmaçao?
   }

    sKey = key;
    strcpy(sIP, IP);
    strcpy(sPort, Port);

    pKey = key;
    strcpy(pIP, IP);
    strcpy(pPort, Port);

    printf("Anel criado com sucesso. Sucessor deste nó é o próprio.\n");


    return 0;
}

int pEntry(int predkey, char *predIP, char *predPort){

    struct addrinfo hints, *res;
    int fd1, newfd, errcode;

    ssize_t n, nw;
    struct sockaddr addr;
    socklen_t addrlen;

    struct sigaction act;

    ssize_t nbytes, nleft, nwritten, nread;
    char *ptr, buffer[128 + 1];

    // "Modo cliente"   (Precisamos de avisar o nosso novo predecessor que somos o seu novo sucessor)

    fd1 = createtSocket(); // if error, exits

    setHints(&hints);
    //hints.ai_flags = AI_PASSIVE;

    if(!strcmp(predIP, "NULL")){
        if ((errcode = getaddrinfo(NULL, predPort, &hints, &res)) != 0){ /*error*/
            printf("Não consegui receber addrinfo.\n");
            exit(1);
        }
    }
    else if ((errcode = getaddrinfo(predIP, predPort, &hints, &res)) != 0){ /*error*/
        printf("Não consegui receber addrinfo.\n");
        exit(1);
    }

    n = connect(fd1, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/
        exit(1);

    memset(&act, 0, sizeof act);

    act.sa_handler = SIG_IGN;


    if (sigaction(SIGPIPE, &act, NULL) == -1) /*error*/
        exit(1);

    char *str;
    str = createSelf(key, IP, Port);

    Write(buffer, str, fd1);


    fdPred = fd1;
    if(fdPred > maxfd) maxfd = fdPred;
    pKey = predkey;
    strcpy(pIP, predIP);
    strcpy(pPort, predPort);

    return 0;

}


int createServer(struct addrinfo *hints, struct addrinfo **res){     //??
    int errcode;
    
    fd = createtSocket(); // if error, exits
    maxfd = fd;
    printf("fd: %d\n", fd);
    fsetHints(hints);

    if(!strcmp(IP, "NULL"))
    {
        printf("Im here in create server TCP NULL case\n");

        if ((errcode = getaddrinfo(NULL, Port, hints, res)) != 0){ /*error*/
            printf("Não consegui receber addrinfo.\n");
            exit(1);
        }
    }
    else if ((errcode = getaddrinfo(IP, Port, hints, res)) != 0){ /*error*/
        printf("Não consegui receber addrinfo.\n");
        exit(1);
    }
    //printf("%s\n", res->ai_addr->sa_data);

    if (bind(fd, (*res)->ai_addr, (*res)->ai_addrlen) == -1){ /*error*/
        printf("Não consegui fazer bind.\n");
        exit(1);
    }
    if (listen(fd, 5) == -1){ /*error*/
        printf("Não consegui fazer listen.\n");
        exit(1);
    }

    fdUDP = createuSocket();
    if(fdUDP > maxfd) maxfd = fdUDP;
    printf("%d\n", fdUDP);
    usetHints(hints);

    if(!strcmp(IP, "NULL"))
    {
        printf("Im here in create server UDP NULL case\n");
        if ((errcode = getaddrinfo(NULL, Port, hints, res)) != 0)
        { /*error*/
            printf("Não consegui receber addrinfo.\n");
            exit(1);
        }
    }
    else if ((errcode = getaddrinfo(IP, Port, hints, res)) != 0){ /*error*/
            printf("Não consegui receber addrinfo.\n");
            exit(1);
    } 
    
    if(bind(fdUDP,(*res)->ai_addr, (*res)->ai_addrlen)==-1)/*error*/exit(1);

    

    return fd;
}


void leave()
{
    //if(!inARing) return;
    if(fdSuc != 0) close(fdSuc);
    if(fdPred != 0) close(fdPred);
    setToZero();
}

void exitProgram()
{
    leave();
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
    int efindk;
    int auxFDUDP;
    int errcode;

    while (cont)
    {
        if(isThereChar)
        {
            //fgets(params, 100, stdin);
            //scanf("%s", params);
            input = fgetc(stdin);
            printf("%c", input);
        }
        else{
            printUI();
            input = fgetc(stdin);
        }
        switch (input)
        {
        case 'n':
            //printf("Escolheu a opção 'n'.\n");
            //printf("%s %s.\n", argv[2], argv[3]);

            if(!createRing()){           
                inARing = 1;
                fgetc(stdin);       // Para apanhar o enter
                return;
            }
            else{
                printf("Erro CreateRing");
                exit(0);
            }
            break;

        case 'p':
            fgets(params, 100, stdin);
            sscanf(params, "%d %s %s", &auxkey, auxip, auxport);
            printf("predkey: %d\n", auxkey);
            printf("predIP: %s\n", auxip);
            printf("predport: %s\n", auxport);
            
            pEntry(auxkey, auxip, auxport);

            inARing = 1;
            return;
            break;      //tirar

        case 'x':
            if((strcmp(sPort, "") != 0 ) && (strcmp(sPort, Port) != 0))
            {
                str = createPred(pKey, pIP, pPort);
                Write(buffer, str, fdSuc); 
            } 
            exitProgram(); // Gestão de memória
        case 'l':
            printf("hereInL\n");
            if((strcmp(sPort, "") != 0 ) && (strcmp(sPort, Port) != 0))
            {
                str = createPred(pKey, pIP, pPort);
                Write(buffer, str, fdSuc); 
            } 
            leave();
            fgetc(stdin);
            return;
            break;      //tirar
        case 'f':
            fgets(params, 100, stdin);
            int findk;
            sscanf(params, "%d", &findk);
            printf("findk: %d\n", findk);
            sprintf(params, "%s %d %d %d %s %s\n", "FND", findk, 1, key, IP, Port);
            Write(buffer, params, fdSuc);
            return;
            break;
        case 'q':
            fgets(params, 100, stdin);
            sscanf(params, "%d %s %s", &efindk, auxIP, auxPort);
            printf("efindk: %d %s %s\n", efindk, auxIP, auxPort);
            sprintf(auxstr, "EFND %d", efindk);
            printf("%s\n", auxstr);
            //auxFDUDP = createuSocket();
            ucsetHints(&hints);

            if(!strcmp(auxIP, "NULL"))
            {
                printf("Im here in create server UDP NULL case\n");
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
            return;
            break;
        case 'b':
            fgets(params, 100, stdin);
            sscanf(params, "%d %s %s", &auxkey, auxIP, auxPort);
            printf("sending to: %d %s %s\n", auxkey, auxIP, auxPort);
            sprintf(auxstr, "EFND %d", key);
            printf("%s\n", auxstr);
            //auxFDUDP = createuSocket();
            ucsetHints(&hints);

            if(!strcmp(auxIP, "NULL"))
            {
                printf("Im here in create server UDP NULL case\n");
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
            return;
            break;
        case 'c':
            fgets(params, 100, stdin);
            sscanf(params, "%d %s %s", &auxkey, auxip, auxport);
            printf("chordkey: %d\n", auxkey);
            printf("chordIP: %s\n", auxip);
            printf("chordport: %s\n", auxport);
            cKey = auxkey;
            strcpy(cIP, auxip);
            strcpy(cPort, auxPort);

            ucsetHints(&hints);

            if(!strcmp(auxip, "NULL"))
            {
                printf("Im here in create server UDP NULL case\n");
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
            break;
        case 'e':
            cKey = 0;
            strcpy(cIP, "");
            strcpy(cPort, "");
            caddr = NULL;
            clen = 0;
            return;
            break;
        default:
            printf("Não é um input válido.");
            break;
        }
    }
}

int main(int argc, char **argv)
{
    setToZero();

    // Declarações
    char input;
    int inARing;
    int cont = 1;       // inutil?
    int numFds = 0;     // inutil?
    maxfd = 0;
    // char finds[2][100][20];
    struct sockaddr *addresses[99];
    socklen_t lens[99];
    //int nCameFromUser[100];
    currentN = 0;
    int i;
    //fndCameFromUser = 1;
    char auxstr[100];

    
    for(i = 0; i < 99; i++){
        addresses[i] = NULL;
        lens[i] = 0;
    }
    

    fd_set rfds;

    // Outras mais gerais e muitas inutilizadas

    struct addrinfo hints, *res;
    int newfd, errcode;

    ssize_t n, nw;
    struct sockaddr addr;
    socklen_t addrlen;

    struct sigaction act;

    ssize_t nbytes, nleft, nwritten, nread;
    char *ptr, buffer[128 + 1];

    //char params[100], predip[100], predport[100];

    int nready;
    int auxfd;
    int x;
    char fst[20], scd[20], thd[20], frt[20], fft[20], sxt[20];
    char *str;
    struct timeval t;

    char *userInput;
    userInput = (char *) malloc(100);

    if ((argc < 4) || (argc > 4))
    {
        printf("Not enough arguments\n");
        exit(EXIT_FAILURE);
    }

    key = atoi(argv[1]); // Vai ser preciso verificar de alguma forma se foi inserido um int ou não.
                         // Clássica verificação de chars que representam números? ( '0' <= char <= '9')
    strcpy(IP, argv[2]);
    strcpy(Port, argv[3]);


    createServer(&hints, &res); // Provavelmente devia ser feito logo logo no principio

    inARing = 0;

    checkInput(0);
    int alreadyRead = 0;
    

    // Saímos numa situação em que vamos passar só a modo servidor. Por isso, vamos fazer o TCP com accept para vários fds(?)

    // Lembrar-me das ligações abertas e não fechá-las no fim, e por isso comentar o close. 


    FD_ZERO(&rfds);    FD_SET(0, &rfds);   FD_SET(fd, &rfds);   FD_SET(fdPred, &rfds);
    // Possivelmente outros fdset se vir que é possivel chegar aqui com mais cenas feitas


    while (1)
    {   
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);   FD_SET(fdSuc, &rfds);   FD_SET(fdPred, &rfds);   FD_SET(fdUDP, &rfds);  FD_SET(fd, &rfds);  

        printf("No while.\n");

        showInfo();
        nready = select(maxfd + 1, &rfds, NULL, NULL, NULL);   // Ter a certeza que sempre que temos um novo fd verificamos se >max!!

        alreadyRead = 0;
        addrlen = sizeof(addr);   
        if((x = FD_ISSET(0, &rfds)) != 0)
        {
            printf("here1\n");

            checkInput(1);
            FD_CLR(0, &rfds);
        }      

        if ((x = FD_ISSET(fdSuc, &rfds)) != 0)
        {
            printf("here2\n");

            if(Read(buffer, fdSuc) == 1){   // Nao pode ser bem isto porque no caso do nó estar sozinho ele tem de voltar a ligar a si!
                                            // Já está corrigida esta questão de cima?
                if(fdSuc != fdPred)
                {
                    sKey = 0;                   // Falta o pred!
                    strcpy(sIP, "");
                    strcpy(sPort, "");
                    printf("Im here somehow\n");
                    FD_CLR(fdSuc, &rfds);       // Inutil??
                }
                else
                {
                    sKey = key;                   // Falta o pred!
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
            printf("here3\n");

            nleft = 100;
            ptr = buffer;
            if(alreadyRead == 1)
            {

            }
            else
            {
                if(Read(buffer, fdPred) == 1){
                    //str = createPred(pKey, pIP, pPort); // Verificar se a info tem formato válido!!
                    //Write(buffer, str, fdSuc);
                    close(fdPred);
                    pKey = 0;
                    strcpy(pIP, "");
                    strcpy(pPort, "");
                    fdPred = 0;
                    FD_CLR(fdPred, &rfds);
                    continue;
                }
            }


            printf("%s\n", buffer);
            sscanf(buffer, "%s", fst);
            //printf("%s, %s, %s, %s\n", fst, scd, thd, frt);

            if(!strcmp(fst, "SELF"))        // Para o caso em que ainda temos só 2 nós e o outro quer informar-nos disso
            {
                sscanf(buffer, "%s %s %s %s", fst, scd, thd, frt);
                sKey = atoi(scd);
                strcpy(sIP, thd);
                strcpy(sPort, frt);
                fdSuc = fdPred;
                if(fdSuc > maxfd) maxfd = fdSuc; // Não faz sentido
            }
            if(!strcmp(fst, "PRED"))        
            {
                // Verificar formatos!!!
                //str = createSelf(key, IP, Port);
                sscanf(buffer, "%s %s %s %s", fst, scd, thd, frt);
                fdPred = createtSocket();
                setHints(&hints); // Sem flags?...
                if(strcmp(thd,"NULL") == 0){
                    // fazer só o if seguinte aqui dentro com um NULL em vez de mudar a variavel? tipo como está agora aqui
                    if ((errcode = getaddrinfo(NULL, frt, &hints, &res)) != 0) /*error*/
                    exit(1);
                }
                else if ((errcode = getaddrinfo(thd, frt, &hints, &res)) != 0) /*error*/
                    exit(1);

                n = connect(fdPred, res->ai_addr, res->ai_addrlen);
                if (n == -1) /*error*/
                {
                    printf("Couldn't connect\n");
                    exit(1);
                }

                memset(&act, 0, sizeof act);

                act.sa_handler = SIG_IGN;


                if (sigaction(SIGPIPE, &act, NULL) == -1) /*error*/
                    exit(1);

                str = createSelf(key, IP, Port);
                Write(buffer, str, fdPred);
                pKey = atoi(scd);
                strcpy(pIP, thd);
                strcpy(pPort, frt);

                if(fdPred > maxfd) maxfd = fdPred; // Não faz sentido
            }
            if(!strcmp(fst, "FND"))        
            {
                sscanf(buffer, "%s %s %s %s %s %s", fst, scd, thd, frt, fft, sxt);
                if(((atoi(scd) >= key) && (atoi(scd) <= sKey)) || (sKey < key) && ((atoi(scd) > key) || (atoi(scd) < sKey)))    // É capaz de ficar mais simples com os controlos dos 32...
                {
                    printf("It's mine!\n");
                    strcpy(fst, "RSP");
                    strcpy(scd, frt);
                    sprintf(auxstr, "%s %s %s %d %s %s\n", fst, scd, thd, key, IP, Port);
                    Write(buffer, auxstr, fdSuc);
                }
                else{

                    if((cKey == 0) || (((atoi(scd) >= key) && (atoi(scd) <= cKey)) || ((cKey < key) && ((atoi(scd) > key) || (atoi(scd) < cKey)))))
                    {
                        strcpy(auxstr, buffer);
                        Write(buffer, auxstr, fdSuc);
                    }
                    else
                    {
                        strcpy(auxstr, buffer);
                        WriteU(buffer, auxstr, fdUDP, caddr, clen);
                    }
                    
                }

            }
            else if(!strcmp(fst, "RSP"))
            {
                sscanf(buffer, "%s %s %s %s %s %s", fst, scd, thd, frt, fft, sxt);
                if(atoi(scd) == key)
                {
                    if(strcmp(thd, "1") == 0){
                        printf("It belongs to: %s, %s, %s\n", frt, fft, sxt);       
                    }
                    else{
                        int n;
                        n = atoi(thd);
                        strcpy(fst, "EPRED");
                        strcpy(scd, frt);
                        strcpy(thd, fft);
                        strcpy(frt, sxt);
                        sprintf(auxstr, "%s %s %s %s", fst, scd, thd, frt);
                        printf("%s\n", auxstr);
                        WriteU(buffer, auxstr, fdUDP, addresses[n-1], lens[n-1]);
                        addresses[n-1] = NULL;
                        lens[n-1] = 0;
                    }
                }
                else{
                    strcpy(auxstr, buffer);
                    Write(buffer, auxstr, fdSuc);
                }
            }


            FD_CLR(fdPred, &rfds);
        }

        if ((x = FD_ISSET(fdUDP, &rfds)) != 0)
        {
            printf("here4\n");

            nleft = 100;
            ptr = buffer;

            ReadU(buffer, fdUDP, &addr, &addrlen);

            printf("%s\n", buffer);
            sscanf(buffer, "%s", fst);

            if(!strcmp(fst, "FND"))      // É para tratar disto, que está um cocó!!  
            {
                sscanf(buffer, "%s %s %s %s %s %s", fst, scd, thd, frt, fft, sxt);
                if(((atoi(scd) >= key) && (atoi(scd) <= sKey)) || (sKey < key) && ((atoi(scd) > key) || (atoi(scd) < sKey)))
                {
                    printf("It's mine!\n");
                    strcpy(fst, "RSP");
                    strcpy(scd, frt);
                    sprintf(auxstr, "%s %s %s %d %s %s\n", fst, scd, thd, key, IP, Port);
                    Write(buffer, auxstr, fdSuc);
                }
                else{
                    strcpy(auxstr, buffer);
                    Write(buffer, auxstr, fdSuc);
                }

            }
            else if(!strcmp(fst, "RSP"))
            {
                sscanf(buffer, "%s %s %s %s %s %s", fst, scd, thd, frt, fft, sxt);
                if(atoi(scd) == key)
                {
                    if(strcmp(thd, "1") == 0){
                        printf("It belongs to: %s, %s, %s\n", frt, fft, sxt);       
                    }
                    else{
                        int n;
                        n = atoi(thd);
                        strcpy(fst, "EPRED");
                        strcpy(scd, frt);
                        strcpy(thd, fft);
                        strcpy(frt, sxt);
                        sprintf(auxstr, "%s %s %s %s", fst, scd, thd, frt);
                        WriteU(buffer, auxstr, fdUDP, addresses[n-1], lens[n-1]); 
                        addresses[n-1] = NULL;
                        lens[n-1] = 0;
                        //WriteU(buffer, str, finds[0][n], finds[1][n]);
                    }
                }
                else{
                    strcpy(auxstr, buffer);
                    Write(buffer, auxstr, fdSuc);
                }
            }
            if(!strcmp(fst, "EFND"))      // É para tratar disto, que está um cocó!!  
            {
                sscanf(buffer, "%s %s", fst, scd);
                if(((atoi(scd) >= key) && (atoi(scd) <= sKey)) || (sKey < key) && ((atoi(scd) > key) || (atoi(scd) < sKey)))
                {
                    printf("It's mine!\n");
                    strcpy(fst, "EPRED");
                    sprintf(auxstr, "%s %d %s %s", fst, key, IP, Port);
                    WriteU(buffer, auxstr, fdUDP, &addr, addrlen);
                }
                else{       // FALTA MUDAR ISTO PARA FAZER A MESMA VEFIFICAÇÃO DE ATALHO QUE JA ESTÁ FEITA NOUTRO SITIO
                    int a;
                    a = findMinFree(addresses, 99);
                    a++;
                    sprintf(auxstr, "FND %s %d %d %s %s\n", scd, a, key, IP, Port);
                    addresses[a-1] = &addr;
                    lens[a-1] = addrlen;
                    Write(buffer, auxstr, fdSuc);
                }

            }
            if(!strcmp(fst, "EPRED"))      // É para tratar disto, que está um cocó!!  
            {
                // Verificar formatos!!!
                //str = createSelf(key, IP, Port);
                sscanf(buffer, "%s %s %s %s", fst, scd, thd, frt);
                fdPred = createtSocket();
                setHints(&hints); // Sem flags?...
                if(strcmp(thd,"NULL") == 0){
                    // fazer só o if seguinte aqui dentro com um NULL em vez de mudar a variavel? tipo como está agora aqui
                    if ((errcode = getaddrinfo(NULL, frt, &hints, &res)) != 0) /*error*/
                    exit(1);
                }
                else if ((errcode = getaddrinfo(thd, frt, &hints, &res)) != 0) /*error*/
                    exit(1);

                n = connect(fdPred, res->ai_addr, res->ai_addrlen);
                if (n == -1) /*error*/
                {
                    printf("Couldn't connect\n");
                    exit(1);
                }

                memset(&act, 0, sizeof act);

                act.sa_handler = SIG_IGN;


                if (sigaction(SIGPIPE, &act, NULL) == -1) /*error*/
                    exit(1);

                str = createSelf(key, IP, Port);
                Write(buffer, str, fdPred);
                pKey = atoi(scd);
                strcpy(pIP, thd);
                strcpy(pPort, frt);

                if(fdPred > maxfd) maxfd = fdPred; // Não faz sentido
            }

            FD_CLR(fdUDP, &rfds);
        }

        if((x = FD_ISSET(fd, &rfds)) != 0)
        {
            printf("here\n");

            if ((newfd = accept(fd, &addr, &addrlen)) == -1)
            {   
                printf("Não consegui fazer accept.\n");
                /*error*/ exit(1);                              
            }
            printf("accepted\n");

            Read(buffer, newfd);

            printf("%s\n", buffer);
            sscanf(buffer, "%s %s %s %s", fst, scd, thd, frt);
            printf("%s, %s, %s, %s\n", fst, scd, thd, frt);

            if(!strcmp(fst, "SELF")){
                if(strcmp(IP, sIP) || strcmp(Port,sPort) || strcmp(IP, pIP) || strcmp(Port, pPort)) //Então aqui já nao evito usar == com strings?...
                {
                    printf("Not a 1-member ring\n");
                    // Informar o sucessor desta entrada
                    if(!strcmp(sIP,"")){

                    }
                    else{
                        
                        str = createPred(atoi(scd), thd, frt); // Verificar se a info tem formato válido!!
                        Write(buffer, str, fdSuc);
                        if(fdPred != fdSuc) close(fdSuc);
                    }
                    sKey = atoi(scd);
                    strcpy(sIP, thd);
                    strcpy(sPort, frt);
                    fdSuc = newfd;
                    if(fdSuc > maxfd) maxfd = fdSuc;

                }
                else
                {
                    sKey = atoi(scd);
                    strcpy(sIP, thd);
                    strcpy(sPort, frt);
                    fdSuc = newfd;

                    printf("1-member ring, going on 2\n");
                    pKey = sKey;
                    strcpy(pIP, sIP);
                    strcpy(pPort, sPort);
                    fdPred = newfd;
                    if(fdPred > maxfd) maxfd = fdSuc; // Não faz sentido
                    printf("fdPred: %d\n", fdPred);
                    // Informar o outro de que estava sozinho
                    char *self;
                    char *ptr2;

                    self = createSelf(key, IP, Port);

                    Write(buffer, self, fdPred);

                }

            }

            //write(newfd, str, strlen(str));

        }
        printf("Fim do while.\n");
        //close(newfd);
    }
    printf("Bazei aqui...\n");

    exit(0);
}