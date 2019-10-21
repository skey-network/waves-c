#include "crypto_tests.h"
#include "utils_tests.h"
#include "tx_tests.h"

void crypto_tests() {
    waves_secure_hash_test();
    waves_public_key_to_address_mainnet_test();
    waves_public_key_to_address_testnet_test();
    curve25519_keygen_test();
    waves_seed_to_address_testnet_test();
    waves_seed_to_address_mainnet_test();
    waves_message_sign_without_random_test();
    waves_message_sign_test();
    waves_message_verify_test();
    waves_message_verify_negative_test();
    waves_crypto_sha256_test();
}

void utils_tests() {
    print_amount_test_1();
    print_amount_test_2();
    print_amount_test_3();
    print_amount_test_4();
    print_amount_test_5();
    print_amount_test_6();
    print_amount_test_7();
}

int main(int argc, char **argv) {
    crypto_tests();
    test_data_tx_bytes();
    test_data_tx_building();
    utils_tests();
}
