#include "minecraft.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void handle_client_connection(struct server *server, int socket_fd) {
	FILE *client_socket = fdopen(socket_fd, "r+");

	struct connection connection = {
		.server = server,
		.socket_fd = socket_fd,
		.socket = client_socket,
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
			printf("unimplemented state\n");
			error = pn_unhandled_packet;
			break;
		default:
			printf("unexpected state\n");
			error = pn_invalid_state;
		}

		end_inbound_packet(c);

		if (error == pn_unhandled_packet) {
			printf("unhandled packet\n");
			break;
		} else if (error != pn_ok) {
			goto close_connection;
		}
	}

close_connection:
	if (c->encryption_enabled)
		free_connection_aes(c);

	fclose(client_socket);
}

pn_error_t read_inbound_packet_socket(struct connection *c) {
	long len = read_varint_from_stream(c->socket);

	if (feof(c->socket) || ferror(c->socket))
		return pn_eof;

	if (len == 0)
		return pn_invalid_packet;

	if (len > 0) {
		c->inbound_packet.len = len;
		void *data = realloc(c->inbound_packet.data, len);

		if (!data)
			return pn_oom;

		c->inbound_packet.data = data;

		fread(c->inbound_packet.data, 1, len, c->socket);
		if (feof(c->socket) || ferror(c->socket))
			return pn_eof;

		c->inbound_packet.stream = fmemopen(c->inbound_packet.data, len, "r");
	}

	return pn_ok;
}

pn_error_t read_inbound_packet_bio(struct connection *c) {
	long len = read_varint_from_bio(c->aes_decrypt_stream);

	if (len == 0)
		return pn_invalid_packet;

	if (len > 0) {
		c->inbound_packet.len = len;
		void *data = realloc(c->inbound_packet.data, len);

		if (!data)
			return pn_oom;

		c->inbound_packet.data = data;

		BIO_read(c->aes_decrypt_stream, c->inbound_packet.data, (int)len);
	}

	return pn_ok;
}

pn_error_t read_inbound_packet(struct connection *c) {
	if (c->encryption_enabled) {
		return read_inbound_packet_bio(c);
	} else {
		return read_inbound_packet_socket(c);
	}
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

pn_error_t send_outbound_packet_stream(struct connection *c) {
	write_varint_to_stream(c->socket, (long)c->outbound_packet.len);
	fwrite(c->outbound_packet.data, 1, c->outbound_packet.len, c->socket);

	if (feof(c->socket) || ferror(c->socket))
		return pn_eof;

	free(c->outbound_packet.data);
	c->outbound_packet.data = nullptr;

	return pn_ok;
}

pn_error_t send_outbound_packet_bio(struct connection *c) {
	write_varint_to_bio(c->aes_encrypt_stream, (long)c->outbound_packet.len);
	BIO_write(c->aes_encrypt_stream, c->outbound_packet.data, (int)c->outbound_packet.len);

	free(c->outbound_packet.data);
	c->outbound_packet.data = nullptr;

	return pn_ok;
}

pn_error_t send_outbound_packet(struct connection *c) {
	fclose(c->outbound_packet.stream);

	assert(c->outbound_packet.data != nullptr);
	assert(c->outbound_packet.len > 0);
	assert(!feof(c->outbound_packet.stream));
	assert(!ferror(c->outbound_packet.stream));

	if (c->encryption_enabled) {
		return send_outbound_packet_bio(c);
	} else {
		return send_outbound_packet_stream(c);
	}
}

void flush_connection_socket(struct connection *c) {
	if (c->encryption_enabled) {
		BIO_flush(c->aes_encrypt_stream);
	} else {
		fflush(c->socket);
	}
}
