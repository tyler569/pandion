#include "minecraft.h"
#include <arpa/inet.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

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

		handle_client_connection(clientfd);
	}
}
