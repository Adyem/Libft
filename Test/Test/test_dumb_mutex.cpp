#include "../test_internal.hpp"
#include "../../DUMB/render_window.hpp"
#include "../../DUMB/sound_device.hpp"
#include "../../DUMB/sound_clip.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

class test_sound_device_mutex_impl : public ft_sound_device
{
    public:
        test_sound_device_mutex_impl(void);
        test_sound_device_mutex_impl(const test_sound_device_mutex_impl &other);
        test_sound_device_mutex_impl(test_sound_device_mutex_impl &&other);
        test_sound_device_mutex_impl &operator=(
            const test_sound_device_mutex_impl &other) = delete;
        test_sound_device_mutex_impl &operator=(
            test_sound_device_mutex_impl &&other) = delete;
        ~test_sound_device_mutex_impl(void);

        int32_t open(const ft_sound_spec *spec);
        void close(void);
        void pause(int32_t pause_on);
        void play(const ft_sound_clip *clip);
        void stop(void);
};

test_sound_device_mutex_impl::test_sound_device_mutex_impl(void)
{
    return ;
}

test_sound_device_mutex_impl::test_sound_device_mutex_impl(
    const test_sound_device_mutex_impl &other)
    : ft_sound_device(other)
{
    return ;
}

test_sound_device_mutex_impl::test_sound_device_mutex_impl(
    test_sound_device_mutex_impl &&other)
    : ft_sound_device(static_cast<ft_sound_device &&>(other))
{
    return ;
}

test_sound_device_mutex_impl::~test_sound_device_mutex_impl(void)
{
    return ;
}

int32_t test_sound_device_mutex_impl::open(const ft_sound_spec *spec)
{
    (void)spec;
    return (FT_ERR_SUCCESS);
}

void test_sound_device_mutex_impl::close(void)
{
    return ;
}

void test_sound_device_mutex_impl::pause(int32_t pause_on)
{
    (void)pause_on;
    return ;
}

void test_sound_device_mutex_impl::play(const ft_sound_clip *clip)
{
    (void)clip;
    return ;
}

void test_sound_device_mutex_impl::stop(void)
{
    return ;
}

static int32_t dumb_expect_recursive_mutex_usable(pt_recursive_mutex *mutex_pointer)
{
    int32_t lock_error;
    int32_t unlock_error;

    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_FALSE, mutex_pointer->lockState());
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    FT_ASSERT_EQ(FT_TRUE, mutex_pointer->lockState());
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    FT_ASSERT_EQ(FT_TRUE, mutex_pointer->lockState());
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    FT_ASSERT_EQ(FT_FALSE, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_dumb_render_mutex_is_recursive_and_balanced)
{
    ft_render_window render_window_instance;
    ft_render_window_desc description;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.initialize());
    description.width = 0;
    description.height = 0;
    description.title = ft_nullptr;
    description.flags = FT_RENDER_WINDOW_FLAG_NONE;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        render_window_instance.initialize(description));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.enable_thread_safety());
    mutex_pointer = render_window_instance._mutex;
    FT_ASSERT_EQ(1, dumb_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(FT_ERR_NOT_INITIALISED, render_window_instance.poll_events());
    FT_ASSERT_EQ(FT_ERR_NOT_INITIALISED, render_window_instance.present());
    FT_ASSERT_EQ(FT_ERR_NOT_INITIALISED, render_window_instance.clear(0U));
    FT_ASSERT_EQ(FT_ERR_NOT_INITIALISED, render_window_instance.put_pixel(0, 0, 0U));
    FT_ASSERT_EQ(FT_ERR_NOT_INITIALISED, render_window_instance.set_fullscreen(FT_FALSE));
    FT_ASSERT_EQ(FT_FALSE, mutex_pointer->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_clip_mutex_is_recursive_and_balanced)
{
    ft_sound_clip sound_clip_instance;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_IO, sound_clip_instance.load_wav("not_found.wav"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.enable_thread_safety());
    mutex_pointer = sound_clip_instance._mutex;
    FT_ASSERT_EQ(1, dumb_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(FT_FALSE, mutex_pointer->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_device_mutex_is_recursive_and_balanced)
{
    test_sound_device_mutex_impl sound_device_instance;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, sound_device_instance.is_thread_safe());
    mutex_pointer = sound_device_instance._mutex;
    FT_ASSERT_EQ(1, dumb_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(FT_FALSE, mutex_pointer->lockState());
    sound_device_instance.disable_thread_safety();
    FT_ASSERT_EQ(FT_FALSE, sound_device_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.destroy());
    return (1);
}
