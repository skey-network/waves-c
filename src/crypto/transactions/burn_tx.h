#ifndef __WAVES_BURN_TRANSACTION_H_26080__
#define __WAVES_BURN_TRANSACTION_H_26080__

#include "tx_common.h"

typedef struct burn_tx_bytes_s
{
    tx_chain_id_t chain_id;
    tx_public_key_t sender_public_key;
    tx_asset_id_t asset_id;
    tx_quantity_t quantity;
    tx_fee_t fee;
    tx_timestamp_t timestamp;
} burn_tx_bytes_t;

ssize_t waves_burn_tx_from_bytes(burn_tx_bytes_t* tx, const unsigned char *src, uint8_t version);
size_t waves_burn_tx_to_bytes(unsigned char *dst, const burn_tx_bytes_t* tx, uint8_t version);
size_t waves_burn_tx_buffer_size(const burn_tx_bytes_t* tx, uint8_t version);
void waves_destroy_burn_tx(burn_tx_bytes_t* tx);

#endif /* __WAVES_BURN_TRANSACTION_H_26080__ */
