#include "../test_internal.hpp"
#include "../../DUMB/dumb_sound.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>

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

static sigjmp_buf g_sound_device_jump_buffer;
static volatile sig_atomic_t g_sound_device_signal;

static void sound_device_sigabrt_handler(int signal_number)
{
    g_sound_device_signal = signal_number;
    siglongjmp(g_sound_device_jump_buffer, 1);
}

static int sound_device_expect_sigabrt_uninitialized(void (*operation)(test_sound_device_impl &))
{
    struct sigaction new_action;
    struct sigaction old_action;
    int result;

    std::memset(&new_action, 0, sizeof(new_action));
    std::memset(&old_action, 0, sizeof(old_action));
    new_action.sa_handler = sound_device_sigabrt_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    if (sigaction(SIGABRT, &new_action, &old_action) != 0)
        return (0);

    g_sound_device_signal = 0;
    if (sigsetjmp(g_sound_device_jump_buffer, 1) == 0)
    {
        alignas(test_sound_device_impl) unsigned char storage[sizeof(test_sound_device_impl)];
        test_sound_device_impl *sound_device_pointer;

        std::memset(storage, 0, sizeof(storage));
        sound_device_pointer = reinterpret_cast<test_sound_device_impl *>(storage);
        operation(*sound_device_pointer);
        result = 0;
    }
    else
    {
        result = (g_sound_device_signal == SIGABRT);
    }

    (void)sigaction(SIGABRT, &old_action, ft_nullptr);
    return (result);
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

FT_TEST(test_dumb_sound_device_uninitialized_destroy_returns_invalid_state,
    "dumb sound device destroy returns invalid state on uninitialized instance")
{
    test_sound_device_impl device;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, device.destroy());
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
