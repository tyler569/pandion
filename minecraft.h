#pragma once

#include <stddef.h>
#include <stdio.h>

void hexdump(const void *data, size_t len);

enum connection_state {
    HANDSHAKE,
    STATUS,
    LOGIN,
    PLAY,
};

struct connection {
    FILE *socket;
    enum connection_state state;
};

static inline void flush_connection_socket(struct connection *c) {
    fflush(c->socket);
}

struct packet {
    unsigned char *data;
    size_t len;

    FILE *reader_writer;
};

struct string {
    char *data;
    size_t len;
};

struct packet *read_packet(FILE *file);
struct packet *new_packet();
void write_packet(struct connection *, struct packet *);
void free_packet(struct packet *);

long read_varint_from_stream(FILE *);
void write_varint_to_stream(FILE *, long);

long read_varint(struct packet *);
struct string read_string(struct packet *);
short read_short(struct packet *);
int read_int(struct packet *);
long read_long(struct packet *);

void write_varint(struct packet *, long);
void write_c_string(struct packet *, const char *);
void write_c_string_len(struct packet *, const char *, size_t len);
void write_string(struct packet *, struct string);
void write_short(struct packet *, short);
void write_int(struct packet *, int);
void write_long(struct packet *, long);

void handle_handshake(struct connection *, struct packet *);
void reply_to_status_request(struct connection *, struct packet *);
void reply_to_status_ping(struct connection *, struct packet *);