#include "../../CPP_class/class_stringbuf.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_ft_stringbuf_read_basic, "ft_stringbuf::read copies data sequentially")
{
    ft_string source("hello");
    ft_stringbuf buffer(source);
    char storage[8];
    std::size_t bytes_read;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    bytes_read = buffer.read(storage, 5);
    storage[bytes_read] = '\0';
    FT_ASSERT_EQ(static_cast<std::size_t>(5), bytes_read);
    FT_ASSERT_EQ(0, ft_strcmp(storage, "hello"));
    FT_ASSERT_EQ(ER_SUCCESS, buffer.get_error());
    FT_ASSERT_EQ(false, buffer.is_bad());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_stringbuf_read_null_buffer_sets_error, "ft_stringbuf::read reports errors for null buffers")
{
    ft_string source("data");
    ft_stringbuf buffer(source);
    std::size_t bytes_read;

    ft_errno = ER_SUCCESS;
    bytes_read = buffer.read(ft_nullptr, 3);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), bytes_read);
    FT_ASSERT_EQ(true, buffer.is_bad());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, buffer.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_stringbuf_str_returns_remaining, "ft_stringbuf::str exposes unread portion and clears errors")
{
    ft_string source("abcdef");
    ft_stringbuf buffer(source);
    char storage[4];
    std::size_t bytes_read;
    ft_string remaining;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    bytes_read = buffer.read(storage, 3);
    storage[bytes_read] = '\0';
    FT_ASSERT_EQ(static_cast<std::size_t>(3), bytes_read);
    FT_ASSERT_EQ(0, ft_strcmp(storage, "abc"));

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    remaining = buffer.str();
    FT_ASSERT_EQ(0, ft_strcmp(remaining.c_str(), "def"));
    FT_ASSERT_EQ(ER_SUCCESS, buffer.get_error());
    FT_ASSERT_EQ(false, buffer.is_bad());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_stringbuf_read_past_end_returns_zero, "ft_stringbuf::read returns zero after reaching the end")
{
    ft_string source("xy");
    ft_stringbuf buffer(source);
    char storage[4];
    std::size_t bytes_read;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    bytes_read = buffer.read(storage, 2);
    storage[bytes_read] = '\0';
    FT_ASSERT_EQ(static_cast<std::size_t>(2), bytes_read);
    FT_ASSERT_EQ(0, ft_strcmp(storage, "xy"));
    FT_ASSERT_EQ(ER_SUCCESS, buffer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    bytes_read = buffer.read(storage, 2);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), bytes_read);
    FT_ASSERT_EQ(ER_SUCCESS, buffer.get_error());
    FT_ASSERT_EQ(false, buffer.is_bad());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
