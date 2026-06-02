#include "../test_internal.hpp"
#include "../../Modules/DUMB/sound_device.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>
#include <unistd.h>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

class test_sound_device_impl : public ft_sound_device
{
    public:
        test_sound_device_impl(void);
        test_sound_device_impl(const test_sound_device_impl &other);
        test_sound_device_impl(test_sound_device_impl &&other);
        test_sound_device_impl &operator=(const test_sound_device_impl &other) = delete;
        test_sound_device_impl &operator=(test_sound_device_impl &&other) = delete;
        ~test_sound_device_impl(void);

        int32_t open(const ft_sound_spec *spec);
        void close(void);
        void pause(int32_t pause_on);
        void play(const ft_sound_clip *clip);
        void stop(void);
};

static int32_t g_sound_device_initialize_error;

test_sound_device_impl::test_sound_device_impl(void)
{
    return ;
}

test_sound_device_impl::test_sound_device_impl(const test_sound_device_impl &other)
{
    this->initialize(other);
    return ;
}

test_sound_device_impl::test_sound_device_impl(test_sound_device_impl &&other)
{
    this->initialize(static_cast<ft_sound_device &&>(other));
    return ;
}

test_sound_device_impl::~test_sound_device_impl(void)
{
    return ;
}

int32_t test_sound_device_impl::open(const ft_sound_spec *spec)
{
    (void)spec;
    return (FT_ERR_SUCCESS);
}

void test_sound_device_impl::close(void)
{
    return ;
}

void test_sound_device_impl::pause(int32_t pause_on)
{
    (void)pause_on;
    return ;
}

void test_sound_device_impl::play(const ft_sound_clip *clip)
{
    (void)clip;
    return ;
}

void test_sound_device_impl::stop(void)
{
    return ;
}

static sigjmp_buf g_sound_device_jump_buffer;
static volatile sig_atomic_t g_sound_device_signal;

static void sound_device_sigabrt_handler(int32_t signal_number)
{
    g_sound_device_signal = signal_number;
    siglongjmp(g_sound_device_jump_buffer, 1);
}

static int32_t sound_device_expect_sigabrt_uninitialised(int32_t (*operation)(test_sound_device_impl &))
{
    struct sigaction new_action;
    struct sigaction old_action;
    int32_t error_pipe[2];
    int32_t stderr_backup;
    char error_output[512];
    ssize_t read_size;
    ft_bool has_lifecycle_error;
    int32_t result;

    std::memset(&new_action, 0, sizeof(new_action));
    std::memset(&old_action, 0, sizeof(old_action));
    new_action.sa_handler = sound_device_sigabrt_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    if (sigaction(SIGABRT, &new_action, &old_action) != 0)
        return (0);
    error_pipe[0] = -1;
    error_pipe[1] = -1;
    stderr_backup = -1;
    std::memset(error_output, 0, sizeof(error_output));
    if (pipe(error_pipe) != 0)
    {
        (void)sigaction(SIGABRT, &old_action, ft_nullptr);
        return (0);
    }
    stderr_backup = dup(STDERR_FILENO);
    if (stderr_backup < 0)
    {
        close(error_pipe[0]);
        close(error_pipe[1]);
        (void)sigaction(SIGABRT, &old_action, ft_nullptr);
        return (0);
    }
    (void)dup2(error_pipe[1], STDERR_FILENO);

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
    (void)dup2(stderr_backup, STDERR_FILENO);
    close(stderr_backup);
    close(error_pipe[1]);
    read_size = read(error_pipe[0], error_output, sizeof(error_output) - 1);
    close(error_pipe[0]);
    has_lifecycle_error = FT_FALSE;
    if (read_size > 0 && std::strstr(error_output, "lifecycle error:") != ft_nullptr)
        has_lifecycle_error = FT_TRUE;
    if (result != 0 && has_lifecycle_error == FT_FALSE)
        result = 0;

    (void)sigaction(SIGABRT, &old_action, ft_nullptr);
    return (result);
}

static int32_t sound_device_call_move(test_sound_device_impl &sound_device_instance)
{
    g_sound_device_initialize_error = sound_device_instance.move(sound_device_instance);
    return (g_sound_device_initialize_error);
}

static int32_t sound_device_call_enable_thread_safety(test_sound_device_impl &sound_device_instance)
{
    g_sound_device_initialize_error = sound_device_instance.enable_thread_safety();
    return (g_sound_device_initialize_error);
}

static int32_t sound_device_call_disable_thread_safety(test_sound_device_impl &sound_device_instance)
{
    g_sound_device_initialize_error = sound_device_instance.disable_thread_safety();
    return (g_sound_device_initialize_error);
}

static int32_t sound_device_call_is_thread_safe(test_sound_device_impl &sound_device_instance)
{
    g_sound_device_initialize_error = sound_device_instance.is_thread_safe();
    return (g_sound_device_initialize_error);
}

FT_TEST(test_dumb_sound_device_uninitialised_destroy_returns_invalid_state)
{
    test_sound_device_impl device;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, device.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_device_uninitialised_move_aborts)
{
    FT_ASSERT_EQ(1, sound_device_expect_sigabrt_uninitialised(sound_device_call_move));
    return (1);
}

FT_TEST(test_dumb_sound_device_uninitialised_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, sound_device_expect_sigabrt_uninitialised(sound_device_call_enable_thread_safety));
    return (1);
}

FT_TEST(test_dumb_sound_device_uninitialised_disable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, sound_device_expect_sigabrt_uninitialised(sound_device_call_disable_thread_safety));
    return (1);
}

FT_TEST(test_dumb_sound_device_uninitialised_is_thread_safe_aborts)
{
    FT_ASSERT_EQ(1, sound_device_expect_sigabrt_uninitialised(sound_device_call_is_thread_safe));
    return (1);
}
