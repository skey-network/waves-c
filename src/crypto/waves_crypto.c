#include <assert.h>
#include <openssl/rand.h>
#include <keygen.h>

#include "crypto.h"
#include "blake2b/sse/blake2.h"
#include "sha256.h"
#include "sha3.h"
#include "sha256.h"
#include "libcurve25519-donna/additions/curve_sigs.h"

void waves_secure_hash(const uint8_t *message, size_t message_len, uint8_t hash[32])
{
    blake2b_state S[1];
    sha3_context c;

    blake2b_init(S, 32);
    blake2b_update(S, message, message_len);
    blake2b_final(S, hash, 32);

    sha3_Init256(&c);
    sha3_Update(&c, hash, 32);
    sha3_Finalize(&c);

    memcpy(hash, c.sb, 32);
}


void waves_gen_private_key(curve25519_secret_key privkey, const unsigned char *seed)
{
    char realkey[1024] = {0, 0, 0, 0};
    memcpy(&realkey[4], seed, strlen((const char *) seed));

    SHA256_CTX ctx;

    waves_secure_hash((uint8_t*)realkey, strlen((const char *) seed) + 4, privkey);

    sha256_init(&ctx);
    sha256_update(&ctx, privkey, 32);
    sha256_final(&ctx, privkey);

    privkey[0] &= 248;
    privkey[31] &= 127;
    privkey[31] |= 64;
}

void waves_gen_public_key(curve25519_public_key pubkey, curve25519_secret_key privkey)
{
    curve25519_keygen(pubkey, privkey);
}

bool waves_sign_message(const curve25519_secret_key private_key, const unsigned char *message, const size_t message_size,
                            curve25519_signature signature) {
    unsigned char random[64];
    RAND_bytes(random, 64);
    return waves_sign_message_custom_random(private_key, message, message_size, signature, random);
}

bool waves_sign_message_custom_random(const curve25519_secret_key private_key, const unsigned char *message,
                                      const size_t message_size, curve25519_signature signature, const unsigned char *random64) {
    return curve25519_sign(signature, (const unsigned char *) private_key, message, message_size, random64) == 0;
}

bool waves_verify_message(const curve25519_public_key public_key, const unsigned char *message, const size_t message_size, const curve25519_signature signature) {
    return curve25519_verify((const unsigned char *)signature, (const unsigned char *)public_key, message, message_size) == 0;
}

// todo move all that stuff to crypto module
// Build waves address from the curve25519 public key, check https://docs.wavesplatform.com/en/waves-environment/waves-protocol/data-structures.html#section-884d9804999fc47a3c2694e49ad2536a
void waves_public_key_to_address(const curve25519_public_key public_key, const unsigned char network_byte, unsigned char address[26])
{
    uint8_t public_key_hash[32];

    uint8_t checksum[32];
    waves_secure_hash(public_key, 32, public_key_hash);

    address[0] = 0x01;
    address[1] = network_byte;
    memmove(&address[2], public_key_hash, 20);

    waves_secure_hash(address, 22, checksum);

    memmove(&address[22], checksum, 4);
}

#if 0
void waves_b58_seed_to_address(const unsigned char *key, const unsigned char network_byte, unsigned char *output)
{
    uint8_t address[26];
    size_t length = 36;
    waves_bin_seed_to_address(key, network_byte, address);
    b58enc((char *) output, &length, address, 26);
}
#endif

unsigned char *waves_sha256(const unsigned char *src, unsigned int src_len, unsigned char* dst)
{
    SHA256_CTX ctx;

    assert(src);
    assert(src_len);
    assert(dst);

    sha256_init(&ctx);
    sha256_update(&ctx, src, src_len);
    sha256_final(&ctx, dst);

    return dst;
}
