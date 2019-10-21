#include <printf.h>
#include <stdlib.h>
#include <stdio.h>

#include "crypto.h"
#include "b58.h"
#include "libcurve25519-donna/additions/keygen.h"
#include "utils.h"

#include "crypto_tests.h"

void waves_secure_hash_test() {
    uint8_t input[] = "A nice, long test to make the day great! :-)";
    uint8_t output[32];
    uint8_t expected[] = {0x5d, 0xf3, 0xcf, 0x20, 0x20, 0x5d, 0x75, 0xe0, 0x9a, 0xe4, 0x6d, 0x13, 0xa8, 0xd9, 0x9a, 0x16, 0x17, 0x4d, 0x71, 0xc8, 0x4f, 0xfc, 0xc0, 0x03, 0x87, 0xfe, 0xc3, 0xd8, 0x1e, 0x39, 0xdc, 0xbe};
    waves_secure_hash(input, strlen((char*)input), (uint8_t*)output);
    if(memcmp(output, expected, 32) != 0) {
        printf("waves_secure_hash test failed\n");
        exit(-1);
    }
}

void waves_public_key_to_address_testnet_test() {
    uint8_t input[] = {0xd8, 0x5b, 0x2f, 0x9e, 0x00, 0xde, 0xa8, 0x88, 0x65, 0x55, 0x3b, 0x6f, 0x69, 0xda, 0x53, 0x18, 0xbe, 0x64, 0x4f, 0x4d, 0x39, 0xa9, 0xc4, 0x8e, 0xba, 0xed, 0x71, 0x46, 0xcb, 0x7a, 0xfb, 0x73};
    unsigned char output[512];
    unsigned char expected[] = "3MxsTK7Sv9kfKT1iUS8vChgJkvoowpNXDJU";
    unsigned char address_bin[26];
    waves_public_key_to_address(input, 'T', address_bin);
    base58_encode(output, address_bin, sizeof(address_bin));
    if(strcmp((const char *) output, (const char *) expected) != 0) {
        printf("waves_public_key_to_address testnet test failed\n");
        exit(-1);
    }
}

void waves_public_key_to_address_mainnet_test() {
    uint8_t input[] = {0xdb, 0x3b, 0xe4, 0xbb, 0x58, 0x3e, 0x58, 0xe5, 0x7b, 0xae, 0xb2, 0xa7, 0xad, 0x40, 0x8f, 0x73, 0xb2, 0x04, 0xab, 0x26, 0xd6, 0x4c, 0x73, 0x0e, 0xbb, 0xe1, 0x4d, 0xd0, 0xaf, 0x33, 0xe8, 0x23};
    unsigned char output[512];
    unsigned char expected[] = "3P86pnxzWotqMsWyyoAvFgAUQHXcLNedasy";
    unsigned char address_bin[26];
    waves_public_key_to_address(input, 'W', address_bin);
    base58_encode(output, address_bin, sizeof(address_bin));
    if(strcmp((const char *) output, (const char *) expected) != 0) {
        printf("waves_public_key_to_address mainnet test failed\n");
        exit(-1);
    }
}

void curve25519_keygen_test() {
    uint8_t privkey[] = {0x88, 0x72, 0x7a, 0x03, 0x37, 0x7b, 0xfb, 0xa1, 0xb3, 0x65, 0x5c, 0x5e, 0xcb, 0x97, 0x8d, 0xa1, 0x71, 0xe0, 0x24, 0xaa, 0xd7, 0x22, 0xee, 0x49, 0xff, 0xf9, 0x21, 0x4a, 0x74, 0x7e, 0x70, 0x61};
    uint8_t expected[] = {0x92, 0xf2, 0xc1, 0x71, 0xcb, 0x60, 0x78, 0xe6, 0x05, 0x50, 0xcb, 0x99, 0x53, 0xfc, 0x3f, 0x11, 0x80, 0x31, 0xd6, 0x31, 0x4c, 0xb6, 0x40, 0x0d, 0xfd, 0x72, 0x11, 0xf6, 0x01, 0x8d, 0x1d, 0x2b};
    privkey[0] &= 248;
    privkey[31] &= 127;
    privkey[31] |= 64;

    uint8_t output[32];

    curve25519_keygen(output, privkey);

    if(memcmp(output, expected, 32) != 0) {
        printf("curve25519_keygen test failed\n");
        exit(-1);
    }
}

void waves_seed_to_address_testnet_test() {
    unsigned char test[] = "industry detail rifle scan weird join crawl connect demand top club hello entry second cargo";
    unsigned char output[512];
    unsigned char expected[] = "3NCyi16BFfFvYhCeg1pKrMKMLDXwazkPuhP";
    unsigned char address_bin[26];

    uint8_t priv_key[32];
    uint8_t pub_key[32];
    waves_gen_private_key(priv_key, (const unsigned char *)test);
    waves_gen_public_key(pub_key, priv_key);
    waves_public_key_to_address(pub_key, 'T', address_bin);
    base58_encode((char*)output, address_bin, sizeof(address_bin));
    base58_encode(output, address_bin, sizeof(address_bin));
    if(strcmp((const char *) output, (const char *) expected) != 0) {
        printf("waves_seed_to_address testnet test failed\n");
        exit(-1);
    }
}

void waves_seed_to_address_mainnet_test() {
    unsigned char test[] = "try south announce math salute shoe blast finish state battle nest tube enjoy yellow layer";
    unsigned char output[512];
    unsigned char expected[] = "3PJXLWbp5ft3LCeesqgJyTpGQRgU9nTY3PA";
    unsigned char address_bin[26];

    uint8_t priv_key[32];
    uint8_t pub_key[32];
    waves_gen_private_key(priv_key, test);
    waves_gen_public_key(pub_key, priv_key);
    waves_public_key_to_address(pub_key, 'W', address_bin);
    base58_encode((char*)output, address_bin, sizeof(address_bin));
    if(strcmp((const char *) output, (const char *) expected) != 0) {
        printf("waves_seed_to_address mainnet test failed\n");
        exit(-1);
    }
}

void waves_message_sign_without_random_test() {
    unsigned char expected[] = "5HRwSL8XGhSEbtLuBfZT1AkfKaybYA67aKTUpg3v47aTZfCLiBMDLj1P9PmoirWcNCVFCoja4gmv5nkjDnYAULus";

    uint8_t privkey[] = {0x88, 0x72, 0x7a, 0x03, 0x37, 0x7b, 0xfb, 0xa1, 0xb3, 0x65, 0x5c, 0x5e, 0xcb, 0x97, 0x8d, 0xa1, 0x71, 0xe0, 0x24, 0xaa, 0xd7, 0x22, 0xee, 0x49, 0xff, 0xf9, 0x21, 0x4a, 0x74, 0x7e, 0x70, 0x61};
    sc_clamp(privkey);

    uint8_t pubkey[32];

    curve25519_keygen(pubkey, privkey);

    uint8_t zero_random64[64];
    memset(zero_random64, 0, sizeof(zero_random64));

    uint8_t signature[64];

    unsigned char signature_base58[89];

    uint8_t message[] = {0x01, 0x02, 0x03, 0x04, 0x05};

    /* XXX Where b58enc() function comes from?! */
#if 0 && !defined(WAVES_DEBUG)
    char message_base58[89];
    size_t message_base58_size = sizeof(message_base58);

    char privkey_base58[45];
    size_t privkey_base58_size = sizeof(privkey_base58);

    char pubkey_base58[45];
    size_t pubkey_base58_size = sizeof(pubkey_base58);

    b58enc(pubkey_base58, &pubkey_base58_size, pubkey, 32);
    b58enc(privkey_base58, &privkey_base58_size, privkey, 32);
    b58enc(message_base58, &message_base58_size, message, 5);
#endif

    if(!waves_sign_message_custom_random(privkey, message, sizeof(message), signature, zero_random64)) {
        printf("waves_message_sign_without_random test failed\n");
        exit(-1);
    }

    base58_encode((char*)signature_base58, signature, 64);

    if(strcmp((const char*) signature_base58, (const char *) expected) != 0) {
        printf("waves_message_sign test failed 1\n");
        exit(-1);
    }
}

void waves_message_sign_test() {
    uint8_t privkey[] = {0x88, 0x72, 0x7a, 0x03, 0x37, 0x7b, 0xfb, 0xa1, 0xb3, 0x65, 0x5c, 0x5e, 0xcb, 0x97, 0x8d, 0xa1, 0x71, 0xe0, 0x24, 0xaa, 0xd7, 0x22, 0xee, 0x49, 0xff, 0xf9, 0x21, 0x4a, 0x74, 0x7e, 0x70, 0x61};
    sc_clamp(privkey);

    uint8_t pubkey[32];

    curve25519_keygen(pubkey, privkey);

    uint8_t signature[64];

    char signature_base58[89];

    uint8_t message[] = {0x01, 0x02, 0x03, 0x04, 0x05};

    if(!waves_sign_message(privkey, message, sizeof(message), signature)) {
        printf("waves_message_sign test failed\n");
        exit(-1);
    }

    base58_encode(signature_base58, signature, 64);

    if(!waves_verify_message(pubkey, message, sizeof(message), signature)) {
        printf("waves_message_sign test failed 1\n");
        exit(-1);
    }
}

void waves_message_verify_test() {
    uint8_t privkey[] = {0x88, 0x72, 0x7a, 0x03, 0x37, 0x7b, 0xfb, 0xa1, 0xb3, 0x65, 0x5c, 0x5e, 0xcb, 0x97, 0x8d, 0xa1, 0x71, 0xe0, 0x24, 0xaa, 0xd7, 0x22, 0xee, 0x49, 0xff, 0xf9, 0x21, 0x4a, 0x74, 0x7e, 0x70, 0x61};
    sc_clamp(privkey);

    uint8_t pubkey[32];

    curve25519_keygen(pubkey, privkey);

    uint8_t message[] = {0x01, 0x02, 0x03, 0x04, 0x05};

    char signature_base58[] = "5HRwSL8XGhSEbtLuBfZT1AkfKaybYA67aKTUpg3v47aTZfCLiBMDLj1P9PmoirWcNCVFCoja4gmv5nkjDnYAULus";
    uint8_t signature[64];

    base58_decode(signature, signature_base58);

    /* XXX Where b58enc() function comes from?! */
#if 0 && !defined(WAVES_DEBUG)
    char privkey_base58[45];
    size_t privkey_base58_size = sizeof(privkey_base58);

    char pubkey_base58[45];
    size_t pubkey_base58_size = sizeof(pubkey_base58);

    char signature_base582[89];
    size_t signature_base58_size = sizeof(signature_base582);

    b58enc(pubkey_base58, &pubkey_base58_size, pubkey, 32);
    b58enc(privkey_base58, &privkey_base58_size, privkey, 32);
    b58enc(signature_base582, &signature_base58_size, signature, 64);
#endif

    if(!waves_verify_message(pubkey, message, sizeof(message), signature)) {
        printf("waves_message_verify test failed\n");
        exit(-1);
    }
}

void waves_message_verify_negative_test() {
    uint8_t privkey[] = {0x88, 0x72, 0x7a, 0x03, 0x37, 0x7b, 0xfb, 0xa1, 0xb3, 0x65, 0x5c, 0x5e, 0xcb, 0x97, 0x8d, 0xa1, 0x71, 0xe0, 0x24, 0xaa, 0xd7, 0x22, 0xee, 0x49, 0xff, 0xf9, 0x21, 0x4a, 0x74, 0x7e, 0x70, 0x61};
    sc_clamp(privkey);

    uint8_t pubkey[32];

    curve25519_keygen(pubkey, privkey);

    uint8_t message[] = {0x01, 0x02, 0x03, 0x04, 0x05};

    char signature_base58[] = "5HRwSL8XGhSEbtLuBfZT1AkfKaybYA67aKTUpg3v47aTZfCLiBMDLj1P9PmoirWcNCVFCoja4gmv5nkjDnYAULus";

    uint8_t signature[64];

    base58_decode(signature, signature_base58);

    // oops
    signature[0] = '1';

    if(waves_verify_message(pubkey, message, sizeof(message), signature)) {
        printf("waves_message_verify_negative test failed\n");
        exit(-1);
    }
}

void waves_crypto_sha256_test()
{
    unsigned char data[] = "Test message";
    unsigned char res[32] = {0};
    const unsigned char* expected_res_hex = "c0719e9a8d5d838d861dc6f675c899d2b309a3a65bb9fe6b11e5afcbf9a2c0b1";
    unsigned char expected_res[32] = {0};
    int rc = 0;

    printf("%s\n", __func__);

    rc = hex2bin((char*)expected_res, (const char *)expected_res_hex);
    if (rc != 0) {
        fprintf(stderr, "hex2bin failed in %s with return code: %d\n", __func__, rc);
        exit(-1);
    }

    unsigned char* ret = waves_sha256(data, sizeof(data) - 1, res);
    if (ret == NULL) {
        fprintf(stderr, "%s: sha256() returned NULL for input: %s\n", __func__, data);
        exit(-1);
    }

    if (memcmp(expected_res, res, sizeof(res) * sizeof(res[0]))) {
        fprintf(stderr, "%s: sha256() produced invalid checksum for string %s\n", __func__, data);
        exit(-1);
    }
}
