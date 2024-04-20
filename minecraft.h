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

struct packet {
    void *data;
    size_t len;

    FILE *stream;
};

struct connection {
    int socket_fd;
    FILE *socket;
    enum connection_state state;

    bool disconnected;

    struct packet inbound_packet;
    struct packet outbound_packet;
};

static inline void flush_connection_socket(struct connection *c) {
    fflush(c->socket);
}

struct t_string {
    char *data;
    size_t len;
};

void read_inbound_packet(struct connection *);
void end_inbound_packet(struct connection *);
void new_outbound_packet(struct connection *, long id);
void send_outbound_packet(struct connection *);

long read_varint_from_stream(FILE *);
void write_varint_to_stream(FILE *, long);

long read_varint(struct connection *);
struct t_string read_string(struct connection *);
short read_short(struct connection *);
int read_int(struct connection *);
long read_long(struct connection *);

void write_varint(struct connection *, long);
void write_c_string(struct connection *, const char *);
void write_c_string_len(struct connection *, const char *, size_t len);
void write_string(struct connection *, struct t_string);
void write_short(struct connection *, short);
void write_int(struct connection *, int);
void write_long(struct connection *, long);

void handle_handshake(struct connection *);
void reply_to_status_request(struct connection *);
void reply_to_status_ping(struct connection *);