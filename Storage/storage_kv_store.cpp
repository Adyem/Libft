#include "kv_store.hpp"
#include "../Compression/compression.hpp"
#include "../CMA/CMA.hpp"
#include "../Encryption/aes.hpp"
#include <vector>
#include <cstring>
#include <cstdint>

kv_store::kv_store(const char *file_path, const char *encryption_key, bool enable_encryption)
    : _data(), _file_path(), _encryption_key(), _encryption_enabled(false), _error_code(ER_SUCCESS)
{
    json_group *group_head;
    json_group *store_group;
    json_item *item_pointer;
    int current_error;

    if (file_path == ft_nullptr)
    {
        this->_file_path.clear();
        this->set_error(FT_EINVAL);
        return ;
    }
    this->_file_path = file_path;
    if (this->_file_path.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_file_path.get_error());
        return ;
    }
    if (enable_encryption)
    {
        if (encryption_key == ft_nullptr)
        {
            this->set_error(FT_EINVAL);
            return ;
        }
        this->_encryption_key = encryption_key;
        if (this->_encryption_key.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_encryption_key.get_error());
            return ;
        }
        if (this->_encryption_key.size() != 16)
        {
            this->set_error(FT_EINVAL);
            return ;
        }
        this->_encryption_enabled = true;
    }
    else
    {
        this->_encryption_enabled = false;
        if (encryption_key != ft_nullptr)
        {
            this->_encryption_key = encryption_key;
            if (this->_encryption_key.get_error() != ER_SUCCESS)
            {
                this->set_error(this->_encryption_key.get_error());
                return ;
            }
        }
        else
            this->_encryption_key.clear();
    }

    group_head = json_read_from_file(file_path);
    if (group_head == ft_nullptr)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_EINVAL;
        this->set_error(current_error);
        return ;
    }
    store_group = json_find_group(group_head, "kv_store");
    if (store_group == ft_nullptr)
    {
        json_free_groups(group_head);
        this->set_error(FT_EINVAL);
        return ;
    }
    item_pointer = store_group->items;
    while (item_pointer != ft_nullptr)
    {
        if (std::strcmp(item_pointer->key, "__encryption__") == 0)
        {
            if (std::strcmp(item_pointer->value, "aes-128-ecb-base64") == 0)
            {
                if (this->_encryption_enabled == false)
                {
                    json_free_groups(group_head);
                    this->set_error(FT_EINVAL);
                    return ;
                }
            }
            else
            {
                json_free_groups(group_head);
                this->set_error(FT_EINVAL);
                return ;
            }
            item_pointer = item_pointer->next;
            continue;
        }
        ft_string key_storage(item_pointer->key);

        if (key_storage.get_error() != ER_SUCCESS)
        {
            json_free_groups(group_head);
            this->set_error(key_storage.get_error());
            return ;
        }
        if (this->_encryption_enabled)
        {
            ft_string encoded_value(item_pointer->value);
            ft_string decrypted_value;

            if (encoded_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(group_head);
                this->set_error(encoded_value.get_error());
                return ;
            }
            if (this->decrypt_value(encoded_value, decrypted_value) != 0)
            {
                json_free_groups(group_head);
                return ;
            }
            this->_data.insert(key_storage, decrypted_value);
        }
        else
        {
            ft_string plain_value(item_pointer->value);

            if (plain_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(group_head);
                this->set_error(plain_value.get_error());
                return ;
            }
            this->_data.insert(key_storage, plain_value);
        }
        if (this->_data.get_error() != ER_SUCCESS)
        {
            json_free_groups(group_head);
            this->set_error(this->_data.get_error());
            return ;
        }
        item_pointer = item_pointer->next;
    }
    json_free_groups(group_head);
    this->set_error(ER_SUCCESS);
    return ;
}

kv_store::~kv_store()
{
    return ;
}

void kv_store::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

int kv_store::encrypt_value(const ft_string &plain_string, ft_string &encoded_string) const
{
    std::vector<uint8_t> output_buffer;
    const char *plain_c_string;
    std::size_t plain_size;
    uint8_t header_plain[16];
    uint8_t header_cipher[16];
    std::size_t header_index;
    uint8_t counter_block[16];
    std::size_t counter_index;
    unsigned char *encoded_buffer;
    std::size_t encoded_size;
    std::size_t output_index;
    std::size_t byte_index;
    uint64_t block_counter;

    if (this->_encryption_enabled == false)
    {
        encoded_string = plain_string;
        if (encoded_string.get_error() != ER_SUCCESS)
        {
            this->set_error(encoded_string.get_error());
            return (-1);
        }
        return (0);
    }
    if (this->_encryption_key.size() != 16)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    plain_c_string = plain_string.c_str();
    plain_size = plain_string.size();
    header_index = 0;
    while (header_index < 16)
    {
        header_plain[header_index] = static_cast<uint8_t>("kvstore-aes-ctr!"[header_index]);
        header_cipher[header_index] = header_plain[header_index];
        header_index++;
    }
    aes_encrypt(header_cipher, reinterpret_cast<const uint8_t *>(this->_encryption_key.c_str()));
    output_buffer.reserve(16 + plain_size);
    header_index = 0;
    while (header_index < 16)
    {
        output_buffer.push_back(header_cipher[header_index]);
        header_index++;
    }
    block_counter = 0;
    output_index = 0;
    while (output_index < plain_size)
    {
        counter_index = 0;
        while (counter_index < 16)
        {
            counter_block[counter_index] = header_plain[counter_index];
            counter_index++;
        }
        counter_index = 0;
        while (counter_index < 8)
        {
            counter_block[15 - counter_index] = static_cast<uint8_t>((block_counter >> (counter_index * 8)) & 0xFF);
            counter_index++;
        }
        aes_encrypt(counter_block, reinterpret_cast<const uint8_t *>(this->_encryption_key.c_str()));
        byte_index = 0;
        while (byte_index < 16 && output_index < plain_size)
        {
            uint8_t cipher_byte;

            cipher_byte = static_cast<uint8_t>(static_cast<unsigned char>(plain_c_string[output_index]) ^ counter_block[byte_index]);
            output_buffer.push_back(cipher_byte);
            output_index++;
            byte_index++;
        }
        block_counter++;
    }
    encoded_size = 0;
    encoded_buffer = ft_base64_encode(reinterpret_cast<const unsigned char *>(output_buffer.data()), output_buffer.size(), &encoded_size);
    if (encoded_buffer == ft_nullptr)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    encoded_string.clear();
    if (encoded_string.get_error() != ER_SUCCESS)
    {
        cma_free(encoded_buffer);
        this->set_error(encoded_string.get_error());
        return (-1);
    }
    output_index = 0;
    while (output_index < encoded_size)
    {
        encoded_string.append(static_cast<char>(encoded_buffer[output_index]));
        if (encoded_string.get_error() != ER_SUCCESS)
        {
            cma_free(encoded_buffer);
            this->set_error(encoded_string.get_error());
            return (-1);
        }
        output_index++;
    }
    cma_free(encoded_buffer);
    return (0);
}

int kv_store::decrypt_value(const ft_string &encoded_string, ft_string &plain_string) const
{
    unsigned char *decoded_buffer;
    std::size_t decoded_size;
    uint8_t header_plain[16];
    uint8_t expected_header[16];
    std::size_t header_index;
    std::size_t payload_size;
    const unsigned char *payload_pointer;
    uint8_t counter_block[16];
    std::size_t counter_index;
    std::size_t payload_index;
    uint64_t block_counter;
    std::size_t byte_index;

    if (this->_encryption_enabled == false)
    {
        plain_string = encoded_string;
        if (plain_string.get_error() != ER_SUCCESS)
        {
            this->set_error(plain_string.get_error());
            return (-1);
        }
        return (0);
    }
    if (this->_encryption_key.size() != 16)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    decoded_size = 0;
    decoded_buffer = ft_base64_decode(reinterpret_cast<const unsigned char *>(encoded_string.c_str()), encoded_string.size(), &decoded_size);
    if (decoded_buffer == ft_nullptr)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    if (decoded_size == 0)
    {
        plain_string.clear();
        if (plain_string.get_error() != ER_SUCCESS)
        {
            cma_free(decoded_buffer);
            this->set_error(plain_string.get_error());
            return (-1);
        }
        cma_free(decoded_buffer);
        return (0);
    }
    if (decoded_size < 16)
    {
        cma_free(decoded_buffer);
        this->set_error(FT_EINVAL);
        return (-1);
    }
    header_index = 0;
    while (header_index < 16)
    {
        header_plain[header_index] = static_cast<uint8_t>("kvstore-aes-ctr!"[header_index]);
        expected_header[header_index] = header_plain[header_index];
        header_index++;
    }
    aes_encrypt(expected_header, reinterpret_cast<const uint8_t *>(this->_encryption_key.c_str()));
    header_index = 0;
    while (header_index < 16)
    {
        if (decoded_buffer[header_index] != expected_header[header_index])
        {
            cma_free(decoded_buffer);
            this->set_error(FT_EINVAL);
            return (-1);
        }
        header_index++;
    }
    payload_size = decoded_size - 16;
    payload_pointer = decoded_buffer + 16;
    plain_string.clear();
    if (plain_string.get_error() != ER_SUCCESS)
    {
        cma_free(decoded_buffer);
        this->set_error(plain_string.get_error());
        return (-1);
    }
    payload_index = 0;
    block_counter = 0;
    while (payload_index < payload_size)
    {
        counter_index = 0;
        while (counter_index < 16)
        {
            counter_block[counter_index] = header_plain[counter_index];
            counter_index++;
        }
        counter_index = 0;
        while (counter_index < 8)
        {
            counter_block[15 - counter_index] = static_cast<uint8_t>((block_counter >> (counter_index * 8)) & 0xFF);
            counter_index++;
        }
        aes_encrypt(counter_block, reinterpret_cast<const uint8_t *>(this->_encryption_key.c_str()));
        byte_index = 0;
        while (byte_index < 16 && payload_index < payload_size)
        {
            char plain_char;

            plain_char = static_cast<char>(payload_pointer[payload_index] ^ counter_block[byte_index]);
            plain_string.append(plain_char);
            if (plain_string.get_error() != ER_SUCCESS)
            {
                cma_free(decoded_buffer);
                this->set_error(plain_string.get_error());
                return (-1);
            }
            payload_index++;
            byte_index++;
        }
        block_counter++;
    }
    cma_free(decoded_buffer);
    return (0);
}

int kv_store::kv_set(const char *key_string, const char *value_string)
{
    ft_string key_storage;
    ft_string value_storage;
    Pair<ft_string, ft_string> *existing_pair;

    if (key_string == ft_nullptr || value_string == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    key_storage = key_string;
    if (key_storage.get_error() != ER_SUCCESS)
    {
        this->set_error(key_storage.get_error());
        return (-1);
    }
    value_storage = value_string;
    if (value_storage.get_error() != ER_SUCCESS)
    {
        this->set_error(value_storage.get_error());
        return (-1);
    }
    existing_pair = this->_data.find(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_data.get_error());
        return (-1);
    }
    if (existing_pair != ft_nullptr)
    {
        existing_pair->value = value_storage;
        if (existing_pair->value.get_error() != ER_SUCCESS)
        {
            this->set_error(existing_pair->value.get_error());
            return (-1);
        }
    }
    else
    {
        this->_data.insert(key_storage, value_storage);
        if (this->_data.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_data.get_error());
            return (-1);
        }
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

const char *kv_store::kv_get(const char *key_string) const
{
    ft_string key_storage;
    const Pair<ft_string, ft_string> *map_pair;

    if (key_string == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    key_storage = key_string;
    if (key_storage.get_error() != ER_SUCCESS)
    {
        this->set_error(key_storage.get_error());
        return (ft_nullptr);
    }
    map_pair = this->_data.find(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_data.get_error());
        return (ft_nullptr);
    }
    if (map_pair == ft_nullptr)
    {
        this->set_error(MAP_KEY_NOT_FOUND);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (map_pair->value.c_str());
}

int kv_store::kv_delete(const char *key_string)
{
    ft_string key_storage;
    const Pair<ft_string, ft_string> *map_pair;

    if (key_string == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    key_storage = key_string;
    if (key_storage.get_error() != ER_SUCCESS)
    {
        this->set_error(key_storage.get_error());
        return (-1);
    }
    map_pair = this->_data.find(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_data.get_error());
        return (-1);
    }
    if (map_pair == ft_nullptr)
    {
        this->set_error(MAP_KEY_NOT_FOUND);
        return (-1);
    }
    this->_data.remove(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_data.get_error());
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::kv_flush() const
{
    json_group *store_group;
    json_group *head_group;
    json_item *item_pointer;
    ft_string stored_value;
    const Pair<ft_string, ft_string> *map_end;
    const Pair<ft_string, ft_string> *map_begin;
    size_t map_size;
    size_t map_index;
    int result;
    int error_code;

    store_group = json_create_json_group("kv_store");
    if (store_group == ft_nullptr)
    {
        this->set_error(JSON_MALLOC_FAIL);
        return (-1);
    }
    if (this->_encryption_enabled)
    {
        item_pointer = json_create_item("__encryption__", "aes-128-ecb-base64");
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(store_group);
            this->set_error(JSON_MALLOC_FAIL);
            return (-1);
        }
        json_add_item_to_group(store_group, item_pointer);
    }
    map_size = this->_data.size();
    if (this->_data.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        this->set_error(this->_data.get_error());
        return (-1);
    }
    map_end = this->_data.end();
    if (this->_data.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        this->set_error(this->_data.get_error());
        return (-1);
    }
    map_begin = map_end - static_cast<std::ptrdiff_t>(map_size);
    map_index = 0;
    while (map_index < map_size)
    {
        const Pair<ft_string, ft_string> &entry = map_begin[map_index];

        if (this->_encryption_enabled)
        {
            stored_value.clear();
            if (this->encrypt_value(entry.value, stored_value) != 0)
            {
                json_free_groups(store_group);
                return (-1);
            }
            item_pointer = json_create_item(entry.key.c_str(), stored_value.c_str());
        }
        else
            item_pointer = json_create_item(entry.key.c_str(), entry.value.c_str());
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(store_group);
            this->set_error(JSON_MALLOC_FAIL);
            return (-1);
        }
        json_add_item_to_group(store_group, item_pointer);
        map_index++;
    }
    head_group = ft_nullptr;
    json_append_group(&head_group, store_group);
    result = json_write_to_file(this->_file_path.c_str(), head_group);
    error_code = ft_errno;
    json_free_groups(head_group);
    if (result != 0)
    {
        if (error_code != ER_SUCCESS)
            this->set_error(error_code);
        else
            this->set_error(FT_EINVAL);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::configure_encryption(const char *encryption_key, bool enable_encryption)
{
    if (enable_encryption)
    {
        if (encryption_key == ft_nullptr)
        {
            this->set_error(FT_EINVAL);
            return (-1);
        }
        this->_encryption_key = encryption_key;
        if (this->_encryption_key.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_encryption_key.get_error());
            return (-1);
        }
        if (this->_encryption_key.size() != 16)
        {
            this->set_error(FT_EINVAL);
            return (-1);
        }
        this->_encryption_enabled = true;
        this->set_error(ER_SUCCESS);
        return (0);
    }
    this->_encryption_enabled = false;
    if (encryption_key != ft_nullptr)
    {
        this->_encryption_key = encryption_key;
        if (this->_encryption_key.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_encryption_key.get_error());
            return (-1);
        }
    }
    else
        this->_encryption_key.clear();
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::get_error() const
{
    return (this->_error_code);
}

const char *kv_store::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

