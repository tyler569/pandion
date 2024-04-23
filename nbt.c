#include "nbt.h"
#include "list.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct nbt_tag *nbt_new_tag(enum nbt_type type) {
	struct nbt_tag *tag = malloc(sizeof(struct nbt_tag));
	tag->type = type;
	tag->name = NULL;
	return tag;
}

struct nbt_tag *nbt_new_byte(char value) {
	struct nbt_tag *tag = nbt_new_tag(NBT_BYTE);
	tag->byte = value;
	return tag;
}

struct nbt_tag *nbt_new_short(short value) {
	struct nbt_tag *tag = nbt_new_tag(NBT_SHORT);
	tag->short_ = value;
	return tag;
}

struct nbt_tag *nbt_new_int(int value) {
	struct nbt_tag *tag = nbt_new_tag(NBT_INT);
	tag->int_ = value;
	return tag;
}

struct nbt_tag *nbt_new_long(long value) {
	struct nbt_tag *tag = nbt_new_tag(NBT_LONG);
	tag->long_ = value;
	return tag;
}

struct nbt_tag *nbt_new_float(float value) {
	struct nbt_tag *tag = nbt_new_tag(NBT_FLOAT);
	tag->float_ = value;
	return tag;
}

struct nbt_tag *nbt_new_double(double value) {
	struct nbt_tag *tag = nbt_new_tag(NBT_DOUBLE);
	tag->double_ = value;
	return tag;
}

struct nbt_tag *nbt_new_byte_array(int len, char *data) {
	struct nbt_tag *tag = nbt_new_tag(NBT_BYTE_ARRAY);
	tag->byte_array.len = len;
	tag->byte_array.data = malloc(len);
	memcpy(tag->byte_array.data, data, len);
	return tag;
}

struct nbt_tag *nbt_new_string(const char *string) {
	struct nbt_tag *tag = nbt_new_tag(NBT_STRING);
	tag->string = strdup(string);
	return tag;
}

struct nbt_tag *nbt_new_list(enum nbt_type type) {
	struct nbt_tag *tag = nbt_new_tag(NBT_LIST);
	list_init(&tag->list.data);
	tag->list.type = type;
	return tag;
}

struct nbt_tag *nbt_new_compound() {
	struct nbt_tag *tag = nbt_new_tag(NBT_COMPOUND);
	list_init(&tag->compound);
	return tag;
}

struct nbt_tag *nbt_new_int_array(int *data, int len) {
	struct nbt_tag *tag = nbt_new_tag(NBT_INT_ARRAY);
	tag->int_array.len = len;
	tag->int_array.data = malloc(len * sizeof(int));
	memcpy(tag->int_array.data, data, len * sizeof(int));
	return tag;
}

struct nbt_tag *nbt_new_long_array(long *data, int len) {
	struct nbt_tag *tag = nbt_new_tag(NBT_LONG_ARRAY);
	tag->long_array.len = len;
	tag->long_array.data = malloc(len * sizeof(long));
	memcpy(tag->long_array.data, data, len * sizeof(long));
	return tag;
}

void nbt_add_to_list(struct nbt_tag *list, struct nbt_tag *tag) {
	assert(list->type == NBT_LIST);
	list_push(&list->list.data, tag);
}

void nbt_add_to_compound(
	struct nbt_tag *compound, const char *name, struct nbt_tag *tag) {
	assert(compound->type == NBT_COMPOUND);
	tag->name = strdup(name);
	list_push(&compound->compound, tag);
}

void nbt_add_byte_to_list(struct nbt_tag *list, char value) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_BYTE);
	nbt_add_to_list(list, nbt_new_byte(value));
}

void nbt_add_short_to_list(struct nbt_tag *list, short value) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_SHORT);
	nbt_add_to_list(list, nbt_new_short(value));
}

void nbt_add_int_to_list(struct nbt_tag *list, int value) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_INT);
	nbt_add_to_list(list, nbt_new_int(value));
}

void nbt_add_long_to_list(struct nbt_tag *list, long value) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_LONG);
	nbt_add_to_list(list, nbt_new_long(value));
}

void nbt_add_float_to_list(struct nbt_tag *list, float value) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_FLOAT);
	nbt_add_to_list(list, nbt_new_float(value));
}

void nbt_add_double_to_list(struct nbt_tag *list, double value) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_DOUBLE);
	nbt_add_to_list(list, nbt_new_double(value));
}

void nbt_add_byte_array_to_list(struct nbt_tag *list, int len, char *data) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_BYTE_ARRAY);
	nbt_add_to_list(list, nbt_new_byte_array(len, data));
}

void nbt_add_string_to_list(struct nbt_tag *list, const char *string) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_STRING);
	nbt_add_to_list(list, nbt_new_string(string));
}

void nbt_add_int_array_to_list(struct nbt_tag *list, int len, int *data) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_INT_ARRAY);
	nbt_add_to_list(list, nbt_new_int_array(data, len));
}

void nbt_add_long_array_to_list(struct nbt_tag *list, int len, long *data) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_LONG_ARRAY);
	nbt_add_to_list(list, nbt_new_long_array(data, len));
}

void nbt_add_list_to_list(struct nbt_tag *list, struct nbt_tag *nested_list) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_LIST);
	nbt_add_to_list(list, nested_list);
}

void nbt_add_compound_to_list(
	struct nbt_tag *list, struct nbt_tag *nested_compound) {
	assert(list->type == NBT_LIST);
	assert(list->list.type == NBT_COMPOUND);
	nbt_add_to_list(list, nested_compound);
}

void nbt_add_byte_to_compound(
	struct nbt_tag *compound, const char *name, char value) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, nbt_new_byte(value));
}

void nbt_add_short_to_compound(
	struct nbt_tag *compound, const char *name, short value) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, nbt_new_short(value));
}

void nbt_add_int_to_compound(
	struct nbt_tag *compound, const char *name, int value) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, nbt_new_int(value));
}

void nbt_add_long_to_compound(
	struct nbt_tag *compound, const char *name, long value) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, nbt_new_long(value));
}

void nbt_add_float_to_compound(
	struct nbt_tag *compound, const char *name, float value) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, nbt_new_float(value));
}

void nbt_add_double_to_compound(
	struct nbt_tag *compound, const char *name, double value) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, nbt_new_double(value));
}

void nbt_add_byte_array_to_compound(
	struct nbt_tag *compound, const char *name, int len, char *data) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, nbt_new_byte_array(len, data));
}

void nbt_add_string_to_compound(
	struct nbt_tag *compound, const char *name, const char *string) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, nbt_new_string(string));
}

void nbt_add_int_array_to_compound(
	struct nbt_tag *compound, const char *name, int len, int *data) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, nbt_new_int_array(data, len));
}

void nbt_add_long_array_to_compound(
	struct nbt_tag *compound, const char *name, int len, long *data) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, nbt_new_long_array(data, len));
}

void nbt_add_list_to_compound(
	struct nbt_tag *compound, const char *name, struct nbt_tag *list) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, list);
}

void nbt_add_compound_to_compound(struct nbt_tag *compound, const char *name,
	struct nbt_tag *nested_compound) {
	assert(compound->type == NBT_COMPOUND);
	nbt_add_to_compound(compound, name, nested_compound);
}

void nbt_print_rec(struct nbt_tag *tag, int depth);

void nbt_print(struct nbt_tag *tag) { nbt_print_rec(tag, 0); }

void nbt_print_rec(struct nbt_tag *tag, int depth) {
	for (int i = 0; i < depth; i++) {
		printf("  ");
	}
	if (tag->name) {
		printf("%s: ", tag->name);
	}
	switch (tag->type) {
	case NBT_BYTE:
		printf("%db\n", tag->byte);
		break;
	case NBT_SHORT:
		printf("%ds\n", tag->short_);
		break;
	case NBT_INT:
		printf("%d\n", tag->int_);
		break;
	case NBT_LONG:
		printf("%ldl\n", tag->long_);
		break;
	case NBT_FLOAT:
		printf("%ff\n", tag->float_);
		break;
	case NBT_DOUBLE:
		printf("%f\n", tag->double_);
		break;
	case NBT_BYTE_ARRAY:
		printf("[");
		for (int i = 0; i < tag->byte_array.len; i++) {
			printf("%db ", tag->byte_array.data[i]);
		}
		printf("]\n");
		break;
	case NBT_STRING:
		printf("\"%s\"\n", tag->string);
		break;
	case NBT_LIST:
		printf("List: [\n");
		for_each(&tag->list.data) { nbt_print_rec(*it, depth + 1); }
		for (int i = 0; i < depth; i++) {
			printf("  ");
		}
		printf("]\n");
		break;
	case NBT_COMPOUND:
		printf("Compound: {\n");
		for_each(&tag->compound) { nbt_print_rec(*it, depth + 1); }
		for (int i = 0; i < depth; i++) {
			printf("  ");
		}
		printf("}\n");
		break;
	case NBT_INT_ARRAY:
		printf("[");
		for (int i = 0; i < tag->int_array.len; i++) {
			printf("%d ", tag->int_array.data[i]);
		}
		printf("]\n");
		break;
	case NBT_LONG_ARRAY:
		printf("[");
		for (int i = 0; i < tag->long_array.len; i++) {
			printf("%ldl ", tag->long_array.data[i]);
		}
		printf("]\n");
		break;
	case NBT_END:
		printf("End\n");
		break;
	}
}

void nbt_write_tag_name_to_stream(struct nbt_tag *tag, FILE *stream);
void nbt_write_tag_data_to_stream(struct nbt_tag *tag, FILE *stream);

void nbt_write_to_stream(struct nbt_tag *tag, FILE *stream) {
	fputc(tag->type, stream);
	nbt_write_tag_name_to_stream(tag, stream);
	nbt_write_tag_data_to_stream(tag, stream);
}

void nbt_write_tag_name_to_stream(struct nbt_tag *tag, FILE *stream) {
	if (tag->type == NBT_END) {
		return;
	}
	if (tag->name) {
		uint16_t len = strlen(tag->name);
		fputc(len >> 8, stream);
		fputc(len, stream);
		fwrite(tag->name, sizeof(char), len, stream);
	} else {
		fputc(0, stream);
		fputc(0, stream);
	}
}

static void nbt_write_be_short_to_stream(short value, FILE *stream) {
	fputc(value >> 8, stream);
	fputc(value, stream);
}

static void nbt_write_be_int_to_stream(int value, FILE *stream) {
	fputc(value >> 24, stream);
	fputc(value >> 16, stream);
	fputc(value >> 8, stream);
	fputc(value, stream);
}

static void nbt_write_be_long_to_stream(long value, FILE *stream) {
	fputc((int)(value >> 56), stream);
	fputc((int)(value >> 48), stream);
	fputc((int)(value >> 40), stream);
	fputc((int)(value >> 32), stream);
	fputc((int)(value >> 24), stream);
	fputc((int)(value >> 16), stream);
	fputc((int)(value >> 8), stream);
	fputc((int)value, stream);
}

void nbt_write_tag_data_to_stream(struct nbt_tag *tag, FILE *stream) {
	switch (tag->type) {
	case NBT_BYTE:
		fputc(tag->byte, stream);
		break;
	case NBT_SHORT:
		nbt_write_be_short_to_stream(tag->short_, stream);
		break;
	case NBT_INT:
		nbt_write_be_int_to_stream(tag->int_, stream);
		break;
	case NBT_LONG:
		nbt_write_be_long_to_stream(tag->long_, stream);
		break;
	case NBT_FLOAT: {
		int float_bits = *(int *)&tag->float_;
		nbt_write_be_int_to_stream(float_bits, stream);
		break;
	}
	case NBT_DOUBLE: {
		long double_bits = *(long *)&tag->double_;
		nbt_write_be_long_to_stream(double_bits, stream);
		break;
	}
	case NBT_BYTE_ARRAY:
		nbt_write_be_int_to_stream(tag->byte_array.len, stream);
		fwrite(tag->byte_array.data, 1, tag->byte_array.len, stream);
		break;
	case NBT_STRING: {
		short len = (short)strlen(tag->string);
		nbt_write_be_short_to_stream(len, stream);
		fwrite(tag->string, sizeof(char), len, stream);
		break;
	}
	case NBT_LIST: {
		fwrite(&tag->list.type, sizeof(char), 1, stream);
		nbt_write_be_int_to_stream(list_length(&tag->list.data), stream);

		for_each(&tag->list.data) { nbt_write_tag_data_to_stream(*it, stream); }
		break;
	}
	case NBT_COMPOUND:
		for_each(&tag->compound) { nbt_write_to_stream(*it, stream); }
		fputc(NBT_END, stream);
		break;
	case NBT_INT_ARRAY:
		nbt_write_be_int_to_stream(tag->int_array.len, stream);
		for (int i = 0; i < tag->int_array.len; i++) {
			nbt_write_be_int_to_stream(tag->int_array.data[i], stream);
		}
		break;
	case NBT_LONG_ARRAY:
		nbt_write_be_int_to_stream(tag->long_array.len, stream);
		for (int i = 0; i < tag->long_array.len; i++) {
			nbt_write_be_long_to_stream(tag->long_array.data[i], stream);
		}
		break;
	case NBT_END:
		break;
	}
}

void nbt_free(struct nbt_tag *tag) {
	if (tag->name) {
		free((void *)tag->name);
	}
	switch (tag->type) {
	case NBT_BYTE_ARRAY:
		free(tag->byte_array.data);
		break;
	case NBT_STRING:
		free((void *)tag->string);
		break;
	case NBT_LIST:
		for_each(&tag->list.data) { nbt_free(*it); }
		list_free(&tag->list.data);
		break;
	case NBT_COMPOUND:
		for_each(&tag->compound) { nbt_free(*it); }
		list_free(&tag->compound);
		break;
	case NBT_INT_ARRAY:
		free(tag->int_array.data);
		break;
	case NBT_LONG_ARRAY:
		free(tag->long_array.data);
		break;
	default:
		break;
	}
	free(tag);
}
