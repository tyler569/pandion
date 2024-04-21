#include "minecraft.h"
#include <string.h>

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
static pn_error_t send_encryption_request(struct connection *c);
static pn_error_t send_set_compression(struct connection *c, int threshold);
static pn_error_t send_login_success(struct connection *c);

pn_error_t handle_login_start(struct connection *c);
pn_error_t handle_encryption_response(struct connection *c);

pn_error_t handle_login_state(struct connection *c) {
	long packet_type = read_varint(c);

	printf("login packet type=%ld\n", packet_type);

	switch (packet_type) {
	case login_start:
		return handle_login_start(c);
	case encryption_response:
		return handle_encryption_response(c);
	case 2 ... 3:
		return pn_unhandled_packet;
	default:
		printf("unexpected packet type\n");
		return pn_invalid_packet;
	}
}

pn_error_t handle_login_start(struct connection *c) {
	struct t_string username = read_string(c);

	printf("  username=\"%.*s\"\n", (int)username.len, username.data);

	if (username.len >= 16) {
		send_disconnect(c, "Username too long");
		return pn_invalid_packet;
	}

	memcpy(c->username, username.data, username.len);

	return send_encryption_request(c);
}

pn_error_t handle_encryption_response(struct connection *c) {
	struct t_string shared_secret = read_string(c);
	struct t_string verify_token = read_string(c);

	unsigned char verify_token_buf[4];
	decrypt_data_rsa(
		c, verify_token_buf, sizeof(verify_token_buf), &verify_token);
	printf("  verify_token: ");
	for (int i = 0; i < sizeof(verify_token_buf); i++) {
		printf("%02hhx", verify_token_buf[i]);
	}
	printf("\n");
	if (memcmp(verify_token_buf, c->verify_token, sizeof(verify_token_buf)) != 0) {
		send_disconnect(c, "Invalid verify token");
		return pn_invalid_packet;
	}

	decrypt_data_rsa(
		c, c->shared_secret, sizeof(c->shared_secret), &shared_secret);

	printf("  shared_secret: ");
	for (int i = 0; i < sizeof(c->shared_secret); i++) {
		printf("%02hhx", c->shared_secret[i]);
	}
	printf("\n");

	init_connection_aes(c);

	return send_login_success(c);
}

static pn_error_t send_disconnect(struct connection *c, const char *reason) {
	new_outbound_packet(c, disconnect);

	write_fprintf(c, "{\"text\": \"%s\"}", reason);

	return send_outbound_packet(c);
}

static pn_error_t send_encryption_request(struct connection *c) {
	new_outbound_packet(c, encryption_request);

	generate_random_bytes(c->verify_token, sizeof(c->verify_token));

	write_c_string(c, "server id");
	write_data_len(c, c->server->der_public_key, c->server->der_public_key_len);
	write_data_len(c, c->verify_token, sizeof(c->verify_token));

	return send_outbound_packet(c);
}

static pn_error_t send_set_compression(struct connection *c, int threshold) {
	new_outbound_packet(c, set_compression);

	c->compression_threshold = threshold;
	write_varint(c, threshold);

	return send_outbound_packet(c);
}

static pn_error_t send_login_success(struct connection *c) {
	new_outbound_packet(c, login_success);

	unsigned char uuid[16];
	generate_random_bytes(uuid, sizeof(uuid));

	write_uuid(c, uuid);
	write_c_string(c, c->username);
	write_varint(c, 0);

	return send_outbound_packet(c);
}