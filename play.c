#include "minecraft.h"

enum play_inbound_packet_id {
	client_settings = 0x05,
	plugin_message_client = 0x0a,
	player_position = 0x11,
	player_position_and_rotation = 0x12,
};

enum play_outbound_packet_id {
	join_game = 0x26,
	plugin_message_server = 0x18,
};

pn_error_t handle_client_settings(struct connection *c);
pn_error_t handle_plugin_message_client(struct connection *c);
pn_error_t handle_player_position(struct connection *c);
pn_error_t handle_player_position_and_rotation(struct connection *c);

pn_error_t send_join_game(struct connection *c);
pn_error_t send_plugin_message_server(struct connection *c);

pn_error_t handle_play_state(struct connection *c) {
	long packet_type = read_varint(c);

	printf("play packet type=%ld\n", packet_type);

	switch (packet_type) {
	default:
		printf("unhandled play packet type\n");
		return pn_unhandled_packet;
	}
}

pn_error_t send_join_game(struct connection *c) {
	new_outbound_packet(c, join_game);

	write_int(c, c->entity_id);
	write_byte(c, 0); // hardcore
	write_byte(c, 1); // gamemode
	write_byte(c, -1); // previous gamemode
	write_varint(c, 1); // world count
	write_c_string(c, "minecraft:overworld"); // world names array
	write_nbt(c, c->server->dimension_codec);
	write_nbt(c, c->server->dimension);
	write_c_string(c, "minecraft:overworld"); // current world name
	write_long(c, 0); // hashed seed
	write_varint(c, 0); // max players (ignored)
	write_varint(c, 10); // view distance
	write_byte(c, 0); // reduced debug info
	write_byte(c, 1); // enable respawn screen
	write_byte(c, 0); // debug world
	write_byte(c, 0); // flat world

	return send_outbound_packet(c);
}