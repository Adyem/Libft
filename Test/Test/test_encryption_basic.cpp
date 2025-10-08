#include "../../Encryption/basic_encryption.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdio>

static int mock_open_failure(const char *path_name, int flags, mode_t mode)
{
    (void)path_name;
    (void)flags;
    (void)mode;
    ft_errno = FT_ERR_IO;
    return (-1);
}

static ssize_t mock_write_failure(int file_descriptor, const void *buffer, size_t count)
{
    (void)file_descriptor;
    (void)buffer;
    (void)count;
    ft_errno = FT_ERR_IO;
    return (-1);
}

FT_TEST(test_be_get_encryption_key_allocation_failure_sets_errno,
    "be_getEncryptionKey reports allocator failure")
{
    const char *key;

    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(1);
    key = be_getEncryptionKey();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, key);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_be_save_game_allocation_failure_sets_errno,
    "be_saveGame reports allocator failure")
{
    int save_result;

    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(1);
    save_result = be_saveGame("be_save_game_alloc.txt", "data", "key");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(1, save_result);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_be_save_game_open_failure_preserves_errno,
    "be_saveGame keeps su_open error code")
{
    int save_result;

    ft_errno = ER_SUCCESS;
    be_set_save_game_hooks(mock_open_failure, ft_nullptr);
    save_result = be_saveGame("be_save_game_open.txt", "data", "key");
    be_set_save_game_hooks(ft_nullptr, ft_nullptr);
    FT_ASSERT_EQ(1, save_result);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    return (1);
}

FT_TEST(test_be_save_game_write_failure_preserves_errno,
    "be_saveGame keeps su_write error code")
{
    int save_result;

    ft_errno = ER_SUCCESS;
    be_set_save_game_hooks(ft_nullptr, mock_write_failure);
    save_result = be_saveGame("be_save_game_write.txt", "data", "key");
    be_set_save_game_hooks(ft_nullptr, ft_nullptr);
    std::remove("be_save_game_write.txt");
    FT_ASSERT_EQ(1, save_result);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    return (1);
}

FT_TEST(test_be_decrypt_data_null_input_sets_errno,
    "be_DecryptData rejects null buffers")
{
    char *missing_buffer = ft_nullptr;
    char **wrapper = &missing_buffer;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, be_DecryptData(ft_nullptr, "key"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, be_DecryptData(wrapper, "key"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}
