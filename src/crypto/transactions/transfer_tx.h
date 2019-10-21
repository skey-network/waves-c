#ifndef __WAVES_TRANSFER_TRANSACTION_H_23281__
#define __WAVES_TRANSFER_TRANSACTION_H_23281__

#include "tx_common.h"

typedef struct transfer_tx_bytes_s
{
    tx_public_key_t sender_public_key;
    tx_asset_id_t asset_id;
    tx_asset_id_t fee_asset_id;
    tx_timestamp_t timestamp;
    tx_amount_t amount;
    tx_fee_t fee;
    tx_addr_or_alias_t recipient;
    tx_attachment_t attachment;
} transfer_tx_bytes_t;

ssize_t waves_transfer_tx_from_bytes(transfer_tx_bytes_t *tx, const unsigned char *src);
size_t waves_transfer_tx_to_bytes(unsigned char* dst, const transfer_tx_bytes_t* tx);
void waves_destroy_transfer_tx(transfer_tx_bytes_t* tx);
size_t waves_transfer_tx_buffer_size(const transfer_tx_bytes_t *tx);

#endif /* __WAVES_TRANSFER_TRANSACTION_H_23281__ */
