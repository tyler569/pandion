#include "minecraft.h"
#include "nbt.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct t_string read_string(struct connection *c) {
	struct t_string string;

	string.len = read_varint(c);
	string.data = malloc(string.len + 1);
	fread(string.data, 1, string.len, c->inbound_packet.stream);
	string.data[string.len] = '\0';

	return string;
}

char read_byte(struct connection *c) {
	int i = fgetc(c->inbound_packet.stream);

	if (i == EOF)
		return 0;

	return (char)i;
}

short read_short(struct connection *c) {
	unsigned char buf[2];
	fread(buf, 1, 2, c->inbound_packet.stream);
	return (short)(((int)buf[0] << 8) | (int)buf[1]);
}

int read_int(struct connection *c) {
	unsigned char buf[4];
	fread(buf, 1, 4, c->inbound_packet.stream);
	return ((int)buf[0] << 24) | ((int)buf[1] << 16) | ((int)buf[2] << 8)
		| (int)buf[3];
}

long read_long(struct connection *c) {
	unsigned char buf[8];
	fread(buf, 1, 8, c->inbound_packet.stream);
	return ((long)buf[0] << 56) | ((long)buf[1] << 48) | ((long)buf[2] << 40)
		| ((long)buf[3] << 32) | ((long)buf[4] << 24) | ((long)buf[5] << 16)
		| ((long)buf[6] << 8) | (long)buf[7];
}

float read_float(struct connection *c) {
	int i = read_int(c);
	return *(float *)&i;
}

double read_double(struct connection *c) {
	long i = read_long(c);
	return *(double *)&i;
}

void read_position(struct connection *c, int *x, int *y, int *z) {
	long val = read_long(c);

	*x = (int)(val >> 38);
	*y = (int)(val << 52 >> 52);
	*z = (int)(val << 26 >> 38);
}

void write_c_string(struct connection *c, const char *string) {
	write_varint(c, (long)strlen(string));
	fputs(string, c->outbound_packet.stream);
}

void write_data_len(struct connection *c, const void *data, size_t len) {
	write_varint(c, (long)len);
	fwrite(data, 1, len, c->outbound_packet.stream);
}

void write_string(struct connection *c, struct t_string string) {
	write_varint(c, (long)string.len);
	fwrite(string.data, 1, string.len, c->outbound_packet.stream);
}

void write_byte(struct connection *c, char value) {
	fputc(value, c->outbound_packet.stream);
}

void write_short(struct connection *c, short value) {
	value = htons(value);
	fwrite(&value, 1, 2, c->outbound_packet.stream);
}

void write_int(struct connection *c, int value) {
	fputc(value >> 24, c->outbound_packet.stream);
	fputc(value >> 16, c->outbound_packet.stream);
	fputc(value >> 8, c->outbound_packet.stream);
	fputc(value, c->outbound_packet.stream);
}

void write_long(struct connection *c, long value) {
	fputc((char)(value >> 56), c->outbound_packet.stream);
	fputc((char)(value >> 48), c->outbound_packet.stream);
	fputc((char)(value >> 40), c->outbound_packet.stream);
	fputc((char)(value >> 32), c->outbound_packet.stream);
	fputc((char)(value >> 24), c->outbound_packet.stream);
	fputc((char)(value >> 16), c->outbound_packet.stream);
	fputc((char)(value >> 8), c->outbound_packet.stream);
	fputc((char)value, c->outbound_packet.stream);
}

void write_float(struct connection *c, float value) {
	write_int(c, *(int *)&value);
}

void write_double(struct connection *c, double value) {
	write_long(c, *(long *)&value);
}

void write_uuid(struct connection *c, unsigned char *uuid) {
	fwrite(uuid, 1, 16, c->outbound_packet.stream);
}

void write_nbt(struct connection *c, struct nbt_tag *tag) {
	nbt_write_to_stream(tag, c->outbound_packet.stream);
}

void write_position(struct connection *c, int x, int y, int z) {
	assert(x >= -33554432 && x < 33554432);
	assert(y >= -2048 && y < 2048);
	assert(z >= -33554432 && z < 33554432);

	write_long(c,
		(((long)x & 0x3FFFFFF) << 38) | (((long)z & 0x3FFFFFF) << 12)
			| ((long)y & 0xFFF));
}