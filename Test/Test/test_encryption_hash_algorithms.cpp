#include "../../Encryption/encryption_blake2.hpp"
#include "../../Encryption/encryption_sha3.hpp"
#include "../../Encryption/encryption_hmac_sha256.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

static int assert_digest_matches(const unsigned char *actual,
    const unsigned char *expected, size_t length)
{
    size_t index;

    index = 0;
    while (index < length)
    {
        FT_ASSERT_EQ(static_cast<int>(expected[index]),
            static_cast<int>(actual[index]));
        index++;
    }
    return (1);
}

FT_TEST(test_blake2_hash_matches_known_vectors,
    "blake2 hash helpers match published digests")
{
    unsigned char digest[64];
    unsigned char expected_empty[64];
    unsigned char expected_abc[64];
    unsigned char digest32[32];
    unsigned char expected_s_empty[32];
    unsigned char expected_s_abc[32];

    expected_empty[0] = 0x78;
    expected_empty[1] = 0x6A;
    expected_empty[2] = 0x02;
    expected_empty[3] = 0xF7;
    expected_empty[4] = 0x42;
    expected_empty[5] = 0x01;
    expected_empty[6] = 0x59;
    expected_empty[7] = 0x03;
    expected_empty[8] = 0xC6;
    expected_empty[9] = 0xC6;
    expected_empty[10] = 0xFD;
    expected_empty[11] = 0x85;
    expected_empty[12] = 0x25;
    expected_empty[13] = 0x52;
    expected_empty[14] = 0xD2;
    expected_empty[15] = 0x72;
    expected_empty[16] = 0x91;
    expected_empty[17] = 0x2F;
    expected_empty[18] = 0x47;
    expected_empty[19] = 0x40;
    expected_empty[20] = 0xE1;
    expected_empty[21] = 0x58;
    expected_empty[22] = 0x47;
    expected_empty[23] = 0x61;
    expected_empty[24] = 0x8A;
    expected_empty[25] = 0x86;
    expected_empty[26] = 0xE2;
    expected_empty[27] = 0x17;
    expected_empty[28] = 0xF7;
    expected_empty[29] = 0x1F;
    expected_empty[30] = 0x54;
    expected_empty[31] = 0x19;
    expected_empty[32] = 0xD2;
    expected_empty[33] = 0x5E;
    expected_empty[34] = 0x10;
    expected_empty[35] = 0x31;
    expected_empty[36] = 0xAF;
    expected_empty[37] = 0xEE;
    expected_empty[38] = 0x58;
    expected_empty[39] = 0x53;
    expected_empty[40] = 0x13;
    expected_empty[41] = 0x89;
    expected_empty[42] = 0x64;
    expected_empty[43] = 0x44;
    expected_empty[44] = 0x93;
    expected_empty[45] = 0x4E;
    expected_empty[46] = 0xB0;
    expected_empty[47] = 0x4B;
    expected_empty[48] = 0x90;
    expected_empty[49] = 0x3A;
    expected_empty[50] = 0x68;
    expected_empty[51] = 0x5B;
    expected_empty[52] = 0x14;
    expected_empty[53] = 0x48;
    expected_empty[54] = 0xB7;
    expected_empty[55] = 0x55;
    expected_empty[56] = 0xD5;
    expected_empty[57] = 0x6F;
    expected_empty[58] = 0x70;
    expected_empty[59] = 0x1A;
    expected_empty[60] = 0xFE;
    expected_empty[61] = 0x9B;
    expected_empty[62] = 0xE2;
    expected_empty[63] = 0xCE;

    expected_abc[0] = 0xBA;
    expected_abc[1] = 0x80;
    expected_abc[2] = 0xA5;
    expected_abc[3] = 0x3F;
    expected_abc[4] = 0x98;
    expected_abc[5] = 0x1C;
    expected_abc[6] = 0x4D;
    expected_abc[7] = 0x0D;
    expected_abc[8] = 0x6A;
    expected_abc[9] = 0x27;
    expected_abc[10] = 0x97;
    expected_abc[11] = 0xB6;
    expected_abc[12] = 0x9F;
    expected_abc[13] = 0x12;
    expected_abc[14] = 0xF6;
    expected_abc[15] = 0xE9;
    expected_abc[16] = 0x4C;
    expected_abc[17] = 0x21;
    expected_abc[18] = 0x2F;
    expected_abc[19] = 0x14;
    expected_abc[20] = 0x68;
    expected_abc[21] = 0x5A;
    expected_abc[22] = 0xC4;
    expected_abc[23] = 0xB7;
    expected_abc[24] = 0x4B;
    expected_abc[25] = 0x12;
    expected_abc[26] = 0xBB;
    expected_abc[27] = 0x6F;
    expected_abc[28] = 0xDB;
    expected_abc[29] = 0xFF;
    expected_abc[30] = 0xA2;
    expected_abc[31] = 0xD1;
    expected_abc[32] = 0x7D;
    expected_abc[33] = 0x87;
    expected_abc[34] = 0xC5;
    expected_abc[35] = 0x39;
    expected_abc[36] = 0x2A;
    expected_abc[37] = 0xAB;
    expected_abc[38] = 0x79;
    expected_abc[39] = 0x2D;
    expected_abc[40] = 0xC2;
    expected_abc[41] = 0x52;
    expected_abc[42] = 0xD5;
    expected_abc[43] = 0xDE;
    expected_abc[44] = 0x45;
    expected_abc[45] = 0x33;
    expected_abc[46] = 0xCC;
    expected_abc[47] = 0x95;
    expected_abc[48] = 0x18;
    expected_abc[49] = 0xD3;
    expected_abc[50] = 0x8A;
    expected_abc[51] = 0xA8;
    expected_abc[52] = 0xDB;
    expected_abc[53] = 0xF1;
    expected_abc[54] = 0x92;
    expected_abc[55] = 0x5A;
    expected_abc[56] = 0xB9;
    expected_abc[57] = 0x23;
    expected_abc[58] = 0x86;
    expected_abc[59] = 0xED;
    expected_abc[60] = 0xD4;
    expected_abc[61] = 0x00;
    expected_abc[62] = 0x99;
    expected_abc[63] = 0x23;

    expected_s_empty[0] = 0x69;
    expected_s_empty[1] = 0x21;
    expected_s_empty[2] = 0x7A;
    expected_s_empty[3] = 0x30;
    expected_s_empty[4] = 0x79;
    expected_s_empty[5] = 0x90;
    expected_s_empty[6] = 0x80;
    expected_s_empty[7] = 0x94;
    expected_s_empty[8] = 0xE1;
    expected_s_empty[9] = 0x11;
    expected_s_empty[10] = 0x21;
    expected_s_empty[11] = 0xD0;
    expected_s_empty[12] = 0x42;
    expected_s_empty[13] = 0x35;
    expected_s_empty[14] = 0x4A;
    expected_s_empty[15] = 0x7C;
    expected_s_empty[16] = 0x1F;
    expected_s_empty[17] = 0x55;
    expected_s_empty[18] = 0xB6;
    expected_s_empty[19] = 0x48;
    expected_s_empty[20] = 0x2C;
    expected_s_empty[21] = 0xA1;
    expected_s_empty[22] = 0xA5;
    expected_s_empty[23] = 0x1E;
    expected_s_empty[24] = 0x1B;
    expected_s_empty[25] = 0x25;
    expected_s_empty[26] = 0x0D;
    expected_s_empty[27] = 0xFD;
    expected_s_empty[28] = 0x1E;
    expected_s_empty[29] = 0xD0;
    expected_s_empty[30] = 0xEE;
    expected_s_empty[31] = 0xF9;

    expected_s_abc[0] = 0x50;
    expected_s_abc[1] = 0x8C;
    expected_s_abc[2] = 0x5E;
    expected_s_abc[3] = 0x8C;
    expected_s_abc[4] = 0x32;
    expected_s_abc[5] = 0x7C;
    expected_s_abc[6] = 0x14;
    expected_s_abc[7] = 0xE2;
    expected_s_abc[8] = 0xE1;
    expected_s_abc[9] = 0xA7;
    expected_s_abc[10] = 0x2B;
    expected_s_abc[11] = 0xA3;
    expected_s_abc[12] = 0x4E;
    expected_s_abc[13] = 0xEB;
    expected_s_abc[14] = 0x45;
    expected_s_abc[15] = 0x2F;
    expected_s_abc[16] = 0x37;
    expected_s_abc[17] = 0x45;
    expected_s_abc[18] = 0x8B;
    expected_s_abc[19] = 0x20;
    expected_s_abc[20] = 0x9E;
    expected_s_abc[21] = 0xD6;
    expected_s_abc[22] = 0x3A;
    expected_s_abc[23] = 0x29;
    expected_s_abc[24] = 0x4D;
    expected_s_abc[25] = 0x99;
    expected_s_abc[26] = 0x9B;
    expected_s_abc[27] = 0x4C;
    expected_s_abc[28] = 0x86;
    expected_s_abc[29] = 0x67;
    expected_s_abc[30] = 0x59;
    expected_s_abc[31] = 0x82;

    ft_errno = FT_ERR_INTERNAL;
    blake2b_hash("", 0, digest, 64);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    assert_digest_matches(digest, expected_empty, 64);

    ft_errno = FT_ERR_INTERNAL;
    blake2b_hash("abc", 3, digest, 64);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    assert_digest_matches(digest, expected_abc, 64);

    ft_errno = FT_ERR_INTERNAL;
    blake2s_hash("", 0, digest32, 32);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    assert_digest_matches(digest32, expected_s_empty, 32);

    ft_errno = FT_ERR_INTERNAL;
    blake2s_hash("abc", 3, digest32, 32);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    assert_digest_matches(digest32, expected_s_abc, 32);
    return (1);
}

FT_TEST(test_sha3_hash_matches_known_vectors,
    "sha3 hash helpers match published digests")
{
    unsigned char digest256[32];
    unsigned char digest512[64];
    unsigned char expected256_empty[32];
    unsigned char expected256_abc[32];
    unsigned char expected512_empty[64];
    unsigned char expected512_abc[64];

    expected256_empty[0] = 0xA7;
    expected256_empty[1] = 0xFF;
    expected256_empty[2] = 0xC6;
    expected256_empty[3] = 0xF8;
    expected256_empty[4] = 0xBF;
    expected256_empty[5] = 0x1E;
    expected256_empty[6] = 0xD7;
    expected256_empty[7] = 0x66;
    expected256_empty[8] = 0x51;
    expected256_empty[9] = 0xC1;
    expected256_empty[10] = 0x47;
    expected256_empty[11] = 0x56;
    expected256_empty[12] = 0xA0;
    expected256_empty[13] = 0x61;
    expected256_empty[14] = 0xD6;
    expected256_empty[15] = 0x62;
    expected256_empty[16] = 0xF5;
    expected256_empty[17] = 0x80;
    expected256_empty[18] = 0xFF;
    expected256_empty[19] = 0x4D;
    expected256_empty[20] = 0xE4;
    expected256_empty[21] = 0x3B;
    expected256_empty[22] = 0x49;
    expected256_empty[23] = 0xFA;
    expected256_empty[24] = 0x82;
    expected256_empty[25] = 0xD8;
    expected256_empty[26] = 0x0A;
    expected256_empty[27] = 0x4B;
    expected256_empty[28] = 0x80;
    expected256_empty[29] = 0xF8;
    expected256_empty[30] = 0x43;
    expected256_empty[31] = 0x4A;

    expected256_abc[0] = 0x3A;
    expected256_abc[1] = 0x98;
    expected256_abc[2] = 0x5D;
    expected256_abc[3] = 0xA7;
    expected256_abc[4] = 0x4F;
    expected256_abc[5] = 0xE2;
    expected256_abc[6] = 0x25;
    expected256_abc[7] = 0xB2;
    expected256_abc[8] = 0x04;
    expected256_abc[9] = 0x5C;
    expected256_abc[10] = 0x17;
    expected256_abc[11] = 0x2D;
    expected256_abc[12] = 0x6B;
    expected256_abc[13] = 0xD3;
    expected256_abc[14] = 0x90;
    expected256_abc[15] = 0xBD;
    expected256_abc[16] = 0x85;
    expected256_abc[17] = 0x5F;
    expected256_abc[18] = 0x08;
    expected256_abc[19] = 0x6E;
    expected256_abc[20] = 0x3E;
    expected256_abc[21] = 0x9D;
    expected256_abc[22] = 0x52;
    expected256_abc[23] = 0x5B;
    expected256_abc[24] = 0x46;
    expected256_abc[25] = 0xBF;
    expected256_abc[26] = 0xE2;
    expected256_abc[27] = 0x45;
    expected256_abc[28] = 0x11;
    expected256_abc[29] = 0x43;
    expected256_abc[30] = 0x15;
    expected256_abc[31] = 0x32;

    expected512_empty[0] = 0xA6;
    expected512_empty[1] = 0x9F;
    expected512_empty[2] = 0x73;
    expected512_empty[3] = 0xCC;
    expected512_empty[4] = 0xA2;
    expected512_empty[5] = 0x3A;
    expected512_empty[6] = 0x9A;
    expected512_empty[7] = 0xC5;
    expected512_empty[8] = 0xC8;
    expected512_empty[9] = 0xB5;
    expected512_empty[10] = 0x67;
    expected512_empty[11] = 0xDC;
    expected512_empty[12] = 0x18;
    expected512_empty[13] = 0x5A;
    expected512_empty[14] = 0x75;
    expected512_empty[15] = 0x6E;
    expected512_empty[16] = 0x97;
    expected512_empty[17] = 0xC9;
    expected512_empty[18] = 0x82;
    expected512_empty[19] = 0x16;
    expected512_empty[20] = 0x4F;
    expected512_empty[21] = 0xE2;
    expected512_empty[22] = 0x58;
    expected512_empty[23] = 0x59;
    expected512_empty[24] = 0xE0;
    expected512_empty[25] = 0xD1;
    expected512_empty[26] = 0xDC;
    expected512_empty[27] = 0xC1;
    expected512_empty[28] = 0x47;
    expected512_empty[29] = 0x5C;
    expected512_empty[30] = 0x80;
    expected512_empty[31] = 0xA6;
    expected512_empty[32] = 0x15;
    expected512_empty[33] = 0xB2;
    expected512_empty[34] = 0x12;
    expected512_empty[35] = 0x3A;
    expected512_empty[36] = 0xF1;
    expected512_empty[37] = 0xF5;
    expected512_empty[38] = 0xF9;
    expected512_empty[39] = 0x4C;
    expected512_empty[40] = 0x11;
    expected512_empty[41] = 0xE3;
    expected512_empty[42] = 0xE9;
    expected512_empty[43] = 0x40;
    expected512_empty[44] = 0x2C;
    expected512_empty[45] = 0x3A;
    expected512_empty[46] = 0xC5;
    expected512_empty[47] = 0x58;
    expected512_empty[48] = 0xF5;
    expected512_empty[49] = 0x00;
    expected512_empty[50] = 0x19;
    expected512_empty[51] = 0x9D;
    expected512_empty[52] = 0x95;
    expected512_empty[53] = 0xB6;
    expected512_empty[54] = 0xD3;
    expected512_empty[55] = 0xE3;
    expected512_empty[56] = 0x01;
    expected512_empty[57] = 0x75;
    expected512_empty[58] = 0x85;
    expected512_empty[59] = 0x86;
    expected512_empty[60] = 0x28;
    expected512_empty[61] = 0x1D;
    expected512_empty[62] = 0xCD;
    expected512_empty[63] = 0x26;

    expected512_abc[0] = 0xB7;
    expected512_abc[1] = 0x51;
    expected512_abc[2] = 0x85;
    expected512_abc[3] = 0x0B;
    expected512_abc[4] = 0x1A;
    expected512_abc[5] = 0x57;
    expected512_abc[6] = 0x16;
    expected512_abc[7] = 0x8A;
    expected512_abc[8] = 0x56;
    expected512_abc[9] = 0x93;
    expected512_abc[10] = 0xCD;
    expected512_abc[11] = 0x92;
    expected512_abc[12] = 0x4B;
    expected512_abc[13] = 0x6B;
    expected512_abc[14] = 0x09;
    expected512_abc[15] = 0x6E;
    expected512_abc[16] = 0x08;
    expected512_abc[17] = 0xF6;
    expected512_abc[18] = 0x21;
    expected512_abc[19] = 0x82;
    expected512_abc[20] = 0x74;
    expected512_abc[21] = 0x44;
    expected512_abc[22] = 0xF7;
    expected512_abc[23] = 0x0D;
    expected512_abc[24] = 0x88;
    expected512_abc[25] = 0x4F;
    expected512_abc[26] = 0x5D;
    expected512_abc[27] = 0x02;
    expected512_abc[28] = 0x40;
    expected512_abc[29] = 0xD2;
    expected512_abc[30] = 0x71;
    expected512_abc[31] = 0x2E;
    expected512_abc[32] = 0x10;
    expected512_abc[33] = 0xE1;
    expected512_abc[34] = 0x16;
    expected512_abc[35] = 0xE9;
    expected512_abc[36] = 0x19;
    expected512_abc[37] = 0x2A;
    expected512_abc[38] = 0xF3;
    expected512_abc[39] = 0xC9;
    expected512_abc[40] = 0x1A;
    expected512_abc[41] = 0x7E;
    expected512_abc[42] = 0xC5;
    expected512_abc[43] = 0x76;
    expected512_abc[44] = 0x47;
    expected512_abc[45] = 0xE3;
    expected512_abc[46] = 0x93;
    expected512_abc[47] = 0x40;
    expected512_abc[48] = 0x57;
    expected512_abc[49] = 0x34;
    expected512_abc[50] = 0x0B;
    expected512_abc[51] = 0x4C;
    expected512_abc[52] = 0xF4;
    expected512_abc[53] = 0x08;
    expected512_abc[54] = 0xD5;
    expected512_abc[55] = 0xA5;
    expected512_abc[56] = 0x65;
    expected512_abc[57] = 0x92;
    expected512_abc[58] = 0xF8;
    expected512_abc[59] = 0x27;
    expected512_abc[60] = 0x4E;
    expected512_abc[61] = 0xEC;
    expected512_abc[62] = 0x53;
    expected512_abc[63] = 0xF0;

    ft_errno = FT_ERR_INTERNAL;
    sha3_256_hash("", 0, digest256);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    assert_digest_matches(digest256, expected256_empty, 32);

    ft_errno = FT_ERR_INTERNAL;
    sha3_256_hash("abc", 3, digest256);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    assert_digest_matches(digest256, expected256_abc, 32);

    ft_errno = FT_ERR_INTERNAL;
    sha3_512_hash("", 0, digest512);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    assert_digest_matches(digest512, expected512_empty, 64);

    ft_errno = FT_ERR_INTERNAL;
    sha3_512_hash("abc", 3, digest512);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    assert_digest_matches(digest512, expected512_abc, 64);
    return (1);
}

FT_TEST(test_hmac_sha256_streaming_matches_single_shot,
    "streaming hmac matches one-shot helper")
{
    const unsigned char key[] = { 's', 'e', 'c', 'r', 'e', 't' };
    const char *part_one = "streaming ";
    const char *part_two = "message ";
    const char *part_three = "example";
    const char *full_message = "streaming message example";
    size_t full_length;
    unsigned char expected_digest[32];
    unsigned char streaming_digest[32];
    hmac_sha256_stream stream;

    full_length = ft_strlen(full_message);
    ft_errno = FT_ERR_INTERNAL;
    hmac_sha256(key, sizeof(key), full_message, full_length, expected_digest);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    FT_ASSERT_EQ(0, hmac_sha256_stream_init(stream, key, sizeof(key)));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    FT_ASSERT_EQ(0, hmac_sha256_stream_update(stream, part_one,
            ft_strlen(part_one)));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    FT_ASSERT_EQ(0, hmac_sha256_stream_update(stream, part_two,
            ft_strlen(part_two)));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    FT_ASSERT_EQ(0, hmac_sha256_stream_update(stream, part_three,
            ft_strlen(part_three)));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    FT_ASSERT_EQ(0, hmac_sha256_stream_final(stream, streaming_digest, 32));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    hmac_sha256_stream_cleanup(stream);

    assert_digest_matches(streaming_digest, expected_digest, 32);
    return (1);
}
