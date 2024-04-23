#pragma once

#include "nbt.h"
#include "rbtree.h"
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
	pn_timeout,
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

struct chunk_section {
	int bits_per_block;

	short *palette;
	int palette_len;
	int palette_size;

	int filled_blocks;

	long *data;
	int data_len;
};

struct chunk {
	int x, z;

	short motion_blocking[256];
	struct nbt_tag *motion_blocking_nbt_cache;

	struct chunk_section sections[24];

	void *data_packet_cache;
	size_t data_packet_cache_len;
};

struct world {
	struct chunk chunk;
};

struct connection {
	struct server *server;

	int socket_fd;

	BIO *inbound_stream;
	BIO *outbound_stream;

	enum connection_state state;

	struct packet inbound_packet;
	struct packet outbound_packet;

	time_t last_keepalive_sent;
	time_t last_keepalive_received;

	unsigned char verify_token[4];
	unsigned char shared_secret[16];

	int compression_threshold;

	char username[16];
	unsigned char uuid[16];

	int entity_id;

	int chunk_x, chunk_z;
	double x, y, z;
	float yaw, pitch;
	bool on_ground;

	int last_teleport_id;
};

struct server {
	EVP_PKEY *server_key;
	unsigned char *der_public_key;
	size_t der_public_key_len;

	struct nbt_tag *dimension;
	struct nbt_tag *dimension_codec;

	struct world world;
};

struct t_string {
	char *data;
	int len;
};

void init_world(struct world *);
struct chunk *get_world_chunk(struct world *, int x, int z);

short get_world_block(struct world *w, int x, int y, int z);
void set_world_block(struct world *w, int x, int y, int z, short block);

void init_server_crypto(struct server *s);
void init_server_state(struct server *s);

void handle_client_connection(struct server *, int client_socket_fd);

pn_error_t decrypt_data_rsa(struct connection *, unsigned char *out,
	size_t out_len, struct t_string *in);

void init_connection_streams(struct connection *);
void init_connection_aes(struct connection *);
void close_connection_streams(struct connection *);

void generate_random_bytes(unsigned char *buf, size_t len);

pn_error_t read_inbound_packet(struct connection *);
void end_inbound_packet(struct connection *);
void new_outbound_packet(struct connection *, long id);
pn_error_t send_outbound_packet(struct connection *);
void flush_connection_socket(struct connection *);

struct chunk new_chunk(int x, int z);
pn_error_t write_chunk_data_to_packet(struct connection *c, struct chunk *k);

int read_varint_from_stream(FILE *stream);
void write_varint_to_stream(FILE *stream, int);
int read_varint_from_bio(BIO *bio);
void write_varint_to_bio(BIO *bio, int);

int read_varint(struct connection *);
struct t_string read_string(struct connection *);
char read_byte(struct connection *);
short read_short(struct connection *);
int read_int(struct connection *);
long read_long(struct connection *);
float read_float(struct connection *);
double read_double(struct connection *);
void read_uuid(struct connection *, unsigned char *uuid);
void read_position(struct connection *, int *x, int *y, int *z);

void write_varint(struct connection *, int);
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
void write_position(struct connection *, int x, int y, int z);

#define write_fprintf(c, fmt, ...) \
	fprintf(c->outbound_packet.stream, fmt, ##__VA_ARGS__)

pn_error_t handle_handshake(struct connection *);
pn_error_t handle_status_state(struct connection *);
pn_error_t handle_login_state(struct connection *);
pn_error_t handle_play_state(struct connection *);

pn_error_t do_player_join_game(struct connection *c);

static inline void print_bytes(const unsigned char *buf, size_t len) {
	for (size_t i = 0; i < len; i++) {
		printf("%02hhx", buf[i]);
	}
	printf("\n");
}
