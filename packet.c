#include "minecraft.h"
#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pn_error_t read_inbound_packet(struct connection *c) {
	long len = read_varint_from_stream(c->socket);

	if (feof(c->socket) || ferror(c->socket))
		return pn_eof;

	if (len == 0)
		return pn_invalid_packet;

	if (len > 0) {
		c->inbound_packet.len = len;
		void *data = realloc(c->inbound_packet.data, len);

		if (!data)
			return pn_oom;

		c->inbound_packet.data = data;

		fread(c->inbound_packet.data, 1, len, c->socket);
		if (feof(c->socket) || ferror(c->socket))
			return pn_eof;

		c->inbound_packet.stream = fmemopen(c->inbound_packet.data, len, "r");
	}

	return pn_ok;
}

void end_inbound_packet(struct connection *c) {
	fclose(c->inbound_packet.stream);
}

void new_outbound_packet(struct connection *c, long id) {
	assert(c->outbound_packet.data == nullptr);

	c->outbound_packet.len = 0;
	c->outbound_packet.stream = open_memstream(
		(char **)&c->outbound_packet.data, &c->outbound_packet.len);

	write_varint_to_stream(c->outbound_packet.stream, (long)id);
}

pn_error_t send_outbound_packet(struct connection *c) {
	fclose(c->outbound_packet.stream);

	assert(c->outbound_packet.data != nullptr);
	assert(c->outbound_packet.len > 0);
	assert(!feof(c->outbound_packet.stream));
	assert(!ferror(c->outbound_packet.stream));

	write_varint_to_stream(c->socket, (long)c->outbound_packet.len);
	fwrite(c->outbound_packet.data, 1, c->outbound_packet.len, c->socket);

	if (feof(c->socket) || ferror(c->socket))
		return pn_eof;

	free(c->outbound_packet.data);
	c->outbound_packet.data = nullptr;

	return pn_ok;
}

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
