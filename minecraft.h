#pragma once

#include "nbt.h"
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

	struct nbt_tag *dimension;
	struct nbt_tag *dimension_codec;
};

void init_server_crypto(struct server *s);
void init_server_state(struct server *s);

struct connection {
	struct server *server;

	int socket_fd;

	BIO *inbound_stream;
	BIO *outbound_stream;

	enum connection_state state;

	struct packet inbound_packet;
	struct packet outbound_packet;

	unsigned char verify_token[4];
	unsigned char shared_secret[16];

	int compression_threshold;
	bool encryption_enabled;

	char username[16];
	unsigned char uuid[16];

	int entity_id;

	double x, y, z;
	float yaw, pitch;
};

void handle_client_connection(struct server *, int client_socket_fd);

struct t_string {
	char *data;
	size_t len;
};

pn_error_t decrypt_data_rsa(struct connection *, unsigned char *out,
	size_t out_len, struct t_string *in);

void init_connection_aes(struct connection *);
void free_connection_aes(struct connection *);

void generate_random_bytes(unsigned char *buf, size_t len);

pn_error_t read_inbound_packet(struct connection *);
void end_inbound_packet(struct connection *);
void new_outbound_packet(struct connection *, long id);
pn_error_t send_outbound_packet(struct connection *);
void flush_connection_socket(struct connection *);

long read_varint_from_stream(FILE *);
void write_varint_to_stream(FILE *, long);
long read_varint_from_buffer(void *data, size_t len);
long read_varint_from_bio(BIO *);
void write_varint_to_bio(BIO *, long);

long read_varint(struct connection *);
struct t_string read_string(struct connection *);
char read_byte(struct connection *);
short read_short(struct connection *);
int read_int(struct connection *);
long read_long(struct connection *);
float read_float(struct connection *);
double read_double(struct connection *);
void read_uuid(struct connection *, unsigned char *uuid);

void write_varint(struct connection *, long);
void write_c_string(struct connection *, const char *);
void write_data_len(struct connection *, const void *, size_t len);
void write_string(struct connection *, struct t_string);
void write_byte(struct connection *, char);
void write_short(struct connection *, short);
void write_int(struct connection *, int);
void write_long(struct connection *, long);
void write_float(struct connection *, float);
void write_double(struct connection *, double);
void write_uuid(struct connection *, unsigned char *uuid);
void write_nbt(struct connection *, struct nbt_tag *);

#define write_fprintf(c, fmt, ...) \
	fprintf(c->outbound_packet.stream, fmt, ##__VA_ARGS__)

pn_error_t handle_handshake(struct connection *);
pn_error_t handle_status_state(struct connection *);
pn_error_t handle_login_state(struct connection *);
pn_error_t handle_play_state(struct connection *);

static inline void print_bytes(const unsigned char *buf, size_t len) {
	for (size_t i = 0; i < len; i++) {
		printf("%02hhx", buf[i]);
	}
	printf("\n");
}