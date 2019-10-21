#ifndef __WAVES_LEASE_TRANSACTION_H_20038__
#define __WAVES_LEASE_TRANSACTION_H_20038__

#include "tx_common.h"

typedef struct lease_tx_bytes_s
{
    tx_lease_asset_id_t lease_asset_id;
    tx_public_key_t sender_public_key;
    tx_addr_or_alias_t recipient;
    tx_amount_t amount;
    tx_fee_t fee;
    tx_timestamp_t timestamp;
} lease_tx_bytes_t;

ssize_t waves_lease_tx_from_bytes(lease_tx_bytes_t* tx, const unsigned char *src, tx_version_t version);
size_t waves_lease_tx_to_bytes(unsigned char *dst, const lease_tx_bytes_t *tx, tx_version_t version);
size_t waves_lease_tx_buffer_size(const lease_tx_bytes_t *tx, tx_version_t version);
void waves_destroy_lease_tx(lease_tx_bytes_t* tx);

#endif /* __WAVES_LEASE_TRANSACTION_H_20038__ */
