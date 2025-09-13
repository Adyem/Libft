#include "../Encryption/ecc.hpp"
#include "../System_utils/test_runner.hpp"

FT_TEST(test_ect_key_exchange, "ect shared secret")
{
    unsigned char alice_public[ECT_PUBLIC_KEY_SIZE];
    unsigned char alice_private[ECT_PRIVATE_KEY_SIZE];
    unsigned char bob_public[ECT_PUBLIC_KEY_SIZE];
    unsigned char bob_private[ECT_PRIVATE_KEY_SIZE];
    FT_ASSERT_EQ(0, ect_generate_keypair(alice_public, alice_private));
    FT_ASSERT_EQ(0, ect_generate_keypair(bob_public, bob_private));
    unsigned char alice_shared[ECT_SHARED_SECRET_SIZE];
    unsigned char bob_shared[ECT_SHARED_SECRET_SIZE];
    FT_ASSERT_EQ(0, ect_compute_shared_secret(alice_shared, alice_private, bob_public));
    FT_ASSERT_EQ(0, ect_compute_shared_secret(bob_shared, bob_private, alice_public));
    std::size_t index = 0;
    while (index < ECT_SHARED_SECRET_SIZE)
    {
        if (alice_shared[index] != bob_shared[index])
            return (0);
        index = index + 1;
    }
    return (1);
}

FT_TEST(test_ect_sign_verify, "ect sign verify")
{
    unsigned char public_key[ECT_PUBLIC_KEY_SIZE];
    unsigned char private_key[ECT_PRIVATE_KEY_SIZE];
    FT_ASSERT_EQ(0, ect_generate_keypair(public_key, private_key));
    const unsigned char message[] = "hello";
    unsigned char signature[ECT_SIGNATURE_SIZE];
    FT_ASSERT_EQ(0, ect_sign(private_key, message, 5, signature));
    FT_ASSERT_EQ(0, ect_verify(public_key, message, 5, signature));
    return (1);
}
