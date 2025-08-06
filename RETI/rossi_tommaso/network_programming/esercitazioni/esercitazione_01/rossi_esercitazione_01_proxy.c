#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>

/* 
man 7 ip
man 7 tcp
 */

#define BUFSIZE 1024
#define MAX_CONN 10
#define UDP_DEST_IP "127.0.0.1"
#define UDP_DEST_PORT 8001


/* man perror */
void error(char *msg) 
{
  perror(msg);
  exit(1);
}

int socket_create_tcp() 
{
    int socket_fd;

    if((socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        error("Cannot create the socket");
        exit(1);
    }

    return socket_fd;
}

int socket_create_udp() 
{
    int socket_fd;

    if((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        error("Errore nella creazione del socket");
        exit(1);
    }
    return socket_fd;
}

void socket_bind(int socket_fd, unsigned short tcp_port) 
{
    struct sockaddr_in serveraddr; /* server address */
  
    /* inizializza la struttura che contiene le informazioni del socket */
    memset(&serveraddr, '0', sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; /* socket IP */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 0.0.0.0 */
    serveraddr.sin_port = htons(tcp_port); /* porta tcp in network order*/

    /* bind del socket con indirizzo e porta */
    if(bind(socket_fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
        error("Errore nella fase di binding");
}

void socket_listen(int socket_fd) 
{
    if(listen(socket_fd, MAX_CONN) < 0) /* in ascolto sul socket max 10 connessioni*/ 
        error("Errore nella fase di listen");
}

int socket_accept(int socket_fd) 
{
    int connection_fd;
    struct sockaddr_in clientaddr; /* client address */
    socklen_t clientlen = sizeof(clientaddr);
    
    /* accetta una connessione TCP da un client */
    if((connection_fd = accept(socket_fd, 
                            (struct sockaddr *) &clientaddr, 
                            &clientlen)) < 0)
        error("Errore nella fase di accept");
    return connection_fd;
}

int socket_receive(int socket_fd, char *buf)
{
    int msg_size;

    bzero(buf, BUFSIZE);
    if((msg_size = read(socket_fd, buf, BUFSIZE)) < 0)
        error("Errore nella ricezione dati");
    
    return msg_size;
}

int socket_send(int socket_fd, char *ip, unsigned short port, char *buf, int msg_size) 
{
    struct sockaddr_in serveraddr; /* indirizzo e porta server */  
    int byte_sent;

     /* prepara le informazioni sulla destinazioen del datagram */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(ip);

   if((byte_sent = sendto(socket_fd, buf, msg_size, 0,
         (struct sockaddr*)&serveraddr, sizeof(serveraddr))) < 0)
         error("Errore nell'invio dati");
    
    return byte_sent;
}

int main(int argc, char **argv) 
{
    unsigned short tcp_port; /* TCP port in ascolto */
    int socket_fd_tcp;       /* welcoming socket file descriptor */
    int socket_fd_udp;
    int connection_fd;       /* connection socket file descriptor */
    char buf[BUFSIZE];       /* RX buffer */
    int msg_size;            /* dimensione messaggio ricevuto */
    int byte_sent;

    /* Verifico la presenza del parametro porta e lo leggo*/ 
    if(argc != 2) {
        printf("uso: %s <porta>\n", argv[0]);
        exit(1);
    }
    tcp_port = (unsigned short)atoi(argv[1]);

    /* Creo i socket */ 
    socket_fd_tcp = socket_create_tcp();
    socket_fd_udp = socket_create_udp();

    /* bind del socket a IP e porta */
    socket_bind(socket_fd_tcp, tcp_port);

    /* metto il socket in ascolto */
    socket_listen(socket_fd_tcp); 
    
    /* ciclo principale del server */
    printf("Proxy TCP/UDP pronto e in ascolto sulla porta %d\n\n", tcp_port);
    for(;;) {
        /* rimango in attesa fino a quando arriva una richiesta da un client */
        connection_fd = socket_accept(socket_fd_tcp);

        msg_size = socket_receive(connection_fd, buf);
        printf("TCP/UDP proxy ha ricevuto %d byte: %s\n", msg_size, buf);

        /* converto in maiuscolo */
        for(int i = 0; i < strlen(buf); i++) {
            buf[i] = toupper((unsigned char)buf[i]);
        }

        /* invio dei dati al server udp*/
        byte_sent = socket_send(socket_fd_udp, UDP_DEST_IP, UDP_DEST_PORT, buf, msg_size); 

        /* chiudo la connessione con il client */
        close(connection_fd);
    }
    close(socket_fd_udp);
}