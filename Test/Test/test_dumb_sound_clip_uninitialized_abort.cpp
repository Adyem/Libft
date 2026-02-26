#include "../test_internal.hpp"
#include "../../DUMB/dumb_sound_clip.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_sound_clip sound_clip_type;

static sigjmp_buf g_sound_clip_jump_buffer;
static volatile sig_atomic_t g_sound_clip_signal;

static void sound_clip_sigabrt_handler(int signal_number)
{
    g_sound_clip_signal = signal_number;
    siglongjmp(g_sound_clip_jump_buffer, 1);
}

static int sound_clip_expect_sigabrt_uninitialized(void (*operation)(sound_clip_type &))
{
    struct sigaction new_action;
    struct sigaction old_action;
    int result;

    std::memset(&new_action, 0, sizeof(new_action));
    std::memset(&old_action, 0, sizeof(old_action));
    new_action.sa_handler = sound_clip_sigabrt_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    if (sigaction(SIGABRT, &new_action, &old_action) != 0)
        return (0);

    g_sound_clip_signal = 0;
    if (sigsetjmp(g_sound_clip_jump_buffer, 1) == 0)
    {
        alignas(sound_clip_type) unsigned char storage[sizeof(sound_clip_type)];
        sound_clip_type *sound_clip_pointer;

        std::memset(storage, 0, sizeof(storage));
        sound_clip_pointer = reinterpret_cast<sound_clip_type *>(storage);
        operation(*sound_clip_pointer);
        result = 0;
    }
    else
    {
        result = (g_sound_clip_signal == SIGABRT);
    }

    (void)sigaction(SIGABRT, &old_action, ft_nullptr);
    return (result);
}

static void sound_clip_call_destructor(sound_clip_type &sound_clip_instance)
{
    sound_clip_instance.~sound_clip_type();
    return ;
}

static void sound_clip_call_move(sound_clip_type &sound_clip_instance)
{
    (void)sound_clip_instance.move(sound_clip_instance);
    return ;
}

static void sound_clip_call_load_wav(sound_clip_type &sound_clip_instance)
{
    (void)sound_clip_instance.load_wav(ft_nullptr);
    return ;
}

static void sound_clip_call_get_data(sound_clip_type &sound_clip_instance)
{
    (void)sound_clip_instance.get_data();
    return ;
}

static void sound_clip_call_get_size(sound_clip_type &sound_clip_instance)
{
    (void)sound_clip_instance.get_size();
    return ;
}

static void sound_clip_call_get_spec(sound_clip_type &sound_clip_instance)
{
    (void)sound_clip_instance.get_spec();
    return ;
}

static void sound_clip_call_enable_thread_safety(sound_clip_type &sound_clip_instance)
{
    (void)sound_clip_instance.enable_thread_safety();
    return ;
}

static void sound_clip_call_disable_thread_safety(sound_clip_type &sound_clip_instance)
{
    sound_clip_instance.disable_thread_safety();
    return ;
}

static void sound_clip_call_is_thread_safe(sound_clip_type &sound_clip_instance)
{
    (void)sound_clip_instance.is_thread_safe_enabled();
    return ;
}

static void sound_clip_call_runtime_mutex(sound_clip_type &sound_clip_instance)
{
    (void)sound_clip_instance.runtime_mutex();
    return ;
}

FT_TEST(test_dumb_sound_clip_uninitialized_destructor_tolerates_object,
    "dumb sound clip destructor tolerates uninitialized instance")
{
    FT_ASSERT_EQ(0, sound_clip_expect_sigabrt_uninitialized(sound_clip_call_destructor));
    return (1);
}

FT_TEST(test_dumb_sound_clip_uninitialized_destroy_returns_invalid_state,
    "dumb sound clip destroy returns invalid state on uninitialized instance")
{
    sound_clip_type clip;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, clip.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_clip_uninitialized_move_aborts,
    "dumb sound clip move aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_clip_expect_sigabrt_uninitialized(sound_clip_call_move));
    return (1);
}

FT_TEST(test_dumb_sound_clip_uninitialized_load_wav_aborts,
    "dumb sound clip load_wav aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_clip_expect_sigabrt_uninitialized(sound_clip_call_load_wav));
    return (1);
}

FT_TEST(test_dumb_sound_clip_uninitialized_get_data_aborts,
    "dumb sound clip get_data aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_clip_expect_sigabrt_uninitialized(sound_clip_call_get_data));
    return (1);
}

FT_TEST(test_dumb_sound_clip_uninitialized_get_size_aborts,
    "dumb sound clip get_size aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_clip_expect_sigabrt_uninitialized(sound_clip_call_get_size));
    return (1);
}

FT_TEST(test_dumb_sound_clip_uninitialized_get_spec_aborts,
    "dumb sound clip get_spec aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_clip_expect_sigabrt_uninitialized(sound_clip_call_get_spec));
    return (1);
}

FT_TEST(test_dumb_sound_clip_uninitialized_enable_thread_safety_aborts,
    "dumb sound clip enable_thread_safety aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_clip_expect_sigabrt_uninitialized(sound_clip_call_enable_thread_safety));
    return (1);
}

FT_TEST(test_dumb_sound_clip_uninitialized_disable_thread_safety_aborts,
    "dumb sound clip disable_thread_safety aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_clip_expect_sigabrt_uninitialized(sound_clip_call_disable_thread_safety));
    return (1);
}

FT_TEST(test_dumb_sound_clip_uninitialized_is_thread_safe_aborts,
    "dumb sound clip is_thread_safe_enabled aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_clip_expect_sigabrt_uninitialized(sound_clip_call_is_thread_safe));
    return (1);
}

FT_TEST(test_dumb_sound_clip_uninitialized_runtime_mutex_aborts,
    "dumb sound clip runtime_mutex aborts on uninitialized instance")
{
    FT_ASSERT_EQ(1, sound_clip_expect_sigabrt_uninitialized(sound_clip_call_runtime_mutex));
    return (1);
}
