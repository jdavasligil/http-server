#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 443
#define MAXMSG 8192
#define ERROR -1
#define SUCCESS 0

#define IS_ERROR(v) ({ __typeof__ (v) _v = (v); _v < 0;})

int read_from_client(int file_descriptor) {
    char message_buffer[MAXMSG];
    const int nbytes = read(file_descriptor, message_buffer, MAXMSG);
    if (IS_ERROR(nbytes)) {
        perror("read");
        exit(EXIT_FAILURE);
    } else if (nbytes == 0) {
        return ERROR;
    } else {
        fprintf(stderr, "SERVER: Received message: `%s`\n", message_buffer);
        return SUCCESS;
    }
}

int make_socket(uint16_t port) {
    const int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (IS_ERROR(sock)) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    const struct sockaddr_in saddr = { 
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    }; 
    if (IS_ERROR(bind(sock, (struct sockaddr *) &saddr, sizeof(saddr)))) {
        perror("bind socket");
        exit(EXIT_FAILURE);
    }

    return sock;
}

//int main(int argc, char *argv[]) {
int main(void) {
    const int sock = make_socket(PORT);
    if (IS_ERROR(listen(sock, 1))) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    socklen_t size;
    int i;
    int new_sock;

    struct sockaddr_in client;

    fd_set active_fd_set;
    fd_set read_fd_set;

    FD_ZERO(&active_fd_set);
    FD_SET(sock, &active_fd_set);

    while (true) {
        read_fd_set = active_fd_set;
        if (IS_ERROR(select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL))) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(i, &read_fd_set)) {
                if (i == sock) {
                    size = sizeof(client);
                    new_sock = accept(sock, (struct sockaddr *) &client, &size);
                    if (IS_ERROR(new_sock)) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }

                    fprintf(stderr,
                            "SERVER: Connection from host `%s`, port %hd.\n",
                            inet_ntoa(client.sin_addr),
                            ntohs(client.sin_port));
                    FD_SET(new_sock, &active_fd_set);
                } else {
                    if (IS_ERROR(read_from_client(i))) {
                        close(i);
                        FD_CLR(i, &active_fd_set);
                    }
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
