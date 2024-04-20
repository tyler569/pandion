#include <stdio.h>
#include "minecraft.h"

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

long read_varint(struct packet *packet) {
    return read_varint_from_stream(packet->reader_writer);
}

void write_varint(struct packet *packet, long value) {
    write_varint_to_stream(packet->reader_writer, value);
}