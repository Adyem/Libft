#include "file_utils.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Encryption/encryption.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static int32_t file_hash_file_contents(const char *path, ft_string *contents)
{
    int32_t error_code;

    if (path == ft_nullptr || contents == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    error_code = contents->initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = file_read_all(path, *contents);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)contents->destroy();
        return (error_code);
    }
    return (FT_ERR_SUCCESS);
}

int32_t file_hash_sha1(const char *path, uint8_t digest[20])
{
    ft_string contents;
    const void *data_pointer;
    int32_t error_code;

    if (path == ft_nullptr || digest == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    error_code = file_hash_file_contents(path, &contents);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    data_pointer = ft_nullptr;
    if (contents.size() > 0)
        data_pointer = contents.data();
    sha1_hash(data_pointer, contents.size(), digest);
    (void)contents.destroy();
    return (FT_ERR_SUCCESS);
}

int32_t file_hash_sha256(const char *path, uint8_t digest[32])
{
    ft_string contents;
    const void *data_pointer;
    int32_t error_code;

    if (path == ft_nullptr || digest == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    error_code = file_hash_file_contents(path, &contents);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    data_pointer = ft_nullptr;
    if (contents.size() > 0)
        data_pointer = contents.data();
    sha256_hash(data_pointer, contents.size(), digest);
    (void)contents.destroy();
    return (FT_ERR_SUCCESS);
}
