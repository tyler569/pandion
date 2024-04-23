#include "minecraft.h"
#include <assert.h>

static int cs_correct_bits_per_block(struct chunk_section *cs) {
	assert(cs->palette_len >= 0);

	switch (cs->palette_len) {
	case 0 ... 1:
		return 0;
	case 2 ... 16:
		return 4;
	case 17 ... 32:
		return 5;
	case 33 ... 64:
		return 6;
	case 65 ... 128:
		return 7;
	case 129 ... 256:
		return 8;
	default:
		return 16;
	}
}

static long cs_block_mask(int bits_per_block) {
	return (1l << bits_per_block) - 1;
}

static int cs_get(struct chunk_section *cs, int index) {
	if (cs->bits_per_block == 0 && cs->palette == nullptr) {
		return 0;
	}

	if (cs->bits_per_block == 0) {
		assert(cs->palette_len == 2);

		return cs->palette[1];
	}

	int blocks_per_long = 64 / cs->bits_per_block;

	int long_index = index / blocks_per_long;
	int block_index = index % blocks_per_long;

	assert(long_index < cs->data_len);

	long l = cs->data[long_index];
	l >>= block_index * cs->bits_per_block;
	return (short)(l & cs_block_mask(cs->bits_per_block));
}

static void cs_set_raw(struct chunk_section *cs, int index, int id) {
	int blocks_per_long = 64 / cs->bits_per_block;

	int long_index = index / blocks_per_long;
	int block_index = index % blocks_per_long;

	assert(long_index < cs->data_len);

	long mask = cs_block_mask(cs->bits_per_block);

	cs->data[long_index] &= ~(mask << block_index * cs->bits_per_block);
	cs->data[long_index] |= (long)id << block_index * cs->bits_per_block;
}

static void cs_relayout_block_data(struct chunk_section *cs) {
	printf("relayout\n");

	int new_bits_per_block = cs_correct_bits_per_block(cs);

	int new_blocks_per_long = 64 / new_bits_per_block;
	int new_data_len = 16 * 16 * 16 / new_blocks_per_long;

	long *new_data = calloc(new_data_len, sizeof(long));

	struct chunk_section new_cs = { .bits_per_block = new_bits_per_block,
		.palette = cs->palette,
		.palette_len = cs->palette_len,
		.palette_size = cs->palette_size,
		.filled_blocks = cs->filled_blocks,
		.data = new_data,
		.data_len = new_data_len };

	for (int i = 0; i < 16 * 16 * 16; i++) {
		int block = cs_get(cs, i);
		cs_set_raw(&new_cs, i, block);
	}

	free(cs->data);
	*cs = new_cs;
}

static void cs_set(struct chunk_section *cs, int index, int id) {
	int was = cs_get(cs, index);

	if (was == id) {
		return;
	}

	if (id && was == 0) {
		cs->filled_blocks++;
	} else if (id == 0 && was) {
		cs->filled_blocks--;
	}

	if (cs->bits_per_block == 0) {
		cs_relayout_block_data(cs);
	}

	cs_set_raw(cs, index, id);
}

static int cs_expand_palette(struct chunk_section *cs, short block) {
	if (cs->palette_len == 0) {
		cs->palette = calloc(16, sizeof(short));
		cs->palette_size = 16;
		cs->palette_len = 2;

		cs->palette[0] = 0;
		cs->palette[1] = block;
	} else if (cs->palette_len == cs->palette_size) {
		cs->palette_size *= 2;
		cs->palette = realloc(cs->palette, cs->palette_size * sizeof(short));

		cs->palette[cs->palette_len++] = block;
	}

	if (cs->bits_per_block != cs_correct_bits_per_block(cs)) {
		cs_relayout_block_data(cs);
	}

	return cs->palette_len - 1;
}

static int cs_palette_id(struct chunk_section *cs, short block) {
	for (int i = 0; i < cs->palette_len; i++) {
		if (cs->palette[i] == block) {
			return i;
		}
	}

	return cs_expand_palette(cs, block);
}

static void cs_set_block(
	struct chunk_section *cs, int x, int y, int z, short block) {
	int index = y * 16 * 16 + z * 16 + x;

	int paletted_id = cs_palette_id(cs, block);

	cs_set(cs, index, paletted_id);
}

static short cs_get_block(struct chunk_section *cs, int x, int y, int z) {
	int index = y * 16 * 16 + z * 16 + x;

	int paletted_id = cs_get(cs, index);

	return cs->palette[paletted_id];
}

static void cs_serialize_to_stream(struct chunk_section *cs, FILE *stream) {
	short block_count = htons(cs->filled_blocks);
	fwrite(&block_count, 1, sizeof(block_count), stream);

	write_varint_to_stream(stream, cs->bits_per_block);

	// single-valued palette in the special all-air case
	if (cs->bits_per_block == 0 && cs->palette == nullptr) {
		write_varint_to_stream(stream, 0);
	}

	// single-valued palette in the general case
	else if (cs->bits_per_block == 0) {
		assert(cs->palette_len == 2);

		write_varint_to_stream(stream, cs->palette[1]);
	}

	// multi-valued palette
	else if (cs->bits_per_block < 16) {
		write_varint_to_stream(stream, cs->palette_len);

		for (int i = 0; i < cs->palette_len; i++) {
			write_varint_to_stream(stream, cs->palette[i]);
		}
	}

	// direct map, no palette
	else { }

	write_varint_to_stream(stream, cs->data_len);

	for (int i = 0; i < cs->data_len; i++) {
		unsigned long be = __builtin_bswap64(cs->data[i]);
		fwrite(&be, 1, sizeof(be), stream);
	}

	write_varint_to_stream(stream, 0); // bits per biome

	write_varint_to_stream(stream, 0); // biome palette length

	write_varint_to_stream(stream, 0); // biome data array length
}

static struct chunk_section uniblock_cs(short block_state) {
	struct chunk_section r = {
		.bits_per_block = 0,
		.palette = calloc(16, sizeof(short)),
		.palette_len = 2,
		.palette_size = 16,
		.filled_blocks = block_state != 0 ? 4096 : 0,
		.data = nullptr,
		.data_len = 0,
	};

	r.palette[1] = block_state;

	return r;
}

static struct chunk_section stone_cs() { return uniblock_cs(1); }

static struct chunk_section *get_chunk_section(struct chunk *c, int y) {
	int section_index = (y + 64) / 16;

	assert(section_index >= 0 && section_index < 24);

	return &c->sections[section_index];
}

static int chunk_16_index(int y) {
	int ny = y % 16;
	if (ny < 0) {
		ny += 16;
	}
	return ny;
}

static void set_chunk_block(struct chunk *c, int x, int y, int z, short block) {
	struct chunk_section *cs = get_chunk_section(c, y);

	cs_set_block(cs, x, chunk_16_index(y), z, block);

	free(c->data_packet_cache);
	c->data_packet_cache = nullptr;
	c->data_packet_cache_len = 0;

	// TODO: update motion_blocking if we're the highest block in the column
}

static short get_chunk_block(struct chunk *c, int x, int y, int z) {
	struct chunk_section *cs = get_chunk_section(c, y);

	return cs_get_block(cs, x, chunk_16_index(y), z);
}

pn_error_t write_chunk_data_to_packet(struct connection *c, struct chunk *k) {
	write_int(c, k->x);
	write_int(c, k->z);

	if (!k->motion_blocking_nbt_cache) {
		struct nbt_tag *mb_compound = nbt_new_compound();

		long *mb_array = calloc(37, sizeof(long));

		struct nbt_tag *mb_tag = nbt_new_long_array(mb_array, 37);

		for (int i = 0; i < 256; i++) {
			int l_index = i / 7;
			int b_index = (i % 7) * 9;

			mb_array[l_index] |= (long)k->motion_blocking[i] << b_index;
		}

		nbt_add_to_compound(mb_compound, "MOTION_BLOCKING", mb_tag);

		free(mb_array);

		k->motion_blocking_nbt_cache = mb_compound;
	}

	write_nbt(c, k->motion_blocking_nbt_cache);

	if (!k->data_packet_cache) {
		FILE *stream = open_memstream(
			(char **)&k->data_packet_cache, &k->data_packet_cache_len);

		for (int i = 0; i < 24; i++) {
			cs_serialize_to_stream(&k->sections[i], stream);
		}

		fclose(stream);
	}

	write_data_len(c, k->data_packet_cache, k->data_packet_cache_len);

	write_varint(c, 0); // block entities count

	write_byte(c, 0); // trust edges

	write_varint(c, 0); // sky light mask bitset length
	write_varint(c, 0); // block light mask bitset length
	write_varint(c, 0); // empty sky light mask bitset length
	write_varint(c, 0); // empty block light mask bitset length
	write_varint(c, 0); // sky light array count
	write_varint(c, 0); // block light array count

	return pn_ok;
}

void init_world(struct world *w) {
	w->chunk = (struct chunk) {
		.sections = {
			stone_cs(),
			stone_cs(),
			stone_cs(),
			stone_cs(),
		},
	};
}

struct chunk *get_world_chunk(struct world *w, int, int) { return &w->chunk; }

short get_world_block(struct world *w, int x, int y, int z) {
	struct chunk *c = get_world_chunk(w, x, z);
	return get_chunk_block(c, chunk_16_index(x), y, chunk_16_index(z));
}

void set_world_block(struct world *w, int x, int y, int z, short block) {
	struct chunk *c = get_world_chunk(w, x, z);
	set_chunk_block(c, chunk_16_index(x), y, chunk_16_index(z), block);
}
