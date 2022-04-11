#ifndef H_RINGAUX
#define H_RINGAUX

void setHints(struct addrinfo *hints);
void fsetHints(struct addrinfo *hints);
void ucsetHints(struct addrinfo *hints);
void usetHints(struct addrinfo *hints);
void printUI();
int createtSocket();
int createuSocket();
char *createSelf(int key, char *IP, char *Port);
char *createPred(int pkey, char *pIP, char *pPort);
int Read(char *buffer, int fd);
int ReadU(char *buffer, int fdUDP, struct sockaddr *addr, socklen_t *addrlen);
void Write(char *buffer, char *str, int fd1);
int WriteU(char *buffer, char *str, int fdUDP, struct sockaddr *addrinfo, socklen_t addrlen);
int findMinFree(struct sockaddr**senderArray, int arraySize);

#endif