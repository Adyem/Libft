#include "../test_internal.hpp"
#include "../../DUMB/dumb_render.hpp"
#include "../../DUMB/dumb_sound.hpp"
#include "../../DUMB/dumb_sound_clip.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

class test_sound_device_mutex_impl : public ft_sound_device
{
    public:
        test_sound_device_mutex_impl(void)
        {
            return ;
        }

        ~test_sound_device_mutex_impl(void)
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

static int dumb_expect_recursive_mutex_usable(pt_recursive_mutex *mutex_pointer)
{
    int lock_error;
    int unlock_error;

    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    FT_ASSERT_EQ(true, mutex_pointer->lockState());
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    FT_ASSERT_EQ(true, mutex_pointer->lockState());
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_dumb_render_mutex_is_recursive_and_balanced,
    "dumb render runtime mutex supports recursive lock and balanced unlock")
{
    ft_render_window render_window_instance;
    ft_render_window_desc description;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.initialize());
    description.width = 0;
    description.height = 0;
    description.title = ft_nullptr;
    description.flags = ft_render_window_flag_none;
    FT_ASSERT_EQ(ft_render_error_invalid_argument,
        render_window_instance.initialize(description));
    mutex_pointer = render_window_instance.runtime_mutex();
    FT_ASSERT_EQ(1, dumb_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(ft_render_error_not_initialized, render_window_instance.poll_events());
    FT_ASSERT_EQ(ft_render_error_not_initialized, render_window_instance.present());
    FT_ASSERT_EQ(ft_render_error_not_initialized, render_window_instance.clear(0U));
    FT_ASSERT_EQ(ft_render_error_not_initialized, render_window_instance.put_pixel(0, 0, 0U));
    FT_ASSERT_EQ(ft_render_error_not_initialized, render_window_instance.set_fullscreen(false));
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_clip_mutex_is_recursive_and_balanced,
    "dumb sound clip runtime mutex supports recursive lock and balanced unlock")
{
    ft_sound_clip sound_clip_instance;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.initialize());
    FT_ASSERT_EQ(ft_sound_error_platform_failure, sound_clip_instance.load_wav("not_found.wav"));
    mutex_pointer = sound_clip_instance.runtime_mutex();
    FT_ASSERT_EQ(1, dumb_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_device_mutex_is_recursive_and_balanced,
    "dumb sound device runtime mutex supports recursive lock and balanced unlock")
{
    test_sound_device_mutex_impl sound_device_instance;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.enable_thread_safety());
    FT_ASSERT_EQ(true, sound_device_instance.is_thread_safe_enabled());
    mutex_pointer = sound_device_instance.runtime_mutex();
    FT_ASSERT_EQ(1, dumb_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    sound_device_instance.disable_thread_safety();
    FT_ASSERT_EQ(false, sound_device_instance.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.destroy());
    return (1);
}
