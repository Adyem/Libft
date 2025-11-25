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
    FT_ASSERT_EQ(ER_SUCCESS, context.update(plaintext, 6, ciphertext, output_length));
    FT_ASSERT_EQ(static_cast<size_t>(6), output_length);
    FT_ASSERT_EQ(ER_SUCCESS, context.finalize(authentication_tag, 16));
    end_time = std::chrono::steady_clock::now();
    return (std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
}

FT_TEST(test_encryption_aead_move_constructor_has_fresh_mutex,
        "encryption_aead_context move constructor initializes an independent mutex")
{
    encryption_aead_context source;
    unsigned char key[16];
    unsigned char iv[12];
    std::atomic<bool> lock_acquired;
    std::atomic<bool> release_lock;
    std::atomic<int> lock_result;
    std::thread locker_thread;
    unsigned char ciphertext[8];
    unsigned char authentication_tag[16];
    size_t output_length;
    long duration_ms;

    fill_sequence(key, 16);
    fill_sequence(iv, 12);
    FT_ASSERT_EQ(ER_SUCCESS, source.initialize_encrypt(key, 16, iv, 12));
    encryption_aead_context moved(ft_move(source));
    lock_acquired.store(false);
    release_lock.store(false);
    lock_result.store(ER_SUCCESS);
    locker_thread = std::thread([&source, &lock_acquired, &release_lock, &lock_result]() {
        lock_result.store(source._mutex.lock(THREAD_ID));
        lock_acquired.store(true);
        if (lock_result.load() != ER_SUCCESS)
            return ;
        while (!release_lock.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        source._mutex.unlock(THREAD_ID);
    });
    wait_for_lock(lock_acquired);
    duration_ms = encrypt_small_message(moved, ciphertext, authentication_tag, output_length);
    release_lock.store(true);
    locker_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, lock_result.load());
    FT_ASSERT(duration_ms < 40);
    FT_ASSERT_EQ(static_cast<size_t>(6), output_length);
    FT_ASSERT_EQ(NULL, source._context);
    FT_ASSERT_EQ(false, source._initialized);
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    return (1);
}

FT_TEST(test_encryption_aead_move_assignment_has_fresh_mutex,
        "encryption_aead_context move assignment initializes an independent mutex")
{
    encryption_aead_context source;
    encryption_aead_context target;
    unsigned char key[16];
    unsigned char iv[12];
    std::atomic<bool> lock_acquired;
    std::atomic<bool> release_lock;
    std::atomic<int> lock_result;
    std::thread locker_thread;
    unsigned char ciphertext[8];
    unsigned char authentication_tag[16];
    size_t output_length;
    long duration_ms;

    fill_sequence(key, 16);
    fill_sequence(iv, 12);
    FT_ASSERT_EQ(ER_SUCCESS, source.initialize_encrypt(key, 16, iv, 12));
    target = ft_move(source);
    lock_acquired.store(false);
    release_lock.store(false);
    lock_result.store(ER_SUCCESS);
    locker_thread = std::thread([&source, &lock_acquired, &release_lock, &lock_result]() {
        lock_result.store(source._mutex.lock(THREAD_ID));
        lock_acquired.store(true);
        if (lock_result.load() != ER_SUCCESS)
            return ;
        while (!release_lock.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        source._mutex.unlock(THREAD_ID);
    });
    wait_for_lock(lock_acquired);
    duration_ms = encrypt_small_message(target, ciphertext, authentication_tag, output_length);
    release_lock.store(true);
    locker_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, lock_result.load());
    FT_ASSERT(duration_ms < 40);
    FT_ASSERT_EQ(static_cast<size_t>(6), output_length);
    FT_ASSERT_EQ(NULL, source._context);
    FT_ASSERT_EQ(false, source._initialized);
    FT_ASSERT_EQ(ER_SUCCESS, target.get_error());
    return (1);
}
