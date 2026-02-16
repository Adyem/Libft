#include "../test_internal.hpp"
#include "../../CPP_class/class_data_buffer.hpp"
#include "../../CPP_class/class_stringbuf.hpp"
#include "../../CPP_class/class_ofstream.hpp"
#include "../../CPP_class/class_fd_istream.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static int expect_sigabrt(void (*operation)(void))
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
    if (!WIFSIGNALED(child_status))
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
}

static DataBuffer *make_uninitialized_data_buffer(void)
{
    alignas(DataBuffer) static unsigned char storage[sizeof(DataBuffer)];

    std::memset(storage, 0, sizeof(storage));
    return (reinterpret_cast<DataBuffer *>(storage));
}

static ft_stringbuf *make_uninitialized_stringbuf(void)
{
    alignas(ft_stringbuf) static unsigned char storage[sizeof(ft_stringbuf)];

    std::memset(storage, 0, sizeof(storage));
    return (reinterpret_cast<ft_stringbuf *>(storage));
}

static ft_ofstream *make_uninitialized_ofstream(void)
{
    alignas(ft_ofstream) static unsigned char storage[sizeof(ft_ofstream)];

    std::memset(storage, 0, sizeof(storage));
    return (reinterpret_cast<ft_ofstream *>(storage));
}

static ft_fd_istream *make_uninitialized_fd_istream(void)
{
    alignas(ft_fd_istream) static unsigned char storage[sizeof(ft_fd_istream)];

    std::memset(storage, 0, sizeof(storage));
    return (reinterpret_cast<ft_fd_istream *>(storage));
}

static void data_buffer_clear_uninitialized_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialized_data_buffer();
    buffer_pointer->clear();
    _exit(0);
}

static void data_buffer_size_uninitialized_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialized_data_buffer();
    (void)buffer_pointer->size();
    _exit(0);
}

static void data_buffer_seek_uninitialized_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialized_data_buffer();
    (void)buffer_pointer->seek(0);
    _exit(0);
}

static void data_buffer_good_uninitialized_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialized_data_buffer();
    (void)buffer_pointer->good();
    _exit(0);
}

static void data_buffer_data_uninitialized_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialized_data_buffer();
    (void)buffer_pointer->data();
    _exit(0);
}

static void data_buffer_tell_uninitialized_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialized_data_buffer();
    (void)buffer_pointer->tell();
    _exit(0);
}

static void data_buffer_bad_uninitialized_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialized_data_buffer();
    (void)buffer_pointer->bad();
    _exit(0);
}

static void data_buffer_bool_uninitialized_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialized_data_buffer();
    (void)static_cast<bool>(*buffer_pointer);
    _exit(0);
}

static void data_buffer_disable_thread_safety_uninitialized_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialized_data_buffer();
    (void)buffer_pointer->disable_thread_safety();
    _exit(0);
}

static void data_buffer_enable_thread_safety_uninitialized_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialized_data_buffer();
    (void)buffer_pointer->enable_thread_safety();
    _exit(0);
}

static void data_buffer_is_thread_safe_uninitialized_aborts(void)
{
    DataBuffer *buffer_pointer = make_uninitialized_data_buffer();
    (void)buffer_pointer->is_thread_safe();
    _exit(0);
}

static void stringbuf_read_uninitialized_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialized_stringbuf();
    char output_buffer[2];

    output_buffer[0] = '\0';
    output_buffer[1] = '\0';
    (void)buffer_pointer->read(output_buffer, 1);
    _exit(0);
}

static void stringbuf_is_valid_uninitialized_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialized_stringbuf();
    (void)buffer_pointer->is_valid();
    _exit(0);
}

static void stringbuf_str_uninitialized_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialized_stringbuf();
    ft_string value;

    (void)value.initialize();
    (void)buffer_pointer->str(value);
    _exit(0);
}

static void stringbuf_enable_thread_safety_uninitialized_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialized_stringbuf();
    (void)buffer_pointer->enable_thread_safety();
    _exit(0);
}

static void stringbuf_disable_thread_safety_uninitialized_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialized_stringbuf();
    (void)buffer_pointer->disable_thread_safety();
    _exit(0);
}

static void stringbuf_is_thread_safe_uninitialized_aborts(void)
{
    ft_stringbuf *buffer_pointer = make_uninitialized_stringbuf();
    (void)buffer_pointer->is_thread_safe();
    _exit(0);
}

static void ofstream_open_uninitialized_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialized_ofstream();
    (void)stream_pointer->open("tmp_ofstream_uninitialized.txt");
    _exit(0);
}

static void ofstream_write_uninitialized_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialized_ofstream();
    (void)stream_pointer->write("payload");
    _exit(0);
}

static void ofstream_close_uninitialized_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialized_ofstream();
    (void)stream_pointer->close();
    _exit(0);
}

static void ofstream_is_thread_safe_uninitialized_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialized_ofstream();
    (void)stream_pointer->is_thread_safe();
    _exit(0);
}

static void ofstream_enable_thread_safety_uninitialized_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialized_ofstream();
    (void)stream_pointer->enable_thread_safety();
    _exit(0);
}

static void ofstream_disable_thread_safety_uninitialized_aborts(void)
{
    ft_ofstream *stream_pointer = make_uninitialized_ofstream();
    (void)stream_pointer->disable_thread_safety();
    _exit(0);
}

static void fd_istream_set_fd_uninitialized_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialized_fd_istream();
    stream_pointer->set_fd(0);
    _exit(0);
}

static void fd_istream_get_fd_uninitialized_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialized_fd_istream();
    (void)stream_pointer->get_fd();
    _exit(0);
}

static void fd_istream_read_uninitialized_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialized_fd_istream();
    char output_buffer[2];

    output_buffer[0] = '\0';
    output_buffer[1] = '\0';
    (void)stream_pointer->read(output_buffer, 1);
    _exit(0);
}

static void fd_istream_gcount_uninitialized_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialized_fd_istream();
    (void)stream_pointer->gcount();
    _exit(0);
}

static void fd_istream_is_valid_uninitialized_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialized_fd_istream();
    (void)stream_pointer->is_valid();
    _exit(0);
}

static void fd_istream_enable_thread_safety_uninitialized_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialized_fd_istream();
    (void)stream_pointer->enable_thread_safety();
    _exit(0);
}

static void fd_istream_disable_thread_safety_uninitialized_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialized_fd_istream();
    (void)stream_pointer->disable_thread_safety();
    _exit(0);
}

static void fd_istream_is_thread_safe_uninitialized_aborts(void)
{
    ft_fd_istream *stream_pointer = make_uninitialized_fd_istream();
    (void)stream_pointer->is_thread_safe();
    _exit(0);
}

FT_TEST(test_data_buffer_clear_uninitialized_aborts,
    "DataBuffer clear aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_clear_uninitialized_aborts));
    return (1);
}

FT_TEST(test_data_buffer_size_uninitialized_aborts,
    "DataBuffer size aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_size_uninitialized_aborts));
    return (1);
}

FT_TEST(test_data_buffer_seek_uninitialized_aborts,
    "DataBuffer seek aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_seek_uninitialized_aborts));
    return (1);
}

FT_TEST(test_data_buffer_good_uninitialized_aborts,
    "DataBuffer good aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_good_uninitialized_aborts));
    return (1);
}

FT_TEST(test_data_buffer_data_uninitialized_aborts,
    "DataBuffer data aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_data_uninitialized_aborts));
    return (1);
}

FT_TEST(test_data_buffer_tell_uninitialized_aborts,
    "DataBuffer tell aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_tell_uninitialized_aborts));
    return (1);
}

FT_TEST(test_data_buffer_bad_uninitialized_aborts,
    "DataBuffer bad aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_bad_uninitialized_aborts));
    return (1);
}

FT_TEST(test_data_buffer_bool_uninitialized_aborts,
    "DataBuffer bool conversion aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_bool_uninitialized_aborts));
    return (1);
}

FT_TEST(test_data_buffer_disable_thread_safety_uninitialized_aborts,
    "DataBuffer disable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_disable_thread_safety_uninitialized_aborts));
    return (1);
}

FT_TEST(test_data_buffer_enable_thread_safety_uninitialized_aborts,
    "DataBuffer enable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_enable_thread_safety_uninitialized_aborts));
    return (1);
}

FT_TEST(test_data_buffer_is_thread_safe_uninitialized_aborts,
    "DataBuffer is_thread_safe aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(data_buffer_is_thread_safe_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_read_uninitialized_aborts,
    "ft_stringbuf read aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_read_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_is_valid_uninitialized_aborts,
    "ft_stringbuf is_valid aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_is_valid_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_str_uninitialized_aborts,
    "ft_stringbuf str aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_str_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_enable_thread_safety_uninitialized_aborts,
    "ft_stringbuf enable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_enable_thread_safety_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_disable_thread_safety_uninitialized_aborts,
    "ft_stringbuf disable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_disable_thread_safety_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_stringbuf_is_thread_safe_uninitialized_aborts,
    "ft_stringbuf is_thread_safe aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(stringbuf_is_thread_safe_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_open_uninitialized_aborts,
    "ft_ofstream open aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_open_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_write_uninitialized_aborts,
    "ft_ofstream write aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_write_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_close_uninitialized_aborts,
    "ft_ofstream close aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_close_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_is_thread_safe_uninitialized_aborts,
    "ft_ofstream is_thread_safe aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_is_thread_safe_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_enable_thread_safety_uninitialized_aborts,
    "ft_ofstream enable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_enable_thread_safety_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_ofstream_disable_thread_safety_uninitialized_aborts,
    "ft_ofstream disable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(ofstream_disable_thread_safety_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_set_fd_uninitialized_aborts,
    "ft_fd_istream set_fd aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_set_fd_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_get_fd_uninitialized_aborts,
    "ft_fd_istream get_fd aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_get_fd_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_read_uninitialized_aborts,
    "ft_fd_istream read aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_read_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_gcount_uninitialized_aborts,
    "ft_fd_istream gcount aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_gcount_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_is_valid_uninitialized_aborts,
    "ft_fd_istream is_valid aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_is_valid_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_enable_thread_safety_uninitialized_aborts,
    "ft_fd_istream enable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_enable_thread_safety_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_disable_thread_safety_uninitialized_aborts,
    "ft_fd_istream disable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_disable_thread_safety_uninitialized_aborts));
    return (1);
}

FT_TEST(test_ft_fd_istream_is_thread_safe_uninitialized_aborts,
    "ft_fd_istream is_thread_safe aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, expect_sigabrt(fd_istream_is_thread_safe_uninitialized_aborts));
    return (1);
}
