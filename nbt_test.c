#include "nbt.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void hexdump(void *data, size_t len);

void nbt_assert_eq(struct nbt_tag *nbt, const char *expected, size_t len);
bool nbt_open_test_file(const char *filename, char **data, size_t *len);

int main() {
	struct nbt_tag *tag = nbt_new_short(32767);
	tag->name = "shortTest";
	nbt_assert_eq(tag, "\x02\x00\x09shortTest\x7f\xff", 14);
	free(tag);

	tag = nbt_new_int(2147483647);
	tag->name = "intTest";
	nbt_assert_eq(tag, "\x03\x00\x07intTest\x7f\xff\xff\xff", 14);
	free(tag);

	tag = nbt_new_compound();
	tag->name = "compound";
	nbt_assert_eq(tag, "\x0a\x00\x{08}compound\x00", 12);
	free(tag);

	tag = nbt_new_byte_array(3, (char[]) { 1, 2, 3 });
	tag->name = "byteArrayTest";
	nbt_assert_eq(
		tag, "\x07\x00\x{0d}byteArrayTest\x00\x00\x00\x03\x01\x02\x03", 23);
	free(tag);

	char *nbt_data;
	size_t nbt_len;

	{ // "Hello world" example
		tag = nbt_new_compound();
		tag->name = strdup("hello world");
		nbt_add_string_to_compound(tag, "name", "Bananrama");

		if (nbt_open_test_file(
				"nbt_test_data/hello_world.nbt", &nbt_data, &nbt_len)) {
			nbt_assert_eq(tag, nbt_data, nbt_len);
			free(nbt_data);
		} else {
			printf("Failed to open test file\n");
		}

		nbt_free(tag);
	}

	{ // "bigtest' example
		tag = nbt_new_compound();
		tag->name = strdup("Level");

		nbt_add_long_to_compound(tag, "longTest", 9223372036854775807);
		nbt_add_short_to_compound(tag, "shortTest", 32767);
		nbt_add_string_to_compound(tag, "stringTest",
			"HELLO WORLD THIS IS A TEST STRING \xc3\x85\xc3\x84\xc3\x96!");
		nbt_add_float_to_compound(tag, "floatTest", 0.49823147058486938f);
		nbt_add_int_to_compound(tag, "intTest", 2147483647);

		{
			struct nbt_tag *nested_compound = nbt_new_compound();

			struct nbt_tag *ham_compound = nbt_new_compound();
			nbt_add_string_to_compound(ham_compound, "name", "Hampus");
			nbt_add_float_to_compound(ham_compound, "value", 0.75f);
			nbt_add_compound_to_compound(nested_compound, "ham", ham_compound);

			struct nbt_tag *egg_compound = nbt_new_compound();
			nbt_add_string_to_compound(egg_compound, "name", "Eggbert");
			nbt_add_float_to_compound(egg_compound, "value", 0.5f);
			nbt_add_compound_to_compound(nested_compound, "egg", egg_compound);

			nbt_add_compound_to_compound(
				tag, "nested compound test", nested_compound);
		}

		{
			struct nbt_tag *long_list = nbt_new_list(NBT_LONG);

			nbt_add_long_to_list(long_list, 11);
			nbt_add_long_to_list(long_list, 12);
			nbt_add_long_to_list(long_list, 13);
			nbt_add_long_to_list(long_list, 14);
			nbt_add_long_to_list(long_list, 15);

			nbt_add_list_to_compound(tag, "listTest (long)", long_list);
		}

		{
			struct nbt_tag *compound_list = nbt_new_list(NBT_COMPOUND);

			struct nbt_tag *compound0 = nbt_new_compound();
			nbt_add_string_to_compound(compound0, "name", "Compound tag #0");
			nbt_add_long_to_compound(compound0, "created-on", 1264099775885);
			nbt_add_compound_to_list(compound_list, compound0);

			struct nbt_tag *compound1 = nbt_new_compound();
			nbt_add_string_to_compound(compound1, "name", "Compound tag #1");
			nbt_add_long_to_compound(compound1, "created-on", 1264099775885);
			nbt_add_compound_to_list(compound_list, compound1);

			nbt_add_list_to_compound(tag, "listTest (compound)", compound_list);
		}

		nbt_add_byte_to_compound(tag, "byteTest", 127);

		char byte_array[1000];
		for (int i = 0; i < 1000; i++) {
			byte_array[i] = (char)((i * i * 255 + i * 7) % 100);
		}
		nbt_add_byte_array_to_compound(tag,
			"byteArrayTest (the first 1000 values of (n*n*255+n*7)%100, "
		    "starting with n=0 (0, 62, 34, 16, 8, ...))",
			1000, byte_array);

		nbt_add_double_to_compound(tag, "doubleTest", 0.49312871321823148);

		nbt_print(tag);

		if (nbt_open_test_file(
				"nbt_test_data/bigtest.nbt", &nbt_data, &nbt_len)) {
			nbt_assert_eq(tag, nbt_data, nbt_len);
			free(nbt_data);
		} else {
			printf("Failed to open test file\n");
		}

		nbt_free(tag);
	}
}

void nbt_assert_eq(struct nbt_tag *nbt, const char *expected, size_t len) {
	unsigned char *u_expected = (unsigned char *)expected;

	size_t nbt_len = 0;
	unsigned char *nbt_data = nullptr;
	FILE *stream = open_memstream((char **)&nbt_data, &nbt_len);
	nbt_write_to_stream(nbt, stream);
	fclose(stream);

	if (nbt_len != len) {
		printf("Expected length: %zu, got: %zu\n", len, nbt_len);

		hexdump(nbt_data, nbt_len);
		printf("\n");
		hexdump(u_expected, len);

		assert(0);
	}

	for (size_t i = 0; i < len; i++) {
		if (nbt_data[i] != u_expected[i]) {
			printf(
				"Expected: %02hhx, got: %02hhx\n", u_expected[i], nbt_data[i]);

			hexdump(nbt_data, nbt_len);
			assert(0);
		}
	}

	free(nbt_data);
}

bool nbt_open_test_file(const char *filename, char **data, size_t *len) {
	FILE *file = fopen(filename, "rb");
	if (!file) {
		return false;
	}

	fseek(file, 0, SEEK_END);
	*len = ftell(file);
	fseek(file, 0, SEEK_SET);
	*data = malloc(*len);
	fread(*data, 1, *len, file);
	fclose(file);

	return true;
}
