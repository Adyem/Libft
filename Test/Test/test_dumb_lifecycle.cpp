#include "../test_internal.hpp"
#include "../../DUMB/dumb_render.hpp"
#include "../../DUMB/dumb_sound.hpp"
#include "../../DUMB/dumb_sound_clip.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

class test_sound_device_lifecycle_impl : public ft_sound_device
{
    public:
        test_sound_device_lifecycle_impl(void)
        {
            return ;
        }

        ~test_sound_device_lifecycle_impl(void)
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

static int dumb_expect_sigabrt(void (*operation)())
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        operation();
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

static void sound_device_initialize_twice_aborts_operation()
{
    test_sound_device_lifecycle_impl sound_device_instance;

    (void)sound_device_instance.initialize();
    (void)sound_device_instance.initialize();
    return ;
}

static void sound_device_destroy_twice_aborts_operation()
{
    test_sound_device_lifecycle_impl sound_device_instance;

    (void)sound_device_instance.initialize();
    (void)sound_device_instance.destroy();
    (void)sound_device_instance.destroy();
    return ;
}

static void sound_clip_initialize_twice_aborts_operation()
{
    ft_sound_clip sound_clip_instance;

    (void)sound_clip_instance.initialize();
    (void)sound_clip_instance.initialize();
    return ;
}

static void sound_clip_destroy_twice_aborts_operation()
{
    ft_sound_clip sound_clip_instance;

    (void)sound_clip_instance.initialize();
    (void)sound_clip_instance.destroy();
    (void)sound_clip_instance.destroy();
    return ;
}

static void render_window_initialize_twice_aborts_operation()
{
    ft_render_window render_window_instance;

    (void)render_window_instance.initialize();
    (void)render_window_instance.initialize();
    return ;
}

static void render_window_destroy_twice_aborts_operation()
{
    ft_render_window render_window_instance;

    (void)render_window_instance.initialize();
    (void)render_window_instance.destroy();
    (void)render_window_instance.destroy();
    return ;
}

static void sound_device_initialize_copy_destroyed_source_aborts_operation()
{
    test_sound_device_lifecycle_impl source_instance;
    test_sound_device_lifecycle_impl destination_copy;

    (void)source_instance.initialize();
    (void)source_instance.destroy();
    (void)destination_copy.initialize(source_instance);
    return ;
}

static void sound_device_initialize_move_destroyed_source_aborts_operation()
{
    test_sound_device_lifecycle_impl source_instance;
    test_sound_device_lifecycle_impl destination_move;

    (void)source_instance.initialize();
    (void)source_instance.destroy();
    (void)destination_move.initialize(static_cast<test_sound_device_lifecycle_impl &&>(source_instance));
    return ;
}

static void sound_clip_initialize_copy_destroyed_source_aborts_operation()
{
    ft_sound_clip source_instance;
    ft_sound_clip destination_copy;

    (void)source_instance.initialize();
    (void)source_instance.destroy();
    (void)destination_copy.initialize(source_instance);
    return ;
}

static void sound_clip_initialize_move_destroyed_source_aborts_operation()
{
    ft_sound_clip source_instance;
    ft_sound_clip destination_move;

    (void)source_instance.initialize();
    (void)source_instance.destroy();
    (void)destination_move.initialize(static_cast<ft_sound_clip &&>(source_instance));
    return ;
}

static void render_window_initialize_copy_destroyed_source_aborts_operation()
{
    ft_render_window source_instance;
    ft_render_window destination_copy;

    (void)source_instance.initialize();
    (void)source_instance.destroy();
    (void)destination_copy.initialize(source_instance);
    return ;
}

static void render_window_initialize_move_destroyed_source_aborts_operation()
{
    ft_render_window source_instance;
    ft_render_window destination_move;

    (void)source_instance.initialize();
    (void)source_instance.destroy();
    (void)destination_move.initialize(static_cast<ft_render_window &&>(source_instance));
    return ;
}

FT_TEST(test_dumb_sound_device_initialize_destroy_reinitialize_success,
    "dumb sound device supports initialize/destroy/reinitialize lifecycle")
{
    test_sound_device_lifecycle_impl sound_device_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_device_initialize_copy_from_destroyed_source_aborts,
    "dumb sound device initialize(copy) from destroyed source aborts")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(
        sound_device_initialize_copy_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_dumb_sound_device_initialize_move_from_destroyed_source_aborts,
    "dumb sound device initialize(move) from destroyed source aborts")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(
        sound_device_initialize_move_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_dumb_sound_device_move_self_initialized_is_noop,
    "dumb sound device move(self) on initialized object returns success")
{
    test_sound_device_lifecycle_impl sound_device_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.move(sound_device_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_device_initialize_copy_self_initialized_is_noop,
    "dumb sound device initialize(copy self) on initialized object is a no-op")
{
    test_sound_device_lifecycle_impl sound_device_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.initialize(sound_device_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_device_initialize_move_self_initialized_is_noop,
    "dumb sound device initialize(move self) on initialized object is a no-op")
{
    test_sound_device_lifecycle_impl sound_device_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        sound_device_instance.initialize(static_cast<test_sound_device_lifecycle_impl &&>(
            sound_device_instance)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_device_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_device_move_uninitialized_destination_succeeds,
    "dumb sound device move allows uninitialized destination")
{
    test_sound_device_lifecycle_impl source_instance;
    test_sound_device_lifecycle_impl destination_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.move(source_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_device_move_destroyed_destination_succeeds,
    "dumb sound device move allows destroyed destination")
{
    test_sound_device_lifecycle_impl source_instance;
    test_sound_device_lifecycle_impl destination_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.move(source_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_device_initialize_twice_aborts,
    "dumb sound device initialize aborts when called while initialized")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(sound_device_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_dumb_sound_device_destroy_twice_aborts,
    "dumb sound device destroy aborts when called after destroy")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(sound_device_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_dumb_sound_clip_initialize_destroy_reinitialize_success,
    "dumb sound clip supports initialize/destroy/reinitialize lifecycle")
{
    ft_sound_clip sound_clip_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_clip_initialize_copy_from_destroyed_source_aborts,
    "dumb sound clip initialize(copy) from destroyed source aborts")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(
        sound_clip_initialize_copy_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_dumb_sound_clip_initialize_move_from_destroyed_source_aborts,
    "dumb sound clip initialize(move) from destroyed source aborts")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(
        sound_clip_initialize_move_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_dumb_sound_clip_move_self_initialized_is_noop,
    "dumb sound clip move(self) on initialized object returns success")
{
    ft_sound_clip sound_clip_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.move(sound_clip_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_clip_initialize_copy_self_initialized_is_noop,
    "dumb sound clip initialize(copy self) on initialized object is a no-op")
{
    ft_sound_clip sound_clip_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.initialize(sound_clip_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_clip_initialize_move_self_initialized_is_noop,
    "dumb sound clip initialize(move self) on initialized object is a no-op")
{
    ft_sound_clip sound_clip_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        sound_clip_instance.initialize(static_cast<ft_sound_clip &&>(sound_clip_instance)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sound_clip_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_clip_move_uninitialized_destination_succeeds,
    "dumb sound clip move allows uninitialized destination")
{
    ft_sound_clip source_instance;
    ft_sound_clip destination_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.move(source_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_clip_move_destroyed_destination_succeeds,
    "dumb sound clip move allows destroyed destination")
{
    ft_sound_clip source_instance;
    ft_sound_clip destination_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.move(source_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_sound_clip_initialize_twice_aborts,
    "dumb sound clip initialize aborts when called while initialized")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(sound_clip_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_dumb_sound_clip_destroy_twice_aborts,
    "dumb sound clip destroy aborts when called after destroy")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(sound_clip_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_dumb_render_window_initialize_destroy_reinitialize_success,
    "dumb render window supports initialize/destroy/reinitialize lifecycle")
{
    ft_render_window render_window_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_render_window_initialize_copy_from_destroyed_source_aborts,
    "dumb render window initialize(copy) from destroyed source aborts")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(
        render_window_initialize_copy_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_dumb_render_window_initialize_move_from_destroyed_source_aborts,
    "dumb render window initialize(move) from destroyed source aborts")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(
        render_window_initialize_move_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_dumb_render_window_move_self_initialized_is_noop,
    "dumb render window move(self) on initialized object returns success")
{
    ft_render_window render_window_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.move(render_window_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_render_window_initialize_copy_self_initialized_is_noop,
    "dumb render window initialize(copy self) on initialized object is a no-op")
{
    ft_render_window render_window_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        render_window_instance.initialize(render_window_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_render_window_initialize_move_self_initialized_is_noop,
    "dumb render window initialize(move self) on initialized object is a no-op")
{
    ft_render_window render_window_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        render_window_instance.initialize(static_cast<ft_render_window &&>(
            render_window_instance)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, render_window_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_render_window_move_uninitialized_destination_succeeds,
    "dumb render window move allows uninitialized destination")
{
    ft_render_window source_instance;
    ft_render_window destination_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.move(source_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_render_window_move_destroyed_destination_succeeds,
    "dumb render window move allows destroyed destination")
{
    ft_render_window source_instance;
    ft_render_window destination_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.move(source_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_instance.destroy());
    return (1);
}

FT_TEST(test_dumb_render_window_initialize_twice_aborts,
    "dumb render window initialize aborts when called while initialized")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(render_window_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_dumb_render_window_destroy_twice_aborts,
    "dumb render window destroy aborts when called after destroy")
{
    FT_ASSERT_EQ(1, dumb_expect_sigabrt(render_window_destroy_twice_aborts_operation));
    return (1);
}
