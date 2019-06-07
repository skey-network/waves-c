#include "mass_transfer_tx.h"

tx_public_key_bytes_t sender_public_key;
tx_optional_asset_id_t asset_id;
tx_transfer_array_t transfers;
tx_timestamp_t timestamp;
tx_fee_t fee;
tx_data_string_t attachment;


ssize_t waves_mass_transfer_tx_from_bytes(mass_transfer_tx_bytes_t *tx, const unsigned char *src)
{
    ssize_t nbytes = 0;
    const unsigned char* p = src;
    if (*p++ != TRANSACTION_TYPE_MASS_TRANSFER)
    {
        return tx_parse_error_pos(p-1, src);
    }
    if (*p++ != TX_VERSION_1)
    {
        return tx_parse_error_pos(p-1, src);
    }
    p += tx_copy_public_key(tx->sender_public_key, p);
    p += tx_load_optional_asset_id(&tx->asset_id, p);
    if ((nbytes = tx_load_transfer_array(&tx->transfers, p)) < 0)
    {
        return tx_parse_error_pos(p, src);
    }
    p += tx_load_timestamp(&tx->timestamp, p);
    p += tx_load_fee(&tx->fee, p);
    if ((nbytes = tx_load_attachment(&tx->attachment, p)) < 0)
    {
        return tx_parse_error_pos(p, src);
    }
    p += nbytes;
    return p - src;
}

size_t waves_mass_transfer_tx_to_bytes(unsigned char* dst, const mass_transfer_tx_bytes_t* tx)
{
    unsigned char* p = dst;
    *p++ = TRANSACTION_TYPE_MASS_TRANSFER;
    *p++ = TX_VERSION_1;
    p += tx_copy_public_key(p, tx->sender_public_key);
    p += tx_store_optional_asset_id(p, &tx->asset_id);
    p += tx_store_transfer_array(p, &tx->transfers);
    p += tx_store_timestamp(p, tx->timestamp);
    p += tx_store_fee(p, tx->fee);
    p += tx_store_attachment(p, &tx->attachment);
    return p - dst;
}
