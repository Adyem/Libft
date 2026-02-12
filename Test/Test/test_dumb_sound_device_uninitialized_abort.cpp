#include "../test_internal.hpp"
#include "../../DUMB/dumb_sound.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

class test_sound_device_impl : public ft_sound_device
{
    public:
        test_sound_device_impl(void)
        {
            return ;
        }

        ~test_sound_device_impl(void)
        {
            return ;
        }

        int open(const ft_sound_spec *)
        {
            return (ft_sound_ok);
        }

        void close(void)
        {
            return ;
        }

        void pause(int)
        {
            return ;
        }

        void play(const ft_sound_clip *)
        {
            return ;
        }

        void stop(void)
        {
            return ;
        }
};

static int sound_device_expect_sigabrt_uninitialized(void (*operation)(test_sound_device_impl &))
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        alignas(test_sound_device_impl) unsigned char storage[sizeof(test_sound_device_impl)];
        test_sound_device_impl *sound_device_pointer;

        std::memset(storage, 0, sizeof(storage));
        sound_device_pointer = reinterpret_cast<test_sound_device_impl *>(storage);
        operation(*sound_device_pointer);
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (WIFSIGNALED(child_status) == 0)
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
}

static void sound_device_call_destructor(test_sound_device_impl &sound_device_instance)
{
    sound_device_instance.~test_sound_device_impl();
    return ;
}

static void sound_device_call_destroy(test_sound_device_impl &sound_device_instance)
{
    (void)sound_device_instance.destroy();
    return ;
}

static void sound_device_call_move(test_sound_device_impl &sound_device_instance)
{
    (void)sound_device_instance.move(sound_device_instance);
    return ;
}

static void sound_device_call_enable_thread_safety(test_sound_device_impl &sound_device_instance)
{
    (void)sound_device_instance.enable_thread_safety();
    return ;
}

static void sound_device_call_disable_thread_safety(test_sound_device_impl &sound_device_instance)
{
    sound_device_instance.disable_thread_safety();
    return ;
}

static void sound_device_call_is_thread_safe(test_sound_device_impl &sound_device_instance)
{
    (void)sound_device_instance.is_thread_safe_enabled();
    return ;
}

static void sound_device_call_runtime_mutex(test_sound_device_impl &sound_device_instance)
{
    (void)sound_device_instance.runtime_mutex();
    return ;
}

FT_TEST(test_dumb_sound_device_uninitialized_destructor_aborts,
    "dumb sound device destructor aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_device_expect_sigabrt_uninitialized(sound_device_call_destructor));
    return (1);
}

FT_TEST(test_dumb_sound_device_uninitialized_destroy_aborts,
    "dumb sound device destroy aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_device_expect_sigabrt_uninitialized(sound_device_call_destroy));
    return (1);
}

FT_TEST(test_dumb_sound_device_uninitialized_move_aborts,
    "dumb sound device move aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_device_expect_sigabrt_uninitialized(sound_device_call_move));
    return (1);
}

FT_TEST(test_dumb_sound_device_uninitialized_enable_thread_safety_aborts,
    "dumb sound device enable_thread_safety aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_device_expect_sigabrt_uninitialized(sound_device_call_enable_thread_safety));
    return (1);
}

FT_TEST(test_dumb_sound_device_uninitialized_disable_thread_safety_aborts,
    "dumb sound device disable_thread_safety aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_device_expect_sigabrt_uninitialized(sound_device_call_disable_thread_safety));
    return (1);
}

FT_TEST(test_dumb_sound_device_uninitialized_is_thread_safe_aborts,
    "dumb sound device is_thread_safe_enabled aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_device_expect_sigabrt_uninitialized(sound_device_call_is_thread_safe));
    return (1);
}

FT_TEST(test_dumb_sound_device_uninitialized_runtime_mutex_aborts,
    "dumb sound device runtime_mutex aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_device_expect_sigabrt_uninitialized(sound_device_call_runtime_mutex));
    return (1);
}
