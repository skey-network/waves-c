#include "lease_tx.h"
#include <string.h>

ssize_t waves_lease_tx_from_bytes(lease_tx_bytes_t* tx, const unsigned char *src, tx_version_t version)
{
    ssize_t nbytes = 0;
    const unsigned char* p = src;
    if (version > TX_VERSION_1)
    {
        p += tx_load_lease_asset_id(&tx->lease_asset_id, p);
    }
    p += tx_load_public_key(&tx->sender_public_key, p);
    if ((nbytes = tx_load_addr_or_alias(&tx->recipient, p)) < 0)
    {
        return tx_parse_error_pos(p, src);
    }
    p += nbytes;
    p += tx_load_amount(&tx->amount, p);
    p += tx_load_fee(&tx->fee, p);
    p += tx_load_timestamp(&tx->timestamp, p);
    return p - src;
}

size_t waves_lease_tx_to_bytes(unsigned char *dst, const lease_tx_bytes_t* tx, tx_version_t version)
{
    unsigned char* p = dst;
    if (version > TX_VERSION_1)
    {
        p += tx_store_lease_asset_id(p, &tx->lease_asset_id);
    }
    p += tx_store_public_key(p, &tx->sender_public_key);
    p += tx_store_addr_or_alias(p, &tx->recipient);
    p += tx_store_amount(p, tx->amount);
    p += tx_store_fee(p, tx->fee);
    p += tx_store_timestamp(p, tx->timestamp);
    return p - dst;
}

size_t waves_lease_tx_buffer_size(const lease_tx_bytes_t *tx, tx_version_t version)
{
    size_t nb = 0;
    if (version > TX_VERSION_1)
    {
        nb += tx_lease_asset_id_buffer_size(&tx->lease_asset_id);
    }
    nb += tx_public_key_buffer_size(&tx->sender_public_key);
    nb += tx_addr_or_alias_buffer_size(&tx->recipient);
    nb += sizeof(tx->amount);
    nb += sizeof(tx->fee);
    nb += sizeof(tx->timestamp);
    return nb;
}

void waves_destroy_lease_tx(lease_tx_bytes_t* tx)
{
    tx_destroy_lease_asset_id(&tx->lease_asset_id);
    tx_destroy_addr_or_alias(&tx->recipient);
    tx_destroy_public_key(&tx->sender_public_key);
}
