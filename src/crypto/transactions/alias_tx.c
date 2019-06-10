#include "alias_tx.h"

ssize_t waves_alias_tx_from_bytes(alias_tx_bytes_t* tx, const unsigned char *src)
{
    const unsigned char* p = src;
    if (*p++ != TRANSACTION_TYPE_ALIAS)
    {
        return tx_parse_error_pos(p-1, src);
    }
    if (*p++ != TX_VERSION_2)
    {
        return tx_parse_error_pos(p-1, src);
    }
    p += tx_copy_public_key(tx->sender_public_key, p);
    p += tx_load_alias(&tx->alias, p);
    p += tx_load_fee(&tx->fee, p);
    p += tx_load_timestamp(&tx->timestamp, p);
    return p - src;
}

size_t waves_alias_tx_to_bytes(unsigned char *dst, const alias_tx_bytes_t* tx)
{
    unsigned char* p = dst;
    *p++ = TRANSACTION_TYPE_ALIAS;
    *p++ = TX_VERSION_2;
    p += tx_copy_public_key(p, tx->sender_public_key);
    p += tx_store_alias(p, &tx->alias);
    p += tx_store_fee(p, tx->fee);
    p += tx_store_timestamp(p, tx->timestamp);
    return p - dst;
}

size_t waves_alias_tx_buffer_size(const alias_tx_bytes_t* tx)
{
    size_t nb = 2;
    nb += sizeof(tx->sender_public_key);
    nb += tx_alias_buffer_size(&tx->alias);
    nb += sizeof(tx->fee);
    nb += sizeof(tx->timestamp);
    return nb;
}
