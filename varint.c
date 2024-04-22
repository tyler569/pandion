#include "minecraft.h"
#include <openssl/bio.h>
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

long read_varint_from_bio(BIO *bio) {
	long value = 0;
	int shift = 0;
	int rc;
	char byte;

	do {
		rc = BIO_read(bio, &byte, 1);
		if (rc <= 0) {
			return value;
		}
		value |= (byte & 0x7F) << shift;
		shift += 7;
	} while (byte & 0x80);
	return value;
}

void write_varint_to_stream(FILE *stream, long v) {
	unsigned char byte;
	unsigned long value = (unsigned long)v;
	do {
		byte = value & 0x7F;
		value >>= 7;
		if (value) {
			byte |= 0x80;
		}
		fputc(byte, stream);
	} while (value);
}

void write_varint_to_bio(BIO *bio, long v) {
	unsigned char byte;
	unsigned long value = (unsigned long)v;
	do {
		byte = value & 0x7F;
		value >>= 7;
		if (value) {
			byte |= 0x80;
		}
		BIO_write(bio, &byte, 1);
	} while (value);
}

long read_varint(struct connection *c) {
	return read_varint_from_stream(c->inbound_packet.stream);
}

void write_varint(struct connection *c, long value) {
	write_varint_to_stream(c->outbound_packet.stream, value);
}

long read_varint_from_buffer(void *data, size_t len) {
	long value = 0;
	int shift = 0;
	int byte;
	size_t i = 0;
	do {
		if (i >= len) {
			return -1;
		}
		byte = ((unsigned char *)data)[i++];
		value |= (byte & 0x7F) << shift;
		shift += 7;
	} while (byte & 0x80);
	return value;
}