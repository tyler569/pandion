#include "minecraft.h"
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

void write_c_string(struct connection *c, const char *string) {
	write_varint(c, (long)strlen(string));
	fputs(string, c->outbound_packet.stream);
}

void write_c_string_len(struct connection *c, const char *string, size_t len) {
	write_varint(c, (long)len);
	fputs(string, c->outbound_packet.stream);
}

void write_string(struct connection *c, struct t_string string) {
	write_varint(c, (long)string.len);
	fwrite(string.data, 1, string.len, c->outbound_packet.stream);
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
