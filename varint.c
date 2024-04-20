#include "minecraft.h"
#include <stdio.h>

long read_varint_from_stream(FILE *stream) {
	long value = 0;
	int shift = 0;
	int byte;
	do {
		byte = fgetc(stream);
		if (byte == EOF) {
			return value;
		}
		value |= (byte & 0x7F) << shift;
		shift += 7;
	} while (byte & 0x80);
	return value;
}

void write_varint_to_stream(FILE *stream, long value) {
	unsigned char byte;
	do {
		byte = value & 0x7F;
		value >>= 7;
		if (value) {
			byte |= 0x80;
		}
		fputc(byte, stream);
	} while (value);
}

long read_varint(struct connection *c) {
	return read_varint_from_stream(c->inbound_packet.stream);
}

void write_varint(struct connection *c, long value) {
	write_varint_to_stream(c->outbound_packet.stream, value);
}