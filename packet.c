#include "minecraft.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct packet *read_packet(FILE *file) {
    struct packet *packet = malloc(sizeof(struct packet));

    packet->len = read_varint_from_stream(file);
    if (feof(file)) {
        free(packet);
        return nullptr;
    }

    packet->data = malloc(packet->len);
    fread(packet->data, 1, packet->len, file);
    packet->reader_writer = fmemopen(packet->data, packet->len, "r");

    return packet;
}

void write_packet(struct connection *c, struct packet *packet) {
    fclose(packet->reader_writer);

    write_varint_to_stream(c->socket, (long)packet->len);
    fwrite(packet->data, 1, packet->len, c->socket);
}

struct packet *new_packet() {
    struct packet *packet = malloc(sizeof(struct packet));

    packet->len = 0;
    packet->data = nullptr;
    packet->reader_writer = open_memstream((char **)&packet->data, &packet->len);

    return packet;
}

void free_packet(struct packet *packet) {
    free(packet->data);
    free(packet);
}

struct string read_string(struct packet *packet) {
    struct string string;

    string.len = read_varint(packet);
    string.data = malloc(string.len + 1);
    fread(string.data, 1, string.len, packet->reader_writer);
    string.data[string.len] = '\0';

    return string;
}

short read_short(struct packet *packet) {
    unsigned char buf[2];
    fread(buf, 1, 2, packet->reader_writer);
    return (short)(((int)buf[0] << 8) | (int)buf[1]);
}

int read_int(struct packet *packet) {
    unsigned char buf[4];
    fread(buf, 1, 4, packet->reader_writer);
    return ((int)buf[0] << 24) | ((int)buf[1] << 16) | ((int)buf[2] << 8) | (int)buf[3];
}

long read_long(struct packet *packet) {
    unsigned char buf[8];
    fread(buf, 1, 8, packet->reader_writer);
    return ((long)buf[0] << 56) | ((long)buf[1] << 48) | ((long)buf[2] << 40) | ((long)buf[3] << 32) |
        ((long)buf[4] << 24) | ((long)buf[5] << 16) | ((long)buf[6] << 8) | (long)buf[7];
}

void write_c_string(struct packet *packet, const char *string) {
    write_varint(packet, (long)strlen(string));
    fputs(string, packet->reader_writer);
}

void write_c_string_len(struct packet *packet, const char *string, size_t len) {
    write_varint(packet, (long)len);
    fputs(string, packet->reader_writer);
}

void write_string(struct packet *packet, struct string string) {
    write_varint(packet, (long)string.len);
    fwrite(string.data, 1, string.len, packet->reader_writer);
}

void write_short(struct packet *packet, short value) {
    value = htons(value);
    fwrite(&value, 1, 2, packet->reader_writer);
}

void write_int(struct packet *packet, int value) {
    fputc(value >> 24, packet->reader_writer);
    fputc(value >> 16, packet->reader_writer);
    fputc(value >> 8, packet->reader_writer);
    fputc(value, packet->reader_writer);
}

void write_long(struct packet *packet, long value) {
    fputc((char)(value >> 56), packet->reader_writer);
    fputc((char)(value >> 48), packet->reader_writer);
    fputc((char)(value >> 40), packet->reader_writer);
    fputc((char)(value >> 32), packet->reader_writer);
    fputc((char)(value >> 24), packet->reader_writer);
    fputc((char)(value >> 16), packet->reader_writer);
    fputc((char)(value >> 8), packet->reader_writer);
    fputc((char)value, packet->reader_writer);
}
