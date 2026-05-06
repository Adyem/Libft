#include "../test_internal.hpp"
#include "../../Modules/Encryption/encryption.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_encryption_secure_wipe_null_buffer_sets_errno)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
            encryption_secure_wipe(ft_nullptr, 4));
    return (1);
}

FT_TEST(test_encryption_secure_wipe_zero_size_clears_errno)
{
    unsigned char data_buffer;

    data_buffer = 42;
    FT_ASSERT_EQ(0, encryption_secure_wipe(&data_buffer, 0));
    FT_ASSERT_EQ(static_cast<unsigned char>(42), data_buffer);
    return (1);
}

FT_TEST(test_encryption_secure_wipe_overwrites_bytes)
{
    unsigned char data_buffer[8];
    std::size_t buffer_index;

    buffer_index = 0;
    while (buffer_index < 8)
    {
        data_buffer[buffer_index] = static_cast<unsigned char>(0xAB);
        buffer_index++;
    }
    FT_ASSERT_EQ(0, encryption_secure_wipe(data_buffer, 8));
    buffer_index = 0;
    while (buffer_index < 8)
    {
        FT_ASSERT_EQ(static_cast<unsigned char>(0), data_buffer[buffer_index]);
        buffer_index++;
    }
    return (1);
}

FT_TEST(test_encryption_secure_wipe_string_clears_contents)
{
    char secret_buffer[7];
    std::size_t buffer_index;

    secret_buffer[0] = 's';
    secret_buffer[1] = 'e';
    secret_buffer[2] = 'c';
    secret_buffer[3] = 'r';
    secret_buffer[4] = 'e';
    secret_buffer[5] = 't';
    secret_buffer[6] = '\0';
    FT_ASSERT_EQ(0, encryption_secure_wipe_string(secret_buffer));
    buffer_index = 0;
    while (buffer_index < 7)
    {
        FT_ASSERT_EQ('\0', secret_buffer[buffer_index]);
        buffer_index++;
    }
    return (1);
}

FT_TEST(test_encryption_secure_wipe_string_null_buffer_sets_errno)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
            encryption_secure_wipe_string(ft_nullptr));
    return (1);
}
