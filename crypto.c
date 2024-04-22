#include "minecraft.h"
#include <assert.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
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

pn_error_t decrypt_data_rsa(struct connection *c, unsigned char *out,
	size_t out_len, struct t_string *in) {

	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(c->server->server_key, nullptr);
	assert(ctx);

	assert(EVP_PKEY_decrypt_init(ctx) > 0);

	unsigned char decrypt_buffer[128];
	size_t decrypt_len = sizeof(decrypt_buffer);

	int err = EVP_PKEY_decrypt(
		ctx, decrypt_buffer, &decrypt_len, (unsigned char *)in->data, in->len);
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

void init_connection_streams(struct connection *c) {
	c->inbound_stream = BIO_new_fd(c->socket_fd, BIO_NOCLOSE);
	c->outbound_stream = BIO_new_fd(c->socket_fd, BIO_CLOSE);
}

void init_connection_aes(struct connection *c) {
	BIO *decrypt = BIO_new(BIO_f_cipher());

	BIO_set_cipher(
		decrypt, EVP_aes_128_cfb8(), c->shared_secret, c->shared_secret, 0);

	c->inbound_stream = BIO_push(decrypt, c->inbound_stream);

	BIO *encrypt = BIO_new(BIO_f_cipher());

	BIO_set_cipher(
		encrypt, EVP_aes_128_cfb8(), c->shared_secret, c->shared_secret, 1);

	c->outbound_stream = BIO_push(encrypt, c->outbound_stream);

	c->encryption_enabled = true;
}

void close_connection_streams(struct connection *c) {
	BIO_free_all(c->inbound_stream);
	BIO_free_all(c->outbound_stream);
}

void generate_random_bytes(unsigned char *buf, size_t len) {
	assert(RAND_bytes(buf, len) == 1);
}