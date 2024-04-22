#include "minecraft.h"
#include <assert.h>

pn_error_t write_chunk_data(struct connection *c, struct chunk *k) {
	static struct nbt_tag *heightmap = nullptr;

	if (!heightmap) {
		int c = 0; // do not use the connection in this block

		heightmap = nbt_new_compound();

		long *motion_blocking_array = calloc(37, sizeof(long));

		for (int i = 0; i < 256; i++) {
			int l_index = i / 7;
			int b_index = (i % 7) * 9;

			assert(l_index < 37);

			motion_blocking_array[l_index] |= 1l << b_index;
		}

		printf("motion_blocking_array: ");
		for (int i = 0; i < 37; i++) {
			printf("[%i] = %#018lx\n", i, motion_blocking_array[i]);
		}
		printf("\n");

		struct nbt_tag *motion_blocking
			= nbt_new_long_array(motion_blocking_array, 37);

		nbt_add_to_compound(heightmap, "MOTION_BLOCKING", motion_blocking);

		FILE *test_file = fopen("heightmap.nbt", "wb");
		nbt_write_to_stream(heightmap, test_file);
		fclose(test_file);
	}

	write_int(c, k->x);
	write_int(c, k->z);
	write_nbt(c, heightmap);

	static char *data = nullptr;
	static size_t len = 0;

	if (!data) {
		int c = 0; // do not use the connection in this block

		FILE *stream = open_memstream(&data, &len);

		// first chunk section
		{
			short block_count = htons(16 * 16);
			fwrite(&block_count, 1, sizeof(block_count), stream);

			// bits per block
			write_varint_to_stream(stream, 1);

			// length of the palette
			write_varint_to_stream(stream, 2);

			// palette
			write_varint_to_stream(stream, 0);
			write_varint_to_stream(stream, 1);

			// data array length
			write_varint_to_stream(stream, 64);

			long full = -1;
			long empty = 0;

			// bottom layer
			for (int i = 0; i < 4; i++)
				fwrite(&full, 1, sizeof(full), stream);

			// rest of the chunk section
			for (int i = 4; i < 64; i++)
				fwrite(&empty, 1, sizeof(empty), stream);

			// bits per biome
			write_varint_to_stream(stream, 0);

			// biome palette
			write_varint_to_stream(stream, 0);

			// biome data array length
			write_varint_to_stream(stream, 0);
		}

		// chunk sections 1-23
		for (int i = 1; i < 24; i++) {
			short block_count = 0;
			fwrite(&block_count, 1, sizeof(block_count), stream);

			// bits per block
			write_varint_to_stream(stream, 0);

			// palette
			write_varint_to_stream(stream, 0);

			// data array length
			write_varint_to_stream(stream, 0);

			// bits per biome
			write_varint_to_stream(stream, 0);

			// biome palette
			write_varint_to_stream(stream, 0);

			// biome data array length
			write_varint_to_stream(stream, 0);
		}

		fclose(stream);

		hexdump(data, len);
	}

	write_data_len(c, data, len);

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