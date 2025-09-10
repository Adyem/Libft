#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../Compatebility/file.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "basic_encryption.hpp"

static void be_encrypt(char *data, size_t data_length, const char *key)
{
    uint32_t hash = 5381;
    size_t key_length = ft_strlen(key);
    size_t key_index = 0;
    while (key_index < key_length)
    {
        hash = ((hash << 5) + hash) + static_cast<uint8_t>(key[key_index]);
        ++key_index;
    }
    size_t data_index = 0;
    while (data_index < data_length)
    {
        data[data_index] ^= static_cast<char>((hash >> (data_index % 8)) & 0xFF);
        ++data_index;
    }
    return ;
}

int be_saveGame(const char *filename, const char *data, const char *key)
{
    size_t data_length = ft_strlen(data);
    char *encrypted_data = static_cast<char *>(cma_malloc(data_length));
    if (!encrypted_data)
        return (1);
    ft_memcpy(encrypted_data, data, data_length);
    be_encrypt(encrypted_data, data_length, key);
    int file_descriptor = ft_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0)
    {
        cma_free(encrypted_data);
        return (1);
    }
    ssize_t bytes_written = ft_write(file_descriptor, encrypted_data, static_cast<int>(data_length));
    ft_close(file_descriptor);
    cma_free(encrypted_data);
    if (bytes_written == static_cast<ssize_t>(data_length))
        return (0);
    return (1);
}

char **be_DecryptData(char **data, const char *key)
{
    if (!data || !*data)
        return (ft_nullptr);
    size_t data_length = ft_strlen(*data);
    be_encrypt(*data, data_length, key);
    return (data);
}
