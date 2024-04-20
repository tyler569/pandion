#include "minecraft.h"
#include <stdio.h>

const char *handshake_json = "{\"version\":{\"name\":\"1.18.2\",\"protocol\":758},\"players\":{\"max\":100,\"online\":0},\"description\":{\"text\":\"Hello, world!\"}}";

void handle_handshake(struct connection *c) {
    long version = read_varint(c);
    struct t_string address = read_string(c);
    short port = read_short(c);
    long state = read_varint(c);

    printf("  version=%ld\n", version);
    printf("  address=\"%.*s\"\n", (int) address.len, address.data);
    printf("  port=%d\n", port);
	printf("  state=%ld\n", state);

	switch (state) {
	case 1:
		c->state = STATUS;
		break;
	case 2:
		c->state = LOGIN;
		break;
	}
}

void reply_to_status_request(struct connection *c) {
    printf("  status request\n");

    new_outbound_packet(c, 0);

    write_c_string(c, handshake_json);

    send_outbound_packet(c);
    flush_connection_socket(c);
}

void reply_to_status_ping(struct connection *c) {
    long payload = read_long(c);

    printf("  ping payload=%ld\n", payload);

    new_outbound_packet(c, 1);

    write_long(c, payload);

    send_outbound_packet(c);
    flush_connection_socket(c);
}