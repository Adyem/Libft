#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "basic_encryption.hpp"

static int be_default_open(const char *path_name, int flags, mode_t mode)
{
    return (su_open(path_name, flags, mode));
}

static ssize_t be_default_write(int file_descriptor, const void *buffer, size_t count)
{
    return (su_write(file_descriptor, buffer, count));
}

static t_be_open_function g_be_open_function = be_default_open;
static t_be_write_function g_be_write_function = be_default_write;

void be_set_save_game_hooks(t_be_open_function open_function, t_be_write_function write_function)
{
    if (open_function != ft_nullptr)
        g_be_open_function = open_function;
    else
        g_be_open_function = be_default_open;
    if (write_function != ft_nullptr)
        g_be_write_function = write_function;
    else
        g_be_write_function = be_default_write;
    return ;
}

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
    if (encrypted_data == ft_nullptr)
    {
        int error_code;

        error_code = ft_global_error_stack_drop_last_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_NO_MEMORY;
        ft_global_error_stack_push(error_code);
        return (1);
    }
    ft_global_error_stack_drop_last_error();
    ft_memcpy(encrypted_data, data, data_length);
    be_encrypt(encrypted_data, data_length, key);
    int file_descriptor = g_be_open_function(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0)
    {
        int error_code;

        cma_free(encrypted_data);
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_HANDLE;
        ft_global_error_stack_push(error_code);
        return (1);
    }
    ft_global_error_stack_drop_last_error();
    ssize_t bytes_written = g_be_write_function(file_descriptor, encrypted_data, static_cast<int>(data_length));
    int write_error_code;

    write_error_code = ft_global_error_stack_drop_last_error();
    cmp_close(file_descriptor);
    cma_free(encrypted_data);
    if (bytes_written == static_cast<ssize_t>(data_length))
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    if (write_error_code == FT_ERR_SUCCESSS)
        write_error_code = FT_ERR_IO;
    ft_global_error_stack_push(write_error_code);
    return (1);
}

char **be_DecryptData(char **data, const char *key)
{
    if (data == ft_nullptr || *data == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    size_t data_length = ft_strlen(*data);
    be_encrypt(*data, data_length, key);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (data);
}
