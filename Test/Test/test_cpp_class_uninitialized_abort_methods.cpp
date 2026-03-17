#include "../test_internal.hpp"
#include "../../CPP_class/class_data_buffer.hpp"
#include "../../CPP_class/class_stringbuf.hpp"
#include "../../CPP_class/class_ofstream.hpp"
#include "../../CPP_class/class_fd_istream.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static sigjmp_buf g_expect_sigabrt_jump_buffer;
static volatile sig_atomic_t g_expect_sigabrt_signal;

static void expect_sigabrt_signal_handler(int signal_number)
{
    g_expect_sigabrt_signal = signal_number;
    siglongjmp(g_expect_sigabrt_jump_buffer, 1);
}

static int expect_sigabrt(void (*operation)(void))
{
    struct sigaction new_action;
    struct sigaction old_action;

    std::memset(&new_action, 0, sizeof(new_action));
    std::memset(&old_action, 0, sizeof(old_action));
    new_action.sa_handler = expect_sigabrt_signal_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    if (sigaction(SIGABRT, &new_action, &old_action) != 0)
    {
        return (0);
    }
    g_expect_sigabrt_signal = 0;
    if (sigsetjmp(g_expect_sigabrt_jump_buffer, 1) == 0)
    {
        operation();
        (void)sigaction(SIGABRT, &old_action, ft_nullptr);
        return (0);
    }
    (void)sigaction(SIGABRT, &old_action, ft_nullptr);
    return (g_expect_sigabrt_signal == SIGABRT);
}

static DataBuffer *make_uninitialised_data_buffer(void)
{
    alignas(DataBuffer) static unsigned char storage[sizeof(DataBuffer)];

    std::memset(storage, 0, sizeof(storage));
    return (reinterpret_cast<DataBuffer *>(storage));
}

static ft_stringbuf *make_uninitialised_stringbuf(void)
{
    alignas(ft_stringbuf) static unsigned char storage[sizeof(ft_stringbuf)];

    std::memset(storage, 0, sizeof(storage));
    return (reinterpret_cast<ft_stringbuf *>(storage));
}

static ft_ofstream *make_uninitialised_ofstream(void)
{
    alignas(ft_ofstream) static unsigned char storage[sizeof(ft_ofstream)];

    std::memset(storage, 0, sizeof(storage));
    return (reinterpret_cast<ft_ofstream *>(storage));
}

static ft_fd_istream *make_uninitialised_fd_istream(void)
{
    alignas(ft_fd_istream) static unsigned char storage[sizeof(ft_fd_istream)];

    std::memset(storage, 0, sizeof(storage));
    return (reinterpret_cast<ft_fd_istream *>(storage));
}

static void data_buffer_clear_uninitialised_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialised_data_buffer();
    buffer_pointer->clear();
    return ;
}

static void data_buffer_size_uninitialised_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialised_data_buffer();
    (void)buffer_pointer->size();
    return ;
}

static void data_buffer_seek_uninitialised_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialised_data_buffer();
    (void)buffer_pointer->seek(0);
    return ;
}

static void data_buffer_good_uninitialised_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialised_data_buffer();
    (void)buffer_pointer->good();
    return ;
}

static void data_buffer_data_uninitialised_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialised_data_buffer();
    (void)buffer_pointer->data();
    return ;
}

static void data_buffer_tell_uninitialised_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialised_data_buffer();
    (void)buffer_pointer->tell();
    return ;
}

static void data_buffer_bad_uninitialised_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialised_data_buffer();
    (void)buffer_pointer->bad();
    return ;
}

static void data_buffer_bool_uninitialised_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialised_data_buffer();
    (void)static_cast<ft_bool>(*buffer_pointer);
    return ;
}

static void data_buffer_disable_thread_safety_uninitialised_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialised_data_buffer();
    (void)buffer_pointer->disable_thread_safety();
    return ;
}

static void data_buffer_enable_thread_safety_uninitialised_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialised_data_buffer();
    (void)buffer_pointer->enable_thread_safety();
    return ;
}

static void data_buffer_is_thread_safe_uninitialised_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialised_data_buffer();
    (void)buffer_pointer->is_thread_safe();
    return ;
}

static void stringbuf_read_uninitialised_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialised_stringbuf();
    char output_buffer[2];

    output_buffer[0] = '\0';
    output_buffer[1] = '\0';
    (void)buffer_pointer->read(output_buffer, 1);
    return ;
}

static void stringbuf_is_valid_uninitialised_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialised_stringbuf();
    (void)buffer_pointer->is_valid();
    return ;
}

static void stringbuf_str_uninitialised_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialised_stringbuf();
    static ft_string value;
    static bool is_value_initialised = false;

    if (!is_value_initialised)
    {
        if (value.initialize() == FT_ERR_SUCCESS)
        {
            is_value_initialised = true;
        }
    }
    (void)buffer_pointer->get_string(value);
    return ;
}

static void stringbuf_enable_thread_safety_uninitialised_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialised_stringbuf();
    (void)buffer_pointer->enable_thread_safety();
    return ;
}

static void stringbuf_disable_thread_safety_uninitialised_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialised_stringbuf();
    (void)buffer_pointer->disable_thread_safety();
    return ;
}

static void stringbuf_is_thread_safe_uninitialised_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialised_stringbuf();
    (void)buffer_pointer->is_thread_safe();
    return ;
}

static void ofstream_open_uninitialised_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialised_ofstream();
    (void)stream_pointer->open("tmp_ofstream_uninitialised.txt");
    return ;
}

static void ofstream_write_uninitialised_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialised_ofstream();
    (void)stream_pointer->write("payload");
    return ;
}

static void ofstream_close_uninitialised_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialised_ofstream();
    (void)stream_pointer->close();
    return ;
}

static void ofstream_is_thread_safe_uninitialised_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialised_ofstream();
    (void)stream_pointer->is_thread_safe();
    return ;
}

static void ofstream_enable_thread_safety_uninitialised_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialised_ofstream();
    (void)stream_pointer->enable_thread_safety();
    return ;
}

static void ofstream_disable_thread_safety_uninitialised_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialised_ofstream();
    (void)stream_pointer->disable_thread_safety();
    return ;
}

static void fd_istream_set_fd_uninitialised_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialised_fd_istream();
    stream_pointer->set_file_descriptor(0);
    return ;
}

static void fd_istream_get_fd_uninitialised_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialised_fd_istream();
    (void)stream_pointer->get_file_descriptor();
    return ;
}

static void fd_istream_read_uninitialised_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialised_fd_istream();
    char output_buffer[2];

    output_buffer[0] = '\0';
    output_buffer[1] = '\0';
    (void)stream_pointer->read(output_buffer, 1);
    return ;
}

static void fd_istream_gcount_uninitialised_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialised_fd_istream();
    (void)stream_pointer->gcount();
    return ;
}

static void fd_istream_is_valid_uninitialised_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialised_fd_istream();
    (void)stream_pointer->is_valid();
    return ;
}

static void fd_istream_enable_thread_safety_uninitialised_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialised_fd_istream();
    (void)stream_pointer->enable_thread_safety();
    return ;
}

static void fd_istream_disable_thread_safety_uninitialised_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialised_fd_istream();
    (void)stream_pointer->disable_thread_safety();
    return ;
}

static void fd_istream_is_thread_safe_uninitialised_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialised_fd_istream();
    (void)stream_pointer->is_thread_safe();
    return ;
}

FT_TEST(test_data_buffer_clear_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_clear_uninitialised_aborts));
    return (1);
}

FT_TEST(test_data_buffer_size_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_size_uninitialised_aborts));
    return (1);
}

FT_TEST(test_data_buffer_seek_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_seek_uninitialised_aborts));
    return (1);
}

FT_TEST(test_data_buffer_good_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_good_uninitialised_aborts));
    return (1);
}

FT_TEST(test_data_buffer_data_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_data_uninitialised_aborts));
    return (1);
}

FT_TEST(test_data_buffer_tell_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_tell_uninitialised_aborts));
    return (1);
}

FT_TEST(test_data_buffer_bad_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_bad_uninitialised_aborts));
    return (1);
}

FT_TEST(test_data_buffer_bool_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_bool_uninitialised_aborts));
    return (1);
}

FT_TEST(test_data_buffer_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_disable_thread_safety_uninitialised_aborts));
    return (1);
}

FT_TEST(test_data_buffer_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_enable_thread_safety_uninitialised_aborts));
    return (1);
}

FT_TEST(test_data_buffer_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_is_thread_safe_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_read_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_read_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_is_valid_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_is_valid_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_str_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_enable_thread_safety_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_disable_thread_safety_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_is_thread_safe_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_open_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_open_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_write_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_write_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_close_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_close_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_is_thread_safe_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_enable_thread_safety_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_disable_thread_safety_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_set_fd_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_set_fd_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_get_fd_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_get_fd_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_read_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_read_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_gcount_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_gcount_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_is_valid_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_is_valid_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_enable_thread_safety_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_disable_thread_safety_uninitialised_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_is_thread_safe_uninitialised_aborts));
    return (1);
}
