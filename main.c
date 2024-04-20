#include "minecraft.h"
#include <arpa/inet.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#define N_STATUS_HANDLERS 2

pn_error_t (*status_handlers[N_STATUS_HANDLERS])(struct connection *) = {
	[0] = reply_to_status_request,
	[1] = reply_to_status_ping,
};

int main() {
	int rc;

	int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd < 0)
		err(EXIT_FAILURE, "socket");

	struct sockaddr_in6 bind_addr = (struct sockaddr_in6) {
		.sin6_family = AF_INET6,
		.sin6_port = htons(25565),
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
		int clientfd
			= accept(sockfd, (struct sockaddr *)&source_addr, &source_len);
		if (clientfd < 0)
			err(EXIT_FAILURE, "accept");

		FILE *client_socket = fdopen(clientfd, "r+");

		struct connection connection = {
			.socket_fd = clientfd,
			.socket = client_socket,
			.state = HANDSHAKE,
		};
		struct connection *c = &connection;

		for (int i = 0; i < 3; i++) {
			pn_error_t error;

			error = read_inbound_packet(c);
			if (error != pn_ok)
				goto close_connection;

			long type = read_varint(c);
			printf("packet type=%ld\n", type);

			switch (connection.state) {
			case HANDSHAKE:
				if (type == 0) {
					error = handle_handshake(&connection);
				} else {
					printf("unexpected packet type\n");
				}
				break;
			case STATUS:
				if (type < N_STATUS_HANDLERS && status_handlers[type]) {
					error = status_handlers[type](c);
				} else {
					printf("unexpected packet type\n");
				}
				break;
			case LOGIN:
			case PLAY:
				printf("unimplemented state\n");
				goto close_connection;
			default:
				printf("unexpected state\n");
				goto close_connection;
			}

			if (error != pn_ok)
				printf("error=%d\n", error);

			end_inbound_packet(c);
		}

	close_connection:
		fclose(client_socket);
	}
}
