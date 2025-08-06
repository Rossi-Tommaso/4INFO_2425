#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFSIZE 1024

void error(char *msg) 
{
    perror(msg);
    exit(1);
}

int socket_create() 
{
    int socket_fd;
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        error("Errore nella creazione del socket");
    }
    return socket_fd;
}

void socket_bind(int socket_fd, unsigned short udp_port) 
{
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(udp_port);

    if (bind(socket_fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        error("Errore nella fase di binding");
    }
}

int socket_receive(int socket_fd, char *buf, struct sockaddr_in *clientaddr)
{
    int msg_size;
    socklen_t client_struct_len = sizeof(*clientaddr);

    bzero(buf, BUFSIZE);
    if ((msg_size = recvfrom(socket_fd, buf, BUFSIZE, 0,
                             (struct sockaddr *)clientaddr, &client_struct_len)) < 0) {
        error("Errore nella ricezione dati");
    }
    return msg_size;
}

int socket_send(int socket_fd, struct sockaddr_in *clientaddr, char *buf)
{
    int byte_sent;
    socklen_t len = sizeof(*clientaddr);

    if ((byte_sent = sendto(socket_fd, buf, strlen(buf), 0,
                            (struct sockaddr *)clientaddr, len)) < 0) {
        error("Errore nell'invio dati");
    }

    return byte_sent;
}

int main(int argc, char **argv) 
{
    unsigned short udp_port;
    int socket_fd;
    char buf[BUFSIZE];
    int msg_size;
    struct sockaddr_in clientaddr;
    int byte_sent;

    if (argc != 2) {
        printf("Uso: %s <porta>\n", argv[0]);
        exit(1);
    }

    udp_port = (unsigned short)atoi(argv[1]);

    socket_fd = socket_create();
    socket_bind(socket_fd, udp_port);

    printf("Server UDP pronto e in ascolto sulla porta %d\n\n", udp_port);

    for (;;) {
        msg_size = socket_receive(socket_fd, buf, &clientaddr);
        printf("Ricevuti %d byte dal client %s:%d: %s\n",
               msg_size,
               inet_ntoa(clientaddr.sin_addr),
               ntohs(clientaddr.sin_port),
               buf);

        byte_sent = socket_send(socket_fd, &clientaddr, buf);
        printf("Inviati %d byte al client\n\n", byte_sent);
    }

    close(socket_fd);
    return 0;
}
