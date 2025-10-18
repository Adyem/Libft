#include "../../Encryption/encryption_sha1.hpp"
#include "../../Encryption/encryption_sha256.hpp"
#include "../../Encryption/encryption_hmac_sha256.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_sha1_hash_matches_known_vectors,
    "sha1_hash matches known digest outputs")
{
    unsigned char digest[20];
    unsigned char empty_expected[20];
    unsigned char abc_expected[20];
    unsigned char pangram_expected[20];
    size_t digest_index;

    empty_expected[0] = 0xDA;
    empty_expected[1] = 0x39;
    empty_expected[2] = 0xA3;
    empty_expected[3] = 0xEE;
    empty_expected[4] = 0x5E;
    empty_expected[5] = 0x6B;
    empty_expected[6] = 0x4B;
    empty_expected[7] = 0x0D;
    empty_expected[8] = 0x32;
    empty_expected[9] = 0x55;
    empty_expected[10] = 0xBF;
    empty_expected[11] = 0xEF;
    empty_expected[12] = 0x95;
    empty_expected[13] = 0x60;
    empty_expected[14] = 0x18;
    empty_expected[15] = 0x90;
    empty_expected[16] = 0xAF;
    empty_expected[17] = 0xD8;
    empty_expected[18] = 0x07;
    empty_expected[19] = 0x09;

    abc_expected[0] = 0xA9;
    abc_expected[1] = 0x99;
    abc_expected[2] = 0x3E;
    abc_expected[3] = 0x36;
    abc_expected[4] = 0x47;
    abc_expected[5] = 0x06;
    abc_expected[6] = 0x81;
    abc_expected[7] = 0x6A;
    abc_expected[8] = 0xBA;
    abc_expected[9] = 0x3E;
    abc_expected[10] = 0x25;
    abc_expected[11] = 0x71;
    abc_expected[12] = 0x78;
    abc_expected[13] = 0x50;
    abc_expected[14] = 0xC2;
    abc_expected[15] = 0x6C;
    abc_expected[16] = 0x9C;
    abc_expected[17] = 0xD0;
    abc_expected[18] = 0xD8;
    abc_expected[19] = 0x9D;

    pangram_expected[0] = 0x2F;
    pangram_expected[1] = 0xD4;
    pangram_expected[2] = 0xE1;
    pangram_expected[3] = 0xC6;
    pangram_expected[4] = 0x7A;
    pangram_expected[5] = 0x2D;
    pangram_expected[6] = 0x28;
    pangram_expected[7] = 0xFC;
    pangram_expected[8] = 0xED;
    pangram_expected[9] = 0x84;
    pangram_expected[10] = 0x9E;
    pangram_expected[11] = 0xE1;
    pangram_expected[12] = 0xBB;
    pangram_expected[13] = 0x76;
    pangram_expected[14] = 0xE7;
    pangram_expected[15] = 0x39;
    pangram_expected[16] = 0x1B;
    pangram_expected[17] = 0x93;
    pangram_expected[18] = 0xEB;
    pangram_expected[19] = 0x12;

    ft_errno = FT_ERR_IO;
    sha1_hash("", 0, digest);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    digest_index = 0;
    while (digest_index < 20)
    {
        FT_ASSERT_EQ(static_cast<int>(empty_expected[digest_index]), static_cast<int>(digest[digest_index]));
        digest_index++;
    }

    ft_errno = FT_ERR_IO;
    sha1_hash("abc", 3, digest);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    digest_index = 0;
    while (digest_index < 20)
    {
        FT_ASSERT_EQ(static_cast<int>(abc_expected[digest_index]), static_cast<int>(digest[digest_index]));
        digest_index++;
    }

    ft_errno = FT_ERR_IO;
    sha1_hash("The quick brown fox jumps over the lazy dog", 43, digest);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    digest_index = 0;
    while (digest_index < 20)
    {
        FT_ASSERT_EQ(static_cast<int>(pangram_expected[digest_index]), static_cast<int>(digest[digest_index]));
        digest_index++;
    }
    return (1);
}

FT_TEST(test_sha256_hash_matches_known_vectors,
    "sha256_hash matches known digest outputs")
{
    unsigned char digest[32];
    unsigned char empty_expected[32];
    unsigned char abc_expected[32];
    unsigned char pangram_expected[32];
    size_t digest_index;

    empty_expected[0] = 0xE3;
    empty_expected[1] = 0xB0;
    empty_expected[2] = 0xC4;
    empty_expected[3] = 0x42;
    empty_expected[4] = 0x98;
    empty_expected[5] = 0xFC;
    empty_expected[6] = 0x1C;
    empty_expected[7] = 0x14;
    empty_expected[8] = 0x9A;
    empty_expected[9] = 0xFB;
    empty_expected[10] = 0xF4;
    empty_expected[11] = 0xC8;
    empty_expected[12] = 0x99;
    empty_expected[13] = 0x6F;
    empty_expected[14] = 0xB9;
    empty_expected[15] = 0x24;
    empty_expected[16] = 0x27;
    empty_expected[17] = 0xAE;
    empty_expected[18] = 0x41;
    empty_expected[19] = 0xE4;
    empty_expected[20] = 0x64;
    empty_expected[21] = 0x9B;
    empty_expected[22] = 0x93;
    empty_expected[23] = 0x4C;
    empty_expected[24] = 0xA4;
    empty_expected[25] = 0x95;
    empty_expected[26] = 0x99;
    empty_expected[27] = 0x1B;
    empty_expected[28] = 0x78;
    empty_expected[29] = 0x52;
    empty_expected[30] = 0xB8;
    empty_expected[31] = 0x55;

    abc_expected[0] = 0xBA;
    abc_expected[1] = 0x78;
    abc_expected[2] = 0x16;
    abc_expected[3] = 0xBF;
    abc_expected[4] = 0x8F;
    abc_expected[5] = 0x01;
    abc_expected[6] = 0xCF;
    abc_expected[7] = 0xEA;
    abc_expected[8] = 0x41;
    abc_expected[9] = 0x41;
    abc_expected[10] = 0x40;
    abc_expected[11] = 0xDE;
    abc_expected[12] = 0x5D;
    abc_expected[13] = 0xAE;
    abc_expected[14] = 0x22;
    abc_expected[15] = 0x23;
    abc_expected[16] = 0xB0;
    abc_expected[17] = 0x03;
    abc_expected[18] = 0x61;
    abc_expected[19] = 0xA3;
    abc_expected[20] = 0x96;
    abc_expected[21] = 0x17;
    abc_expected[22] = 0x7A;
    abc_expected[23] = 0x9C;
    abc_expected[24] = 0xB4;
    abc_expected[25] = 0x10;
    abc_expected[26] = 0xFF;
    abc_expected[27] = 0x61;
    abc_expected[28] = 0xF2;
    abc_expected[29] = 0x00;
    abc_expected[30] = 0x15;
    abc_expected[31] = 0xAD;

    pangram_expected[0] = 0xD7;
    pangram_expected[1] = 0xA8;
    pangram_expected[2] = 0xFB;
    pangram_expected[3] = 0xB3;
    pangram_expected[4] = 0x07;
    pangram_expected[5] = 0xD7;
    pangram_expected[6] = 0x80;
    pangram_expected[7] = 0x94;
    pangram_expected[8] = 0x69;
    pangram_expected[9] = 0xCA;
    pangram_expected[10] = 0x9A;
    pangram_expected[11] = 0xBC;
    pangram_expected[12] = 0xB0;
    pangram_expected[13] = 0x08;
    pangram_expected[14] = 0x2E;
    pangram_expected[15] = 0x4F;
    pangram_expected[16] = 0x8D;
    pangram_expected[17] = 0x56;
    pangram_expected[18] = 0x51;
    pangram_expected[19] = 0xE4;
    pangram_expected[20] = 0x6D;
    pangram_expected[21] = 0x3C;
    pangram_expected[22] = 0xDB;
    pangram_expected[23] = 0x76;
    pangram_expected[24] = 0x2D;
    pangram_expected[25] = 0x02;
    pangram_expected[26] = 0xD0;
    pangram_expected[27] = 0xBF;
    pangram_expected[28] = 0x37;
    pangram_expected[29] = 0xC9;
    pangram_expected[30] = 0xE5;
    pangram_expected[31] = 0x92;

    ft_errno = FT_ERR_IO;
    sha256_hash("", 0, digest);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    digest_index = 0;
    while (digest_index < 32)
    {
        FT_ASSERT_EQ(static_cast<int>(empty_expected[digest_index]), static_cast<int>(digest[digest_index]));
        digest_index++;
    }

    ft_errno = FT_ERR_IO;
    sha256_hash("abc", 3, digest);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    digest_index = 0;
    while (digest_index < 32)
    {
        FT_ASSERT_EQ(static_cast<int>(abc_expected[digest_index]), static_cast<int>(digest[digest_index]));
        digest_index++;
    }

    ft_errno = FT_ERR_IO;
    sha256_hash("The quick brown fox jumps over the lazy dog", 43, digest);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    digest_index = 0;
    while (digest_index < 32)
    {
        FT_ASSERT_EQ(static_cast<int>(pangram_expected[digest_index]), static_cast<int>(digest[digest_index]));
        digest_index++;
    }
    return (1);
}

FT_TEST(test_hmac_sha256_matches_known_vector,
    "hmac_sha256 matches known digest output")
{
    unsigned char digest[32];
    unsigned char expected_digest[32];
    const unsigned char *key;
    const char *message;
    size_t digest_index;

    expected_digest[0] = 0xF7;
    expected_digest[1] = 0xBC;
    expected_digest[2] = 0x83;
    expected_digest[3] = 0xF4;
    expected_digest[4] = 0x30;
    expected_digest[5] = 0x53;
    expected_digest[6] = 0x84;
    expected_digest[7] = 0x24;
    expected_digest[8] = 0xB1;
    expected_digest[9] = 0x32;
    expected_digest[10] = 0x98;
    expected_digest[11] = 0xE6;
    expected_digest[12] = 0xAA;
    expected_digest[13] = 0x6F;
    expected_digest[14] = 0xB1;
    expected_digest[15] = 0x43;
    expected_digest[16] = 0xEF;
    expected_digest[17] = 0x4D;
    expected_digest[18] = 0x59;
    expected_digest[19] = 0xA1;
    expected_digest[20] = 0x49;
    expected_digest[21] = 0x46;
    expected_digest[22] = 0x17;
    expected_digest[23] = 0x59;
    expected_digest[24] = 0x97;
    expected_digest[25] = 0x47;
    expected_digest[26] = 0x9D;
    expected_digest[27] = 0xBC;
    expected_digest[28] = 0x2D;
    expected_digest[29] = 0x1A;
    expected_digest[30] = 0x3C;
    expected_digest[31] = 0xD8;

    key = reinterpret_cast<const unsigned char *>("key");
    message = "The quick brown fox jumps over the lazy dog";
    ft_errno = FT_ERR_IO;
    hmac_sha256(key, 3, message, 43, digest);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    digest_index = 0;
    while (digest_index < 32)
    {
        FT_ASSERT_EQ(static_cast<int>(expected_digest[digest_index]), static_cast<int>(digest[digest_index]));
        digest_index++;
    }
    return (1);
}
