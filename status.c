#include "minecraft.h"
#include <stdio.h>

// protocol 758
enum status_inbound_packet_id {
	status_response = 0,
	pong = 1,
};

enum status_outbound_packet_id {
	status_request = 0,
	ping = 1,
};

pn_error_t reply_to_status_request(struct connection *c);
pn_error_t reply_to_status_ping(struct connection *c);

const char *status_json
	= "{\"version\":{\"name\":\"1.18.2\",\"protocol\":758},\"players\":{"
	  "\"max\":100,\"online\":0},\"description\":{\"text\":\"Hello, world!\"}}";

pn_error_t handle_status_state(struct connection *c) {
	long packet_type = read_varint(c);

	printf("status packet type=%ld\n", packet_type);

	switch (packet_type) {
	case status_request:
		return reply_to_status_request(c);
	case ping:
		return reply_to_status_ping(c);
	default:
		printf("invalid status packet type\n");
		return pn_invalid_packet;
	}
}

pn_error_t handle_handshake(struct connection *c) {
	long packet_type = read_varint(c);
	if (packet_type != 0) {
		printf("invalid handshake packet id\n");
		return pn_invalid_packet;
	}

	printf("handshake packet type=%ld\n", packet_type);

	long version = read_varint(c);
	struct t_string address = read_string(c);
	short port = read_short(c);
	long state = read_varint(c);

	printf("  version=%ld\n", version);
	printf("  address=\"%.*s\"\n", (int)address.len, address.data);
	printf("  port=%d\n", port);
	printf("  state=%ld\n", state);

	switch (state) {
	case 1:
		c->state = STATUS;
		break;
	case 2:
		c->state = LOGIN;
		break;
	default:
		printf("  invalid state\n");
		return pn_invalid_state;
	}

	return pn_ok;
}

pn_error_t reply_to_status_request(struct connection *c) {
	pn_error_t rc;

	printf("  status request\n");

	new_outbound_packet(c, status_response);

	write_c_string(c, status_json);

	rc = send_outbound_packet(c);
	if (rc != pn_ok)
		return rc;

	flush_connection_socket(c);

	return pn_ok;
}

pn_error_t reply_to_status_ping(struct connection *c) {
	pn_error_t rc;

	long payload = read_long(c);

	printf("  ping payload=%ld\n", payload);

	new_outbound_packet(c, pong);

	write_long(c, payload);

	rc = send_outbound_packet(c);
	if (rc != pn_ok)
		return rc;

	flush_connection_socket(c);

	return pn_ok;
}