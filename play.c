#include "minecraft.h"

enum play_inbound_packet_id {
	client_settings = 0x05,
	player_position = 0x11,
	player_position_and_rotation = 0x12,
	player_rotation = 0x13,
};

enum play_outbound_packet_id {
	join_game = 0x26,
	plugin_message_server = 0x18,
};

pn_error_t handle_client_settings(struct connection *c);
pn_error_t handle_position(struct connection *c);
pn_error_t handle_position_and_rotation(struct connection *c);
pn_error_t handle_rotation(struct connection *c);

pn_error_t send_join_game(struct connection *c);
pn_error_t send_plugin_message_server(struct connection *c);

pn_error_t handle_play_state(struct connection *c) {
	long packet_type = read_varint(c);

	printf("play packet type=%ld\n", packet_type);

	switch (packet_type) {
	case client_settings:
		return handle_client_settings(c);
	case player_position:
		return handle_position(c);
	case player_position_and_rotation:
		return handle_position_and_rotation(c);
	case player_rotation:
		return handle_rotation(c);
	default:
		printf("unhandled play packet type\n");
		return pn_unhandled_packet;
	}
}

pn_error_t handle_client_settings(struct connection *c) {
	struct t_string locale = read_string(c);
	char view_distance = read_byte(c);
	char chat_mode = read_byte(c);
	char chat_colors = read_byte(c);
	char displayed_skin_parts = read_byte(c);
	char main_hand = read_byte(c);
	char enable_text_filtering = read_byte(c);
	char allow_server_listings = read_byte(c);

	printf("  locale=\"%.*s\"\n", (int)locale.len, locale.data);
	printf("  view_distance=%d\n", view_distance);
	printf("  chat_mode=%d\n", chat_mode);
	printf("  chat_colors=%d\n", chat_colors);
	printf("  displayed_skin_parts=%d\n", displayed_skin_parts);
	printf("  main_hand=%d\n", main_hand);
	printf("  enable_text_filtering=%d\n", enable_text_filtering);
	printf("  allow_server_listings=%d\n", allow_server_listings);

	return pn_ok;
}

static void read_xyz(struct connection *c) {
	c->x = read_double(c);
	c->y = read_double(c);
	c->z = read_double(c);
}

static void read_rotation(struct connection *c) {
	c->yaw = read_float(c);
	c->pitch = read_float(c);
}

static void read_on_ground(struct connection *c) {
	c->on_ground = read_byte(c);
}

pn_error_t handle_position(struct connection *c) {
	read_xyz(c);
	read_on_ground(c);

	return pn_ok;
}

pn_error_t handle_position_and_rotation(struct connection *c) {
	read_xyz(c);
	read_rotation(c);
	read_on_ground(c);

	return pn_ok;
}

pn_error_t handle_rotation(struct connection *c) {
	read_rotation(c);
	read_on_ground(c);

	return pn_ok;
}

pn_error_t send_brand_plugin_message(struct connection *c) {
	new_outbound_packet(c, plugin_message_server);

	write_c_string(c, "minecraft:brand");
	write_data_len(c, "pandion", 7);

	return send_outbound_packet(c);
}

pn_error_t send_join_game(struct connection *c) {
	new_outbound_packet(c, join_game);

	write_int(c, c->entity_id);
	write_byte(c, 0); // hardcore
	write_byte(c, 1); // gamemode
	write_byte(c, 1); // previous gamemode
	write_varint(c, 1); // world count
	write_c_string(c, "minecraft:overworld"); // world names array
	write_nbt(c, c->server->dimension_codec);
	write_nbt(c, c->server->dimension);
	write_c_string(c, "minecraft:overworld"); // current world name
	write_long(c, 1); // hashed seed
	write_varint(c, 100); // max players (ignored)
	write_varint(c, 10); // view distance
	write_varint(c, 10); // simulation distance
	write_byte(c, 0); // reduced debug info
	write_byte(c, 1); // enable respawn screen
	write_byte(c, 0); // debug world
	write_byte(c, 0); // flat world

	return send_outbound_packet(c);
}

pn_error_t do_player_join_game(struct connection *c) {
	pn_error_t rc;

	rc = send_join_game(c);
	if (rc != pn_ok)
		return rc;

	rc = send_brand_plugin_message(c);
	if (rc != pn_ok)
		return rc;

	return pn_ok;
}
