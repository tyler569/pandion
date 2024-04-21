#include "minecraft.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void handle_client_connection(struct server *server, int socket_fd) {
	struct connection connection = {
		.server = server,
		.socket_fd = socket_fd,
		.inbound_stream = BIO_new_fd(socket_fd, BIO_NOCLOSE),
		.outbound_stream = BIO_new_fd(socket_fd, BIO_NOCLOSE),
		.state = HANDSHAKE,
	};
	struct connection *c = &connection;

	while (true) {
		pn_error_t error;

		error = read_inbound_packet(c);
		if (error != pn_ok)
			goto close_connection;

		switch (connection.state) {
		case HANDSHAKE:
			error = handle_handshake(c);
			break;
		case STATUS:
			error = handle_status_state(c);
			break;
		case LOGIN:
			error = handle_login_state(c);
			break;
		case PLAY:
			error = handle_play_state(c);
			break;
		default:
			printf("unexpected state\n");
			error = pn_invalid_state;
		}

		end_inbound_packet(c);

		if (error == pn_unhandled_packet) {
			printf("unhandled packet\n");
		} else if (error != pn_ok) {
			goto close_connection;
		}
	}

close_connection:
	BIO_free_all(c->inbound_stream);
	BIO_free_all(c->outbound_stream);
}

pn_error_t read_inbound_packet_bio(struct connection *c) {
	long len = read_varint_from_bio(c->inbound_stream);

	if (len == 0)
		return pn_invalid_packet;

	if (len > 0) {
		c->inbound_packet.len = len;
		void *data = malloc(len);

		if (!data)
			return pn_oom;

		c->inbound_packet.data = data;

		BIO_read(c->inbound_stream, c->inbound_packet.data, (int)len);

		c->inbound_packet.stream = fmemopen(c->inbound_packet.data, len, "r");
	}

	return pn_ok;
}

pn_error_t read_inbound_packet(struct connection *c) {
	return read_inbound_packet_bio(c);
}

void end_inbound_packet(struct connection *c) {
	fclose(c->inbound_packet.stream);
}

void new_outbound_packet(struct connection *c, long id) {
	assert(c->outbound_packet.data == nullptr);

	c->outbound_packet.len = 0;
	c->outbound_packet.stream = open_memstream(
		(char **)&c->outbound_packet.data, &c->outbound_packet.len);

	write_varint_to_stream(c->outbound_packet.stream, (long)id);
}

void send_outbound_packet_bio(struct connection *c) {
	write_varint_to_bio(c->outbound_stream, (long)c->outbound_packet.len);
	BIO_write(c->outbound_stream, c->outbound_packet.data,
		(int)c->outbound_packet.len);
}

pn_error_t send_outbound_packet(struct connection *c) {
	fclose(c->outbound_packet.stream);

	assert(c->outbound_packet.data != nullptr);
	assert(c->outbound_packet.len > 0);
	assert(!feof(c->outbound_packet.stream));
	assert(!ferror(c->outbound_packet.stream));

	send_outbound_packet_bio(c);

	free(c->outbound_packet.data);
	c->outbound_packet.data = nullptr;

	if (BIO_eof(c->outbound_stream))
		return pn_eof;

	return pn_ok;
}

void flush_connection_socket(struct connection *c) {
	BIO_flush(c->outbound_stream);
}
