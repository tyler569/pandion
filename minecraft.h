#pragma once

#include <openssl/evp.h>
#include <stddef.h>
#include <stdio.h>

enum pn_error {
	pn_ok = 0,
	pn_eof,
	pn_invalid_packet,
	pn_invalid_state,
	pn_oom,
	pn_unhandled_packet,
};
typedef enum pn_error pn_error_t;

void hexdump(const void *data, size_t len);

enum connection_state {
	HANDSHAKE,
	STATUS,
	LOGIN,
	PLAY,
};

struct packet {
	void *data;
	size_t len;

	FILE *stream;
};

struct server {
	EVP_PKEY *server_key;
	unsigned char *der_public_key;
	size_t der_public_key_len;
};

void init_server_crypto(struct server *s);

struct connection {
	struct server *server;

	int socket_fd;
	FILE *socket;
	enum connection_state state;

	char username[16];
	unsigned char verify_token[4];
	unsigned char shared_secret[16];

	struct packet inbound_packet;
	struct packet outbound_packet;

	double x, y, z;
	float yaw, pitch;
};

void handle_client_connection(struct server *, int client_socket_fd);

static inline void flush_connection_socket(struct connection *c) {
	fflush(c->socket);
}

struct t_string {
	char *data;
	size_t len;
};

pn_error_t rsa_decrypt_data(struct connection *c, unsigned char *out,
	size_t out_len, struct t_string *in);

pn_error_t read_inbound_packet(struct connection *);
void end_inbound_packet(struct connection *);
void new_outbound_packet(struct connection *, long id);
pn_error_t send_outbound_packet(struct connection *);

long read_varint_from_stream(FILE *);
void write_varint_to_stream(FILE *, long);

long read_varint(struct connection *);
struct t_string read_string(struct connection *);
short read_short(struct connection *);
int read_int(struct connection *);
long read_long(struct connection *);

void write_varint(struct connection *, long);
void write_c_string(struct connection *, const char *);
void write_data_len(struct connection *, const void *, size_t len);
void write_string(struct connection *, struct t_string);
void write_short(struct connection *, short);
void write_int(struct connection *, int);
void write_long(struct connection *, long);
#define write_fprintf(c, fmt, ...) fprintf(c->outbound_packet.stream, fmt , ## __VA_ARGS__)

pn_error_t handle_handshake(struct connection *);
pn_error_t handle_status_state(struct connection *);
pn_error_t handle_login_state(struct connection *);
pn_error_t handle_play_state(struct connection *);

// pn_error_t reply_to_status_request(struct connection *);
// pn_error_t reply_to_status_ping(struct connection *);