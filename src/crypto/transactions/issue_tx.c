#include "issue_tx.h"

ssize_t waves_issue_tx_from_bytes(issue_tx_bytes_t *tx, const unsigned char *src, tx_version_t version)
{
    const unsigned char* p = src;
    if (version > TX_VERSION_1)
    {
        p += tx_load_chain_id(&tx->chain_id, p);
    }
    p += tx_load_public_key(&tx->sender_public_key, p);
    p += tx_load_string(&tx->name, p);
    p += tx_load_string(&tx->description, p);
    p += tx_load_quantity(&tx->quantity, p);
    p += tx_load_decimals(&tx->decimals, p);
    p += tx_load_reissuable(&tx->reissuable, p);
    p += tx_load_fee(&tx->fee, p);
    p += tx_load_timestamp(&tx->timestamp, p);
    p += tx_load_script(&tx->script, p);
    return p - src;
}

size_t waves_issue_tx_to_bytes(unsigned char* dst, const issue_tx_bytes_t* tx, tx_version_t version)
{
    unsigned char* p = dst;
    if (version > TX_VERSION_1)
    {
        p += tx_store_chain_id(p, tx->chain_id);
    }
    p += tx_store_public_key(p, &tx->sender_public_key);
    p += tx_store_string(p, &tx->name);
    p += tx_store_string(p, &tx->description);
    p += tx_store_quantity(p, tx->quantity);
    p += tx_store_decimals(p, tx->decimals);
    p += tx_store_reissuable(p, tx->reissuable);
    p += tx_store_fee(p, tx->fee);
    p += tx_store_timestamp(p, tx->timestamp);
    p += tx_store_script(p, &tx->script);
    return p - dst;
}

void waves_destroy_issue_tx(issue_tx_bytes_t* tx)
{
    tx_destroy_public_key(&tx->sender_public_key);
    tx_destroy_string(&tx->name);
    tx_destroy_string(&tx->description);
    tx_destroy_base64_string(&tx->script);
}

size_t waves_issue_tx_buffer_size(const issue_tx_bytes_t *tx, tx_version_t version)
{
    size_t nb = 0;
    if (version > TX_VERSION_1)
    {
        nb += sizeof(tx->chain_id);
    }
    nb += tx_public_key_buffer_size(&tx->sender_public_key);
    nb += tx_string_buffer_size(&tx->name);
    nb += tx_string_buffer_size(&tx->description);
    nb += sizeof(tx->quantity);
    nb += sizeof(tx->decimals);
    nb += sizeof(tx->reissuable);
    nb += sizeof(tx->fee);
    nb += sizeof(tx->timestamp);
    nb += tx_script_buffer_size(&tx->script);
    return nb;
}
