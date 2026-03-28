#include "../test_internal.hpp"
#include "../../CPP_class/class_stringbuf.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../Basic/basic.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cpp_class_stringbuf_move_constructor_transfers_state)
{
    ft_string source_string;
    ft_string remaining_string;
    ft_stringbuf source_buffer;
    char prefix_buffer[3];
    ssize_t bytes_read;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_string.initialize("abcdef"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, remaining_string.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize(source_string));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.enable_thread_safety());
    bytes_read = source_buffer.read(prefix_buffer, 2);
    FT_ASSERT_EQ(2, static_cast<int>(bytes_read));
    prefix_buffer[2] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(prefix_buffer, "ab"));

    ft_stringbuf moved_buffer(static_cast<ft_stringbuf &&>(source_buffer));

    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source_buffer._initialised_state);
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, moved_buffer._initialised_state);
    FT_ASSERT_EQ(2U, moved_buffer._position);
    FT_ASSERT_EQ(FT_TRUE, moved_buffer.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_buffer.get_string(remaining_string));
    FT_ASSERT_EQ(0, ft_strcmp(remaining_string.c_str(), "cdef"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, remaining_string.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_string.destroy());
    return (1);
}
