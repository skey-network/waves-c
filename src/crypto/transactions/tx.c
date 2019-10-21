#include "tx.h"
#include "blake2b/sse/blake2.h"
#include "base58/b58.h"
#include <string.h>

static bool tx_has_only_v1(uint8_t type)
{
    switch (type)
    {
        case TRANSACTION_TYPE_DATA:
        case TRANSACTION_TYPE_INVOKE_SCRIPT:
        case TRANSACTION_TYPE_MASS_TRANSFER:
        case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        case TRANSACTION_TYPE_SET_SCRIPT:
        case TRANSACTION_TYPE_SPONSORSHIP:
            return true;
        default:
            return false;
    }
}

void waves_tx_hash_bytes(uint8_t* hash, const uint8_t* bytes, size_t nb)
{
    blake2b_state hs[1];
    blake2b_init(hs, 32);
    blake2b_update(hs, bytes, nb);
    blake2b_final(hs, hash, 32);
}

ssize_t waves_tx_get_id(uint8_t* dst, const uint8_t* bytes, size_t nb)
{
    const uint8_t* p = bytes;
    uint8_t tx_type = p[0] == TX_MULTIVERSION ? p[1] : p[0];
    if (p[0] == TX_MULTIVERSION && tx_type != TRANSACTION_TYPE_EXCHANGE)
    {
        nb--;
        p++;
    }
    if (tx_type == TRANSACTION_TYPE_ALIAS)
    {
        if (nb <= 51) // 35+16
        {
            return -1;
        }
        size_t tmp_nb = nb - 51;
        unsigned char tmp_buf[tmp_nb];
        tmp_buf[0] = p[0];
        memcpy(&tmp_buf[1], &p[36], tmp_nb-1);
        waves_tx_hash_bytes(dst, tmp_buf, tmp_nb);
    }
    else
    {
        waves_tx_hash_bytes(dst, p, nb);
    }
    return 32;
}

tx_string_t* waves_tx_id(waves_tx_t* tx)
{
    size_t nb = waves_tx_buffer_size(tx);
    unsigned char buf[nb];
    nb = waves_tx_to_bytes(buf, tx);
    uint8_t hash[32];
    if (waves_tx_get_id(hash, buf, nb) < 0)
    {
        return NULL;
    }
    char id_buf [sizeof(hash)*2];
    ssize_t id_sz = base58_encode(id_buf, hash, sizeof(hash));
    tx_string_t* id = tx_malloc(sizeof(tx_string_t));
    id->data = (char*)tx_malloc(id_sz+1);
    id->len = id_sz;
    memcpy(id->data, id_buf, id_sz);
    id->data[id_sz] = '\0';
    return id;
}

void waves_tx_destroy_string(tx_string_t* id)
{
    if (id == NULL)
    {
        return;
    }
    tx_destroy_string(id);
    tx_free(id);
}


int waves_tx_init(waves_tx_t* tx, uint8_t tx_type)
{
    switch (tx_type)
    {
    case TRANSACTION_TYPE_ISSUE:
    case TRANSACTION_TYPE_TRANSFER:
    case TRANSACTION_TYPE_REISSUE:
    case TRANSACTION_TYPE_BURN:
        break;
    case TRANSACTION_TYPE_EXCHANGE:
        tx_array_init(&tx->data.exchange.order1.proofs, sizeof(tx_encoded_string_t), tx_destroy_proof);
        tx_array_init(&tx->data.exchange.order2.proofs, sizeof(tx_encoded_string_t), tx_destroy_proof);
        break;
    case TRANSACTION_TYPE_LEASE:
    case TRANSACTION_TYPE_CANCEL_LEASE:
    case TRANSACTION_TYPE_ALIAS:
        break;
    case TRANSACTION_TYPE_MASS_TRANSFER:
        tx_array_init(&tx->data.mass_transfer.transfers, sizeof(tx_transfer_t), tx_destroy_transfer);
        break;
    case TRANSACTION_TYPE_DATA:
        tx_array_init(&tx->data.data.data, sizeof(tx_data_entry_t), tx_destroy_data_entry);
        break;
    case TRANSACTION_TYPE_SET_SCRIPT:
    case TRANSACTION_TYPE_SPONSORSHIP:
    case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        break;
    case TRANSACTION_TYPE_INVOKE_SCRIPT:
        tx_array_init(&tx->data.invoke_script.payments, sizeof(tx_payment_t), tx_destroy_payment);
        tx_array_init(&tx->data.invoke_script.call.args, sizeof(tx_func_arg_t), tx_destroy_func_arg);
        break;
    default:
        return -1;
    }
    tx->type = tx_type;
    return 0;
}

waves_tx_t* waves_tx_new(uint8_t tx_type)
{
    waves_tx_t* tx = (waves_tx_t*)tx_malloc(sizeof(waves_tx_t));
    memset(tx, 0, sizeof(waves_tx_t));
    if (waves_tx_init(tx, tx_type) < 0)
    {
        tx_free(tx);
        return NULL;
    }
    return tx;
}

waves_tx_t* waves_tx_load(const unsigned char *src)
{
    waves_tx_t* tx;
    ssize_t nbytes;
    uint8_t type = src[0];
    if (type == 0x0)
    {
        type = src[1];
    }
    if ((tx = waves_tx_new(type)) == NULL)
    {
        return NULL;
    }
    if ((nbytes = waves_tx_from_bytes(tx, src)) < 0)
    {
        waves_tx_destroy(tx);
        return NULL;
    }
    return tx;
}

void waves_tx_destroy_buffer(waves_tx_buffer_t* buf)
{
    if (buf->data != NULL)
    {
        tx_free(buf->data);
        buf->data = NULL;
    }
    buf->size = 0;
}

ssize_t waves_tx_from_bytes(waves_tx_t* tx, const unsigned char *src)
{
    ssize_t nbytes;
    const unsigned char *p = src;
    tx->type = p[0];
    if (tx->type == TX_MULTIVERSION)
    {
        tx->type = p[1];
        if (tx_has_only_v1(tx->type))
        {
            return -1;
        }
        tx->version = p[2];
        if (tx->version != TX_VERSION_1 && tx->version != TX_VERSION_2)
        {
            return -1;
        }
        p += 3;
    }
    else
    {
        tx->version = TX_VERSION_1;
        p++;
        if (tx_has_only_v1(tx->type))
        {
            if (*p++ != TX_VERSION_1)
            {
                return -1;
            }
        }
    }
    switch (tx->type)
    {
    case TRANSACTION_TYPE_ISSUE:
        nbytes = waves_issue_tx_from_bytes(&tx->data.issue, p, tx->version);
        break;
    case TRANSACTION_TYPE_TRANSFER:
        nbytes = waves_transfer_tx_from_bytes(&tx->data.transfer, p);
        break;
    case TRANSACTION_TYPE_REISSUE:
        nbytes = waves_reissue_tx_from_bytes(&tx->data.reissue, p, tx->version);
        break;
    case TRANSACTION_TYPE_BURN:
        nbytes = waves_burn_tx_from_bytes(&tx->data.burn, p, tx->version);
        break;
    case TRANSACTION_TYPE_EXCHANGE:
        nbytes = waves_exchange_tx_from_bytes(&tx->data.exchange, p);
        break;
    case TRANSACTION_TYPE_LEASE:
        nbytes = waves_lease_tx_from_bytes(&tx->data.lease, p, tx->version);
        break;
    case TRANSACTION_TYPE_CANCEL_LEASE:
        nbytes = waves_lease_cancel_tx_from_bytes(&tx->data.lease_cancel, p);
        break;
    case TRANSACTION_TYPE_ALIAS:
        nbytes = waves_alias_tx_from_bytes(&tx->data.alias, p);
        break;
    case TRANSACTION_TYPE_MASS_TRANSFER:
        nbytes = waves_mass_transfer_tx_from_bytes(&tx->data.mass_transfer, p);
        break;
    case TRANSACTION_TYPE_DATA:
        nbytes = waves_data_tx_from_bytes(&tx->data.data, p);
        break;
    case TRANSACTION_TYPE_SET_SCRIPT:
        nbytes = waves_set_script_tx_from_bytes(&tx->data.set_script, p);
        break;
    case TRANSACTION_TYPE_SPONSORSHIP:
        nbytes = waves_sponsorship_tx_from_bytes(&tx->data.sponsorship, p);
        break;
    case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        nbytes = waves_set_asset_script_tx_from_bytes(&tx->data.set_asset_script, p);
        break;
    case TRANSACTION_TYPE_INVOKE_SCRIPT:
        nbytes = waves_invoke_script_tx_from_bytes(&tx->data.invoke_script, p);
        break;
    default:
        return -1;
    }
    if (nbytes < 0)
    {
        return nbytes;
    }
    p += nbytes;
    return p - src;
}

size_t waves_tx_to_bytes(unsigned char *dst, const waves_tx_t* tx)
{
    unsigned char *p = dst;
    if (tx->version > TX_VERSION_1)
    {
        *p++ = TX_MULTIVERSION;
        *p++ = tx->type;
        *p++ = tx->version;
    }
    else
    {
        *p++ = tx->type;
        if (tx_has_only_v1(tx->type))
        {
            *p++ = tx->version;
        }
    }
    switch (tx->type)
    {
    case TRANSACTION_TYPE_ISSUE:
        p += waves_issue_tx_to_bytes(p, &tx->data.issue, tx->version);
        break;
    case TRANSACTION_TYPE_TRANSFER:
        p += waves_transfer_tx_to_bytes(p, &tx->data.transfer);
        break;
    case TRANSACTION_TYPE_REISSUE:
        p += waves_reissue_tx_to_bytes(p, &tx->data.reissue, tx->version);
        break;
    case TRANSACTION_TYPE_BURN:
        p += waves_burn_tx_to_bytes(p, &tx->data.burn, tx->version);
        break;
    case TRANSACTION_TYPE_EXCHANGE:
        p += waves_exchange_tx_to_bytes(p, &tx->data.exchange, tx->version);
        break;
    case TRANSACTION_TYPE_LEASE:
        p += waves_lease_tx_to_bytes(p, &tx->data.lease, tx->version);
        break;
    case TRANSACTION_TYPE_CANCEL_LEASE:
        p += waves_lease_cancel_tx_to_bytes(p, &tx->data.lease_cancel);
        break;
    case TRANSACTION_TYPE_ALIAS:
        p += waves_alias_tx_to_bytes(p, &tx->data.alias);
        break;
    case TRANSACTION_TYPE_MASS_TRANSFER:
        p += waves_mass_transfer_tx_to_bytes(p, &tx->data.mass_transfer);
        break;
    case TRANSACTION_TYPE_DATA:
        p += waves_data_tx_to_bytes(p, &tx->data.data);
        break;
    case TRANSACTION_TYPE_SET_SCRIPT:
        p += waves_set_script_tx_to_bytes(p, &tx->data.set_script);
        break;
    case TRANSACTION_TYPE_SPONSORSHIP:
        p += waves_sponsorship_tx_to_bytes(p, &tx->data.sponsorship);
        break;
    case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        p += waves_set_asset_script_tx_to_bytes(p, &tx->data.set_asset_script);
        break;
    case TRANSACTION_TYPE_INVOKE_SCRIPT:
        p += waves_invoke_script_tx_to_bytes(p, &tx->data.invoke_script);
        break;
    default:
        return 0;
    }
    return p - dst;
}

waves_tx_buffer_t waves_tx_to_byte_buffer(const waves_tx_t* tx)
{
    size_t nb = waves_tx_buffer_size(tx);
    waves_tx_buffer_t buffer;
    buffer.data = (unsigned char*)tx_malloc(nb);
    buffer.size = nb;
    waves_tx_to_bytes(buffer.data, tx);
    return buffer;
}

size_t waves_tx_buffer_size(const waves_tx_t* tx)
{
    size_t nb = tx->version == TX_VERSION_1 ? 1 : 3;
    if (tx_has_only_v1(tx->type))
    {
        nb++;
    }
    switch (tx->type)
    {
    case TRANSACTION_TYPE_ISSUE:
        return nb + waves_issue_tx_buffer_size(&tx->data.issue, tx->version);
    case TRANSACTION_TYPE_TRANSFER:
        return nb + waves_transfer_tx_buffer_size(&tx->data.transfer);
    case TRANSACTION_TYPE_REISSUE:
        return nb + waves_reissue_tx_buffer_size(&tx->data.reissue, tx->version);
    case TRANSACTION_TYPE_BURN:
        return nb + waves_burn_tx_buffer_size(&tx->data.burn, tx->version);
    case TRANSACTION_TYPE_EXCHANGE:
        return nb + waves_exchange_tx_buffer_size(&tx->data.exchange);
    case TRANSACTION_TYPE_LEASE:
        return nb + waves_lease_tx_buffer_size(&tx->data.lease, tx->version);
    case TRANSACTION_TYPE_CANCEL_LEASE:
        return nb + waves_lease_cancel_tx_buffer_size(&tx->data.lease_cancel);
    case TRANSACTION_TYPE_ALIAS:
        return nb + waves_alias_tx_buffer_size(&tx->data.alias);
    case TRANSACTION_TYPE_MASS_TRANSFER:
        return nb + waves_mass_transfer_tx_buffer_size(&tx->data.mass_transfer);
    case TRANSACTION_TYPE_DATA:
        return nb + waves_data_tx_buffer_size(&tx->data.data);
    case TRANSACTION_TYPE_SET_SCRIPT:
        return nb + waves_set_script_tx_buffer_size(&tx->data.set_script);
    case TRANSACTION_TYPE_SPONSORSHIP:
        return nb + waves_sponsorship_tx_buffer_size(&tx->data.sponsorship);
    case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        return nb + waves_set_asset_script_tx_buffer_size(&tx->data.set_asset_script);
    case TRANSACTION_TYPE_INVOKE_SCRIPT:
        return nb + waves_invoke_script_tx_buffer_size(&tx->data.invoke_script);
    default:
        return 0;
    }
}

void waves_tx_destroy(waves_tx_t* tx)
{
    if (tx == NULL)
    {
        return;
    }
    switch (tx->type)
    {
        case TRANSACTION_TYPE_ISSUE:
            waves_destroy_issue_tx(&tx->data.issue);
            break;
        case TRANSACTION_TYPE_TRANSFER:
            waves_destroy_transfer_tx(&tx->data.transfer);
            break;
        case TRANSACTION_TYPE_MASS_TRANSFER:
            waves_destroy_mass_transfer_tx(&tx->data.mass_transfer);
            break;
        case TRANSACTION_TYPE_EXCHANGE:
            waves_destroy_exchange_tx(&tx->data.exchange);
            break;
        case TRANSACTION_TYPE_DATA:
            waves_destroy_data_tx(&tx->data.data);
            break;
        case TRANSACTION_TYPE_SET_SCRIPT:
            waves_destroy_set_script_tx(&tx->data.set_script);
            break;
        case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
            waves_destroy_set_asset_script_tx(&tx->data.set_asset_script);
            break;
        case TRANSACTION_TYPE_INVOKE_SCRIPT:
            waves_destroy_invoke_script_tx(&tx->data.invoke_script);
            break;
        case TRANSACTION_TYPE_REISSUE:
            waves_destroy_reissue_tx(&tx->data.reissue);
            break;
        case TRANSACTION_TYPE_BURN:
            waves_destroy_burn_tx(&tx->data.burn);
            break;
        case TRANSACTION_TYPE_LEASE:
            waves_destroy_lease_tx(&tx->data.lease);
            break;
        case TRANSACTION_TYPE_CANCEL_LEASE:
            waves_destroy_lease_cancel_tx(&tx->data.lease_cancel);
            break;
        case TRANSACTION_TYPE_ALIAS:
            waves_destroy_alias_tx(&tx->data.alias);
            break;
        case TRANSACTION_TYPE_SPONSORSHIP:
            waves_destroy_sponsorship_tx(&tx->data.sponsorship);
            break;
        default:;
    }
    tx_free(tx);
}

tx_timestamp_t waves_tx_get_timestamp(waves_tx_t* tx)
{
    switch (tx->type)
    {
    case TRANSACTION_TYPE_ISSUE:
        return tx->data.issue.timestamp;
    case TRANSACTION_TYPE_TRANSFER:
        return tx->data.transfer.timestamp;
    case TRANSACTION_TYPE_REISSUE:
        return tx->data.reissue.timestamp;
    case TRANSACTION_TYPE_BURN:
        return tx->data.burn.timestamp;
    case TRANSACTION_TYPE_EXCHANGE:
        return tx->data.exchange.timestamp;
    case TRANSACTION_TYPE_LEASE:
        return tx->data.lease.timestamp;
    case TRANSACTION_TYPE_CANCEL_LEASE:
        return tx->data.lease_cancel.timestamp;
    case TRANSACTION_TYPE_ALIAS:
        return tx->data.alias.timestamp;
    case TRANSACTION_TYPE_MASS_TRANSFER:
        return tx->data.mass_transfer.timestamp;
    case TRANSACTION_TYPE_DATA:
        return tx->data.data.timestamp;
    case TRANSACTION_TYPE_SET_SCRIPT:
        return tx->data.set_script.timestamp;
    case TRANSACTION_TYPE_SPONSORSHIP:
        return tx->data.sponsorship.timestamp;
    case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        return tx->data.set_asset_script.timestamp;
    case TRANSACTION_TYPE_INVOKE_SCRIPT:
        return tx->data.invoke_script.timestamp;
    default:
        return 0;
    }
}

tx_fee_t waves_tx_get_fee(waves_tx_t* tx)
{
    switch (tx->type)
    {
    case TRANSACTION_TYPE_ISSUE:
        return tx->data.issue.fee;
    case TRANSACTION_TYPE_TRANSFER:
        return tx->data.transfer.fee;
    case TRANSACTION_TYPE_REISSUE:
        return tx->data.reissue.fee;
    case TRANSACTION_TYPE_BURN:
        return tx->data.burn.fee;
    case TRANSACTION_TYPE_EXCHANGE:
        return tx->data.exchange.fee;
    case TRANSACTION_TYPE_LEASE:
        return tx->data.lease.fee;
    case TRANSACTION_TYPE_CANCEL_LEASE:
        return tx->data.lease_cancel.fee;
    case TRANSACTION_TYPE_ALIAS:
        return tx->data.alias.fee;
    case TRANSACTION_TYPE_MASS_TRANSFER:
        return tx->data.mass_transfer.fee;
    case TRANSACTION_TYPE_DATA:
        return tx->data.data.fee;
    case TRANSACTION_TYPE_SET_SCRIPT:
        return tx->data.set_script.fee;
    case TRANSACTION_TYPE_SPONSORSHIP:
        return tx->data.sponsorship.fee;
    case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        return tx->data.set_asset_script.fee;
    case TRANSACTION_TYPE_INVOKE_SCRIPT:
        return tx->data.invoke_script.fee;
    default:
        return 0;
    }
}

ssize_t waves_tx_set_sender_public_key(waves_tx_t* tx, const char* src)
{
    switch (tx->type)
    {
    case TRANSACTION_TYPE_ISSUE:
        return tx_set_sender_public_key(&tx->data.issue.sender_public_key, src);
    case TRANSACTION_TYPE_TRANSFER:
        return tx_set_sender_public_key(&tx->data.transfer.sender_public_key, src);
    case TRANSACTION_TYPE_REISSUE:
        return tx_set_sender_public_key(&tx->data.reissue.sender_public_key, src);
    case TRANSACTION_TYPE_BURN:
        return tx_set_sender_public_key(&tx->data.burn.sender_public_key, src);
    //case TRANSACTION_TYPE_EXCHANGE:
    case TRANSACTION_TYPE_LEASE:
        return tx_set_sender_public_key(&tx->data.lease.sender_public_key, src);
    case TRANSACTION_TYPE_CANCEL_LEASE:
        return tx_set_sender_public_key(&tx->data.lease_cancel.sender_public_key, src);
    case TRANSACTION_TYPE_ALIAS:
        return tx_set_sender_public_key(&tx->data.alias.sender_public_key, src);
    case TRANSACTION_TYPE_MASS_TRANSFER:
        return tx_set_sender_public_key(&tx->data.mass_transfer.sender_public_key, src);
    case TRANSACTION_TYPE_DATA:
        return tx_set_sender_public_key(&tx->data.data.sender_public_key, src);
    case TRANSACTION_TYPE_SET_SCRIPT:
        return tx_set_sender_public_key(&tx->data.set_script.sender_public_key, src);
    case TRANSACTION_TYPE_SPONSORSHIP:
        return tx_set_sender_public_key(&tx->data.sponsorship.sender_public_key, src);
    case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        return tx_set_sender_public_key(&tx->data.set_asset_script.sender_public_key, src);
    case TRANSACTION_TYPE_INVOKE_SCRIPT:
        return tx_set_sender_public_key(&tx->data.invoke_script.sender_public_key, src);
    default:
        return -1;
    }
}

ssize_t waves_tx_set_asset_id(waves_tx_t* tx, const char* src)
{
    switch (tx->type)
    {
    case TRANSACTION_TYPE_TRANSFER:
        return tx_set_asset_id(&tx->data.transfer.asset_id, src);
    case TRANSACTION_TYPE_REISSUE:
        return tx_set_asset_id(&tx->data.reissue.asset_id, src);
    case TRANSACTION_TYPE_BURN:
        return tx_set_asset_id(&tx->data.burn.asset_id, src);
    case TRANSACTION_TYPE_MASS_TRANSFER:
        return tx_set_asset_id(&tx->data.mass_transfer.asset_id, src);
    case TRANSACTION_TYPE_SPONSORSHIP:
        return tx_set_asset_id(&tx->data.sponsorship.asset_id, src);
    case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        return tx_set_asset_id(&tx->data.set_asset_script.asset_id, src);
    default:
        return -1;
    }
}

ssize_t waves_tx_set_chain_id(waves_tx_t* tx, tx_chain_id_t chain_id)
{
    switch (tx->type)
    {
    case TRANSACTION_TYPE_ISSUE:
        tx->data.issue.chain_id = chain_id;
        break;
    case TRANSACTION_TYPE_REISSUE:
        tx->data.reissue.chain_id = chain_id;
        break;
    case TRANSACTION_TYPE_BURN:
        tx->data.burn.chain_id = chain_id;
        break;
    case TRANSACTION_TYPE_CANCEL_LEASE:
        tx->data.lease_cancel.chain_id = chain_id;
        break;
    case TRANSACTION_TYPE_SET_SCRIPT:
        tx->data.set_script.chain_id = chain_id;
        break;
    case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        tx->data.set_asset_script.chain_id = chain_id;
        break;
    case TRANSACTION_TYPE_INVOKE_SCRIPT:
        tx->data.invoke_script.chain_id = chain_id;
        break;
    default:
        return -1;
    }
    return sizeof(tx_chain_id_t);
}

ssize_t waves_tx_set_timestamp(waves_tx_t* tx, tx_timestamp_t timestamp)
{
    switch (tx->type)
    {
    case TRANSACTION_TYPE_ISSUE:
        tx->data.issue.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_TRANSFER:
        tx->data.transfer.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_REISSUE:
        tx->data.reissue.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_BURN:
        tx->data.burn.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_EXCHANGE:
        tx->data.exchange.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_LEASE:
        tx->data.lease.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_CANCEL_LEASE:
        tx->data.lease_cancel.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_ALIAS:
        tx->data.alias.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_MASS_TRANSFER:
        tx->data.mass_transfer.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_DATA:
        tx->data.data.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_SET_SCRIPT:
        tx->data.set_script.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_SPONSORSHIP:
        tx->data.sponsorship.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        tx->data.set_asset_script.timestamp = timestamp;
        break;
    case TRANSACTION_TYPE_INVOKE_SCRIPT:
        tx->data.invoke_script.timestamp = timestamp;
        break;
    default:
        return -1;
    }
    return sizeof(tx_timestamp_t);
}

ssize_t waves_tx_set_quantity(waves_tx_t* tx, tx_quantity_t quantity)
{
    switch (tx->type)
    {
    case TRANSACTION_TYPE_ISSUE:
        tx->data.issue.quantity = quantity;
        break;
    case TRANSACTION_TYPE_REISSUE:
        tx->data.reissue.quantity = quantity;
        break;
    case TRANSACTION_TYPE_BURN:
        tx->data.burn.quantity = quantity;
        break;
    default:
        return -1;
    }
    return sizeof(tx_quantity_t);
}

ssize_t waves_tx_set_amount(waves_tx_t* tx, tx_amount_t amount)
{
    switch (tx->type)
    {
    case TRANSACTION_TYPE_TRANSFER:
        tx->data.transfer.amount = amount;
        break;
    case TRANSACTION_TYPE_LEASE:
        tx->data.lease.amount = amount;
        break;
    default:
        return -1;
    }
    return sizeof(tx_amount_t);
}

ssize_t waves_tx_set_fee(waves_tx_t* tx, tx_fee_t fee)
{
    switch (tx->type)
    {
    case TRANSACTION_TYPE_ISSUE:
        tx->data.issue.fee = fee;
        break;
    case TRANSACTION_TYPE_TRANSFER:
        tx->data.transfer.fee = fee;
        break;
    case TRANSACTION_TYPE_REISSUE:
        tx->data.reissue.fee = fee;
        break;
    case TRANSACTION_TYPE_BURN:
        tx->data.burn.fee = fee;
        break;
    case TRANSACTION_TYPE_EXCHANGE:
        tx->data.exchange.fee = fee;
        break;
    case TRANSACTION_TYPE_LEASE:
        tx->data.lease.fee = fee;
        break;
    case TRANSACTION_TYPE_CANCEL_LEASE:
        tx->data.lease_cancel.fee = fee;
        break;
    case TRANSACTION_TYPE_ALIAS:
        tx->data.alias.fee = fee;
        break;
    case TRANSACTION_TYPE_MASS_TRANSFER:
        tx->data.mass_transfer.fee = fee;
        break;
    case TRANSACTION_TYPE_DATA:
        tx->data.data.fee = fee;
        break;
    case TRANSACTION_TYPE_SET_SCRIPT:
        tx->data.set_script.fee = fee;
        break;
    case TRANSACTION_TYPE_SPONSORSHIP:
        tx->data.sponsorship.fee = fee;
        break;
    case TRANSACTION_TYPE_SET_ASSET_SCRIPT:
        tx->data.set_asset_script.fee = fee;
        break;
    case TRANSACTION_TYPE_INVOKE_SCRIPT:
        tx->data.invoke_script.fee = fee;
        break;
    default: return -1;
    }
    return sizeof(tx_fee_t);
}

ssize_t waves_tx_set_reissuable(waves_tx_t* tx, tx_reissuable_t reissuable)
{
    switch (tx->type)
    {
    case TRANSACTION_TYPE_ISSUE:
        tx->data.issue.reissuable = reissuable;
        break;
    case TRANSACTION_TYPE_REISSUE:
        tx->data.reissue.reissuable = reissuable;
        break;
    default:
        return -1;
    }
    return sizeof(tx_reissuable_t);
}

tx_array_ssize_t waves_tx_data_get_num_entries(waves_tx_t* tx)
{
    if (tx->type != TRANSACTION_TYPE_DATA)
    {
        return -1;
    }
    return tx->data.data.data.len;
}

tx_data_entry_t* waves_tx_data_get_entry(waves_tx_t* tx, size_t i)
{
    if (tx->type != TRANSACTION_TYPE_DATA)
    {
        return NULL;
    }
    data_tx_bytes_t* data_tx = &tx->data.data;
    if (data_tx->data.len <= i)
    {
        return NULL;
    }
    tx_data_entry_t* entries = (tx_data_entry_t*)data_tx->data.array;
    return &entries[i];
}

tx_data_entry_t* waves_tx_data_add_entry(waves_tx_t* tx, const char* key)
{
    if (tx->type != TRANSACTION_TYPE_DATA)
    {
        return NULL;
    }
    tx_array_t* array = &tx->data.data.data;
    tx_data_entry_t* e = (tx_data_entry_t*)tx_array_new_elem(array);
    waves_tx_set_string(&e->key, key);
    return e;
}

tx_data_entry_t* waves_tx_data_add_entry_integer(waves_tx_t* tx, const char* key, tx_data_integer_t value)
{
    tx_data_entry_t* e;
    if ((e = waves_tx_data_add_entry(tx, key)) == NULL)
    {
        return NULL;
    }
    waves_tx_data_set_integer(&e->value, value);
    return e;
}

tx_data_entry_t* waves_tx_data_add_entry_boolean(waves_tx_t* tx, const char* key, tx_data_boolean_t value)
{
    tx_data_entry_t* e;
    if ((e = waves_tx_data_add_entry(tx, key)) == NULL)
    {
        return NULL;
    }
    waves_tx_data_set_boolean(&e->value, value);
    return e;
}

tx_data_entry_t* waves_tx_data_add_entry_string(waves_tx_t* tx, const char* key, const char* value)
{
    tx_data_entry_t* e;
    if ((e = waves_tx_data_add_entry(tx, key)) == NULL)
    {
        return NULL;
    }
    waves_tx_data_set_string(&e->value, value);
    return e;
}

tx_data_entry_t* waves_tx_data_add_entry_binary(waves_tx_t* tx, const char* key, const char* value, size_t len)
{
    tx_data_entry_t* e;
    if ((e = waves_tx_data_add_entry(tx, key)) == NULL)
    {
        return NULL;
    }
    waves_tx_data_set_binary(&e->value, value, len);
    return e;
}
