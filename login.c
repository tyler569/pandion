#include "minecraft.h"

// protocol 758
enum login_inbound_packet_id {
	disconnect = 0,
	encryption_request = 1,
	login_success = 2,
	set_compression = 3,
};

enum login_outbound_packet_id {
	login_start = 0,
	encryption_response = 1,
};


static pn_error_t send_disconnect(struct connection *c, const char *reason);

pn_error_t handle_login_state(struct connection *c) {
	long packet_type = read_varint(c);

	printf("login packet type=%ld\n", packet_type);

	switch (packet_type) {
	case login_start:
		return send_disconnect(c, "{\"text\": \"Not implemented\"}");
	case 1 ... 3:
		return pn_unhandled_packet;
	default:
		printf("unexpected packet type\n");
		return pn_invalid_packet;
	}
}

static pn_error_t send_disconnect(struct connection *c, const char *reason) {
	new_outbound_packet(c, disconnect);

	write_c_string(c, reason);

	return send_outbound_packet(c);
}

pn_error_t handle_login_start(struct connection *c) {
	struct t_string username = read_string(c);

	printf("  username=\"%.*s\"\n", (int)username.len, username.data);

	return send_disconnect(c, "Not implemented");
}

pn_error_t handle_encryption_response(struct connection *c) {
	struct t_string shared_secret = read_string(c);
	struct t_string verify_token = read_string(c);

	printf("  shared_secret: ");
	for (int i = 0; i < shared_secret.len; i++) {
		printf("%02hhx", shared_secret.data[i]);
	}
	printf("\n");

	printf("  verify_token: ");
	for (int i = 0; i < verify_token.len; i++) {
		printf("%02hhx", verify_token.data[i]);
	}

	return pn_ok;
}
