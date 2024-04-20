#include <arpa/inet.h>
#include <err.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "minecraft.h"

int main() {
    int rc;
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0)
        err(EXIT_FAILURE, "socket");

    struct sockaddr_in6 bind_addr = (struct sockaddr_in6) {
        .sin6_family = AF_INET6,
        .sin6_port = htons(25566),
        .sin6_addr = in6addr_any,
    };

    rc = bind(sockfd, (struct sockaddr *)&bind_addr, sizeof(bind_addr));
    if (rc < 0)
        err(EXIT_FAILURE, "bind");

    rc = listen(sockfd, 5);
    if (rc < 0)
        err(EXIT_FAILURE, "listen");

    struct sockaddr_in6 source_addr;
    socklen_t source_len = sizeof(source_addr);

    while (true) {
        int clientfd = accept(sockfd, (struct sockaddr *)&source_addr, &source_len);
        if (clientfd < 0)
            err(EXIT_FAILURE, "accept");

        FILE *client_socket = fdopen(clientfd, "r+");

        struct connection connection = {
            .socket = client_socket,
            .state = HANDSHAKE,
        };

        for (int i = 0; i < 3; i++) {
            struct packet *packet = read_packet(client_socket);
            if (!packet) {
                printf("EOF\n");
                break;
            }

            long type = read_varint(packet);
            printf("packet type=%ld len=%zd\n", type, packet->len);

            if (connection.state == HANDSHAKE) {
                if (type == 0) {
                    handle_handshake(&connection, packet);
                } else {
                    printf("unexpected packet type\n");
                }
            } else if (connection.state == STATUS) {
                if (type == 0) {
                    reply_to_status_request(&connection, packet);
                } else if (type == 1) {
                    reply_to_status_ping(&connection, packet);
                } else {
                    printf("unexpected packet type\n");
                }
            } else {
                printf("unexpected state\n");
                break;
            }

            free_packet(packet);
        }

        fclose(client_socket);
    }
}
