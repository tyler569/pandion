#pragma once

#include "list.h"
#include <stdint.h>
#include <stdio.h>

enum nbt_type {
	NBT_END = 0,
	NBT_BYTE = 1,
	NBT_SHORT = 2,
	NBT_INT = 3,
	NBT_LONG = 4,
	NBT_FLOAT = 5,
	NBT_DOUBLE = 6,
	NBT_BYTE_ARRAY = 7,
	NBT_STRING = 8,
	NBT_LIST = 9,
	NBT_COMPOUND = 10,
	NBT_INT_ARRAY = 11,
	NBT_LONG_ARRAY = 12
};

typedef list(struct nbt_tag *) nbt_list;

struct nbt_tag {
	enum nbt_type type;
	const char *name;

	union {
		char byte;
		short short_;
		int int_;
		long long_;
		float float_;
		double double_;
		struct {
			int len;
			char *data;
		} byte_array;
		const char *string;
		struct {
			nbt_list data;
			enum nbt_type type;
		} list;
		nbt_list compound;
		struct {
			int len;
			int *data;
		} int_array;
		struct {
			int len;
			long *data;
		} long_array;
	};
};

struct nbt_tag *nbt_new_tag(enum nbt_type type);
struct nbt_tag *nbt_new_byte(char value);
struct nbt_tag *nbt_new_short(short value);
struct nbt_tag *nbt_new_int(int value);
struct nbt_tag *nbt_new_long(long value);
struct nbt_tag *nbt_new_float(float value);
struct nbt_tag *nbt_new_double(double value);
struct nbt_tag *nbt_new_byte_array(int len, char *data);
struct nbt_tag *nbt_new_string(const char *string);
struct nbt_tag *nbt_new_list(enum nbt_type type);
struct nbt_tag *nbt_new_compound();
struct nbt_tag *nbt_new_int_array(int *data, int len);
struct nbt_tag *nbt_new_long_array(long *data, int len);

void nbt_add_to_list(struct nbt_tag *list, struct nbt_tag *tag);
void nbt_add_to_compound(
	struct nbt_tag *compound, const char *name, struct nbt_tag *tag);

void nbt_add_byte_to_list(struct nbt_tag *list, char value);
void nbt_add_short_to_list(struct nbt_tag *list, short value);
void nbt_add_int_to_list(struct nbt_tag *list, int value);
void nbt_add_long_to_list(struct nbt_tag *list, long value);
void nbt_add_float_to_list(struct nbt_tag *list, float value);
void nbt_add_double_to_list(struct nbt_tag *list, double value);
void nbt_add_byte_array_to_list(struct nbt_tag *list, int len, char *data);
void nbt_add_string_to_list(struct nbt_tag *list, const char *string);
void nbt_add_int_array_to_list(struct nbt_tag *list, int len, int *data);
void nbt_add_long_array_to_list(struct nbt_tag *list, int len, long *data);
void nbt_add_list_to_list(struct nbt_tag *list, struct nbt_tag *list_);
void nbt_add_compound_to_list(struct nbt_tag *list, struct nbt_tag *compound);

void nbt_add_byte_to_compound(
	struct nbt_tag *compound, const char *name, char value);
void nbt_add_short_to_compound(
	struct nbt_tag *compound, const char *name, short value);
void nbt_add_int_to_compound(
	struct nbt_tag *compound, const char *name, int value);
void nbt_add_long_to_compound(
	struct nbt_tag *compound, const char *name, long value);
void nbt_add_float_to_compound(
	struct nbt_tag *compound, const char *name, float value);
void nbt_add_double_to_compound(
	struct nbt_tag *compound, const char *name, double value);
void nbt_add_byte_array_to_compound(
	struct nbt_tag *compound, const char *name, int len, char *data);
void nbt_add_string_to_compound(
	struct nbt_tag *compound, const char *name, const char *string);
void nbt_add_int_array_to_compound(
	struct nbt_tag *compound, const char *name, int len, int *data);
void nbt_add_long_array_to_compound(
	struct nbt_tag *compound, const char *name, int len, long *data);
void nbt_add_list_to_compound(
	struct nbt_tag *compound, const char *name, struct nbt_tag *list);
void nbt_add_compound_to_compound(
	struct nbt_tag *compound, const char *name, struct nbt_tag *compound_);

void nbt_print(struct nbt_tag *tag);

void nbt_free(struct nbt_tag *tag);

void nbt_write_to_stream(struct nbt_tag *tag, FILE *stream);
