#include "../../Encryption/encryption_secure_wipe.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_encryption_secure_wipe_null_buffer_sets_errno,
    "encryption_secure_wipe rejects null buffers")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, encryption_secure_wipe(ft_nullptr, 4));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_encryption_secure_wipe_zero_size_clears_errno,
    "encryption_secure_wipe succeeds for zero length")
{
    unsigned char data_buffer;

    data_buffer = 42;
    ft_errno = FT_ERR_IO;
    FT_ASSERT_EQ(0, encryption_secure_wipe(&data_buffer, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(static_cast<unsigned char>(42), data_buffer);
    return (1);
}

FT_TEST(test_encryption_secure_wipe_overwrites_bytes,
    "encryption_secure_wipe zeroes the provided buffer")
{
    unsigned char data_buffer[8];
    std::size_t buffer_index;

    buffer_index = 0;
    while (buffer_index < 8)
    {
        data_buffer[buffer_index] = static_cast<unsigned char>(0xAB);
        buffer_index++;
    }
    ft_errno = FT_ERR_IO;
    FT_ASSERT_EQ(0, encryption_secure_wipe(data_buffer, 8));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    buffer_index = 0;
    while (buffer_index < 8)
    {
        FT_ASSERT_EQ(static_cast<unsigned char>(0), data_buffer[buffer_index]);
        buffer_index++;
    }
    return (1);
}

FT_TEST(test_encryption_secure_wipe_string_clears_contents,
    "encryption_secure_wipe_string erases the string including terminator")
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
    ft_errno = FT_ERR_IO;
    FT_ASSERT_EQ(0, encryption_secure_wipe_string(secret_buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    buffer_index = 0;
    while (buffer_index < 7)
    {
        FT_ASSERT_EQ('\0', secret_buffer[buffer_index]);
        buffer_index++;
    }
    return (1);
}

FT_TEST(test_encryption_secure_wipe_string_null_buffer_sets_errno,
    "encryption_secure_wipe_string rejects null buffers")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, encryption_secure_wipe_string(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}
