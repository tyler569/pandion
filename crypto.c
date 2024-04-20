#include "minecraft.h"
#include <assert.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/x509.h>

void init_server_crypto(struct server *s) {
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
	assert(ctx);

	assert(EVP_PKEY_keygen_init(ctx) > 0);
	assert(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 1024) > 0);
	assert(EVP_PKEY_keygen(ctx, &s->server_key) > 0);

	EVP_PKEY_CTX_free(ctx);

	BIO *bio = BIO_new(BIO_s_mem());
	assert(bio);

	assert(i2d_PUBKEY_bio(bio, s->server_key) > 0);

	BUF_MEM *buf_ptr;
	BIO_get_mem_ptr(bio, &buf_ptr);

	s->der_public_key = (unsigned char *)malloc(buf_ptr->length);
	assert(s->der_public_key);
	memcpy(s->der_public_key, buf_ptr->data, buf_ptr->length);
	s->der_public_key_len = buf_ptr->length;

	BIO_free(bio);
}

pn_error_t rsa_decrypt_data(struct connection *c, unsigned char *out,
	size_t out_len, struct t_string *in) {

	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(c->server->server_key, nullptr);
	assert(ctx);

	assert(EVP_PKEY_decrypt_init(ctx) > 0);

	unsigned char decrypt_buffer[128];
	size_t decrypt_len = sizeof(decrypt_buffer);

	int err = EVP_PKEY_decrypt(ctx, decrypt_buffer, &decrypt_len, (unsigned char *)in->data, in->len);
	if (err <= 0) {
		ERR_print_errors_fp(stderr);
		return pn_invalid_packet;
	}

	EVP_PKEY_CTX_free(ctx);

	if (decrypt_len != out_len) {
		printf("decrypt length mismatch\n");
		return pn_invalid_packet;
	}

	memcpy(out, decrypt_buffer, decrypt_len);

	return pn_ok;
}