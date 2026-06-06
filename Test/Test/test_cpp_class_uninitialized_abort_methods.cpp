#include "../test_internal.hpp"
#include "../../Modules/CPP_class/class_data_buffer.hpp"
#include "../../Modules/CPP_class/class_fd_istream.hpp"
#include "../../Modules/CPP_class/class_ofstream.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/CPP_class/class_stringbuf.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"

template <typename TypeName>
static int32_t expect_sigabrt(void (*operation)(TypeName &))
{
    return (test_expect_sigabrt_signal_uninitialised<TypeName>(operation));
}

static void data_buffer_clear_uninitialised_aborts(DataBuffer &buffer_value)
{
    buffer_value.clear();
    return ;
}

static void data_buffer_size_uninitialised_aborts(DataBuffer &buffer_value)
{
    (void)buffer_value.size();
    return ;
}

static void data_buffer_seek_uninitialised_aborts(DataBuffer &buffer_value)
{
    (void)buffer_value.seek(0);
    return ;
}

static void data_buffer_good_uninitialised_aborts(DataBuffer &buffer_value)
{
    (void)buffer_value.good();
    return ;
}

static void data_buffer_data_uninitialised_aborts(DataBuffer &buffer_value)
{
    (void)buffer_value.data();
    return ;
}

static void data_buffer_tell_uninitialised_aborts(DataBuffer &buffer_value)
{
    (void)buffer_value.tell();
    return ;
}

static void data_buffer_bad_uninitialised_aborts(DataBuffer &buffer_value)
{
    (void)buffer_value.bad();
    return ;
}

static void data_buffer_bool_uninitialised_aborts(DataBuffer &buffer_value)
{
    (void)static_cast<ft_bool>(buffer_value);
    return ;
}

static void data_buffer_disable_thread_safety_uninitialised_aborts(DataBuffer &buffer_value)
{
    (void)buffer_value.disable_thread_safety();
    return ;
}

static void data_buffer_enable_thread_safety_uninitialised_aborts(DataBuffer &buffer_value)
{
    (void)buffer_value.enable_thread_safety();
    return ;
}

static void data_buffer_is_thread_safe_uninitialised_aborts(DataBuffer &buffer_value)
{
    (void)buffer_value.is_thread_safe();
    return ;
}

static void stringbuf_read_uninitialised_aborts(ft_stringbuf &buffer_value)
{
    char output_buffer[2];

    output_buffer[0] = '\0';
    output_buffer[1] = '\0';
    (void)buffer_value.read(output_buffer, 1);
    return ;
}

static void stringbuf_is_valid_uninitialised_aborts(ft_stringbuf &buffer_value)
{
    (void)buffer_value.is_valid();
    return ;
}

static void stringbuf_str_uninitialised_aborts(ft_stringbuf &buffer_value)
{
    ft_string value;

    (void)value.initialize();
    (void)buffer_value.get_string(value);
    (void)value.destroy();
    return ;
}

static void stringbuf_enable_thread_safety_uninitialised_aborts(ft_stringbuf &buffer_value)
{
    (void)buffer_value.enable_thread_safety();
    return ;
}

static void stringbuf_disable_thread_safety_uninitialised_aborts(ft_stringbuf &buffer_value)
{
    (void)buffer_value.disable_thread_safety();
    return ;
}

static void stringbuf_is_thread_safe_uninitialised_aborts(ft_stringbuf &buffer_value)
{
    (void)buffer_value.is_thread_safe();
    return ;
}

static void ofstream_open_uninitialised_aborts(ft_ofstream &stream_value)
{
    (void)stream_value.open("tmp_ofstream_uninitialised.txt");
    return ;
}

static void ofstream_write_uninitialised_aborts(ft_ofstream &stream_value)
{
    (void)stream_value.write("payload");
    return ;
}

static void ofstream_close_uninitialised_aborts(ft_ofstream &stream_value)
{
    (void)stream_value.close();
    return ;
}

static void ofstream_is_thread_safe_uninitialised_aborts(ft_ofstream &stream_value)
{
    (void)stream_value.is_thread_safe();
    return ;
}

static void ofstream_enable_thread_safety_uninitialised_aborts(ft_ofstream &stream_value)
{
    (void)stream_value.enable_thread_safety();
    return ;
}

static void ofstream_disable_thread_safety_uninitialised_aborts(ft_ofstream &stream_value)
{
    (void)stream_value.disable_thread_safety();
    return ;
}

static void fd_istream_set_fd_uninitialised_aborts(ft_fd_istream &stream_value)
{
    stream_value.set_file_descriptor(0);
    return ;
}

static void fd_istream_get_fd_uninitialised_aborts(ft_fd_istream &stream_value)
{
    (void)stream_value.get_file_descriptor();
    return ;
}

static void fd_istream_read_uninitialised_aborts(ft_fd_istream &stream_value)
{
    char output_buffer[2];

    output_buffer[0] = '\0';
    output_buffer[1] = '\0';
    (void)stream_value.read(output_buffer, 1);
    return ;
}

static void fd_istream_gcount_uninitialised_aborts(ft_fd_istream &stream_value)
{
    (void)stream_value.gcount();
    return ;
}

static void fd_istream_is_valid_uninitialised_aborts(ft_fd_istream &stream_value)
{
    (void)stream_value.is_valid();
    return ;
}

static void fd_istream_enable_thread_safety_uninitialised_aborts(ft_fd_istream &stream_value)
{
    (void)stream_value.enable_thread_safety();
    return ;
}

static void fd_istream_disable_thread_safety_uninitialised_aborts(ft_fd_istream &stream_value)
{
    (void)stream_value.disable_thread_safety();
    return ;
}

static void fd_istream_is_thread_safe_uninitialised_aborts(ft_fd_istream &stream_value)
{
    (void)stream_value.is_thread_safe();
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
