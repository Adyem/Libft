#include "../test_internal.hpp"

#ifndef LIBFT_TEST_BUILD
#endif
#define private public
#define protected public
#include "../../Encryption/encryption_aead.hpp"
#undef private
#undef protected
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

static void fill_sequence(unsigned char *buffer, size_t length)
{
    size_t index;
    unsigned char value;

    index = 0;
    value = 1;
    while (index < length)
    {
        buffer[index] = value;
        value = static_cast<unsigned char>(value + 1);
        index++;
    }
    return ;
}

static void wait_for_lock(std::atomic<bool> &flag)
{
    while (!flag.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return ;
}

static long encrypt_small_message(encryption_aead_context &context,
        unsigned char *ciphertext, unsigned char *authentication_tag,
        size_t &output_length)
{
    unsigned char plaintext[6];
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;

    plaintext[0] = 's';
    plaintext[1] = 'e';
    plaintext[2] = 'c';
    plaintext[3] = 'r';
    plaintext[4] = 'e';
    plaintext[5] = 't';
    start_time = std::chrono::steady_clock::now();
    output_length = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.update(plaintext, 6, ciphertext, output_length));
    FT_ASSERT_EQ(static_cast<size_t>(6), output_length);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.finalize(authentication_tag, 16));
    end_time = std::chrono::steady_clock::now();
    return (std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
}

FT_TEST(test_encryption_aead_thread_safe_contexts_have_independent_mutexes,
        "encryption_aead_context mutexes do not interfere between instances")
{
    encryption_aead_context first;
    encryption_aead_context second;
    unsigned char key[16];
    unsigned char iv[12];
    std::atomic<bool> lock_acquired(false);
    std::atomic<bool> release_lock(false);
    std::atomic<int> lock_result(FT_ERR_SUCCESS);
    std::thread locker_thread;
    unsigned char ciphertext[8];
    unsigned char authentication_tag[16];
    size_t output_length;
    long duration_ms;

    fill_sequence(key, 16);
    fill_sequence(iv, 12);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize_encrypt(key, 16, iv, 12));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize_encrypt(key, 16, iv, 12));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.enable_thread_safety());
    locker_thread = std::thread([&first, &lock_acquired, &release_lock, &lock_result]() -> void {
        pt_recursive_mutex *mutex = first.get_mutex_for_validation();
        if (mutex == ft_nullptr)
            return ;
        lock_result.store(mutex->lock());
        lock_acquired.store(true);
        if (lock_result.load() != FT_ERR_SUCCESS)
            return ;
        while (!release_lock.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        mutex->unlock();
    });
    wait_for_lock(lock_acquired);
    duration_ms = encrypt_small_message(second, ciphertext, authentication_tag, output_length);
    release_lock.store(true);
    locker_thread.join();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_result.load());
    FT_ASSERT(duration_ms < 40);
    FT_ASSERT_EQ(static_cast<size_t>(6), output_length);
    return (1);
}

FT_TEST(test_encryption_aead_thread_safety_toggle_round_trip,
        "encryption_aead_context can toggle thread safety without breaking operations")
{
    encryption_aead_context context;
    unsigned char key[16];
    unsigned char iv[12];
    unsigned char ciphertext[8];
    unsigned char authentication_tag[16];
    size_t output_length;
    long duration_ms;

    fill_sequence(key, 16);
    fill_sequence(iv, 12);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.initialize_encrypt(key, 16, iv, 12));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.enable_thread_safety());
    FT_ASSERT(context.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.disable_thread_safety());
    FT_ASSERT(!context.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.enable_thread_safety());
    duration_ms = encrypt_small_message(context, ciphertext, authentication_tag, output_length);
    FT_ASSERT(duration_ms < 40);
    FT_ASSERT_EQ(static_cast<size_t>(6), output_length);
    return (1);
}
