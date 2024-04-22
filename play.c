#include "minecraft.h"
#include <math.h>

// protocol 758
enum play_inbound_packet_id {
	client_settings = 0x05,
	player_position = 0x11,
	player_position_and_rotation = 0x12,
	player_rotation = 0x13,
	keepalive_client = 0x0f,
};

enum play_outbound_packet_id {
	join_game = 0x26,
	plugin_message_server = 0x18,
	keepalive_server = 0x21,
	chunk_data = 0x22,
	teleport = 0x38,
	update_view_position = 0x49,
};

pn_error_t process_keepalive_status(struct connection *c);

pn_error_t handle_client_settings(struct connection *c);
pn_error_t handle_position(struct connection *c);
pn_error_t handle_position_and_rotation(struct connection *c);
pn_error_t handle_rotation(struct connection *c);
pn_error_t handle_keepalive(struct connection *c);

pn_error_t send_join_game(struct connection *c);
pn_error_t send_plugin_message_server(struct connection *c);
pn_error_t send_keep_alive(struct connection *c);
pn_error_t send_teleport(struct connection *c);
pn_error_t send_update_view_position(struct connection *c);

pn_error_t handle_play_state(struct connection *c) {
	pn_error_t rc;

	rc = process_keepalive_status(c);
	if (rc != pn_ok)
		return rc;

	long packet_type = read_varint(c);

	printf("play packet type=%ld\n", packet_type);

	switch (packet_type) {
	case client_settings:
		rc = handle_client_settings(c);
		break;
	case player_position:
		rc = handle_position(c);
		break;
	case player_position_and_rotation:
		rc = handle_position_and_rotation(c);
		break;
	case player_rotation:
		rc = handle_rotation(c);
		break;
	case keepalive_client:
		rc = handle_keepalive(c);
		break;
	default:
		rc = pn_unhandled_packet;
	}

	if (rc == pn_unhandled_packet) {
		printf("  unhandled packet\n");
	} else if (rc != pn_ok) {
		return rc;
	}

	return pn_ok;
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

	printf("  locale=\"%.*s\"\n", locale.len, locale.data);
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

int chunk_index(double position) { return (int)floor(position / 16); }

pn_error_t handle_any_movement(struct connection *c) {
	pn_error_t rc;

	if (chunk_index(c->x) != c->chunk_x || chunk_index(c->z) != c->chunk_z) {
		c->chunk_x = chunk_index(c->x);
		c->chunk_z = chunk_index(c->z);

		rc = send_update_view_position(c);

		if (rc != pn_ok)
			return rc;
	}

	return pn_ok;
}

pn_error_t handle_position(struct connection *c) {
	read_xyz(c);
	read_on_ground(c);

	return handle_any_movement(c);
}

pn_error_t handle_position_and_rotation(struct connection *c) {
	read_xyz(c);
	read_rotation(c);
	read_on_ground(c);

	return handle_any_movement(c);
}

pn_error_t handle_rotation(struct connection *c) {
	read_rotation(c);
	read_on_ground(c);

	return handle_any_movement(c);
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

pn_error_t send_chunk_data(struct connection *c, struct chunk *k);

pn_error_t do_player_join_game(struct connection *c) {
	pn_error_t rc;

	rc = send_join_game(c);
	if (rc != pn_ok)
		return rc;

	rc = send_brand_plugin_message(c);
	if (rc != pn_ok)
		return rc;

	c->chunk_x = 0;
	c->chunk_z = 0;
	c->x = 0.5;
	c->y = 64;
	c->z = 0.5;
	c->yaw = 0;
	c->pitch = 0;
	c->on_ground = false;

	rc = send_teleport(c);
	if (rc != pn_ok)
		return rc;

	rc = send_update_view_position(c);
	if (rc != pn_ok)
		return rc;

	static struct chunk k;
	static bool init = false;
	if (!init) {
		k = new_chunk(0, 0);
	}

	for (int x = -3; x <= 3; x++) {
		for (int z = -3; z <= 3; z++) {
			k.x = x;
			k.z = z;

			rc = send_chunk_data(c, &k);
			if (rc != pn_ok)
				return rc;
		}
	}

	c->last_keepalive_received = time(nullptr);

	return pn_ok;
}

pn_error_t handle_keepalive(struct connection *c) {
	long keepalive_id = read_long(c);
	printf("  keepalive id=%ld\n", keepalive_id);

	if (keepalive_id == c->last_keepalive_sent) {
		c->last_keepalive_received = time(nullptr);
		return pn_ok;
	} else {
		return pn_invalid_packet;
	}
}

pn_error_t send_keep_alive(struct connection *c) {
	new_outbound_packet(c, keepalive_server);

	write_long(c, c->last_keepalive_sent);

	return send_outbound_packet(c);
}

pn_error_t send_chunk_data(struct connection *c, struct chunk *k) {
	new_outbound_packet(c, chunk_data);

	write_chunk_data_to_packet(c, k);

	return send_outbound_packet(c);
}

pn_error_t send_teleport(struct connection *c) {
	new_outbound_packet(c, teleport);

	write_double(c, c->x);
	write_double(c, c->y);
	write_double(c, c->z);
	write_float(c, c->yaw);
	write_float(c, c->pitch);
	write_byte(c, 0); // flags
	write_varint(c, ++c->last_teleport_id);
	write_byte(c, 0); // dismount vehicle

	return send_outbound_packet(c);
}

pn_error_t send_update_view_position(struct connection *c) {
	new_outbound_packet(c, update_view_position);

	write_varint(c, c->chunk_x);
	write_varint(c, c->chunk_z);

	return send_outbound_packet(c);
}

pn_error_t process_keepalive_status(struct connection *c) {
	time_t now = time(nullptr);

	if (now - c->last_keepalive_sent > 20) {
		c->last_keepalive_sent = now;

		pn_error_t rc = send_keep_alive(c);
		if (rc != pn_ok)
			return rc;
	}

	if (now - c->last_keepalive_received > 30)
		return pn_timeout;

	return pn_ok;
}