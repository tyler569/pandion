#include "minecraft.h"
#include <stdio.h>

const char *handshake_json = "{\"version\":{\"name\":\"1.20\",\"protocol\":765},\"players\":{\"max\":100,\"online\":0},\"description\":{\"text\":\"Hello, world!\"}}";

void handle_handshake(struct connection *c, struct packet *p) {
    long version = read_varint(p);
    struct string address = read_string(p);
    short port = read_short(p);
    long state = read_varint(p);

    printf("  version=%ld\n", version);
    printf("  address=\"%.*s\"\n", (int) address.len, address.data);
    printf("  port=%d\n", port);
    printf("  state=%ld\n", state);

    c->state = STATUS;
}

void reply_to_status_request(struct connection *c, struct packet *) {
    printf("  status request\n");

    struct packet *response = new_packet();

    write_varint(response, 0); // packet ID
    write_c_string(response, handshake_json);

    write_packet(c, response);
    flush_connection_socket(c);

    free_packet(response);
}

void reply_to_ping(struct connection *c, struct packet *p) {
    long payload = read_long(p);

    printf("  ping payload=%ld\n", payload);

    struct packet *response = new_packet();

    write_varint(response, 1); // p ID
    write_long(response, payload);

    write_packet(c, response);
    flush_connection_socket(c);

    free_packet(response);
}