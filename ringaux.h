#ifndef H_RINGAUX
#define H_RINGAUX

void setHints(struct addrinfo *hints);
void fsetHints(struct addrinfo *hints);
void printUI();
int createSocket();
char *createSelf(int key, char *IP, char *Port);
char *createPred(int pkey, char *pIP, char *pPort);
int Read(char *buffer, int fd);
void Write(char *buffer, char *str, int fd1);

#endif