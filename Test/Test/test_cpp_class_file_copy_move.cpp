#include "../test_internal.hpp"
#include "../../CPP_class/class_file.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

static void file_copy_move_make_template(char *path_buffer)
{
    const char *template_value;
    ft_size_t index;

    template_value = "/tmp/libft_file_copy_move_XXXXXX";
    index = 0;
    while (template_value[index] != '\0')
    {
        path_buffer[index] = template_value[index];
        index++;
    }
    path_buffer[index] = '\0';
    return ;
}

FT_TEST(test_cpp_class_file_copy_constructor_preserves_open_handle)
{
    char path_buffer[64];
    char read_buffer[3];
    int32_t template_descriptor;
    ft_file source_file;

    file_copy_move_make_template(path_buffer);
    template_descriptor = mkstemp(path_buffer);
    FT_ASSERT(template_descriptor >= 0);
    close(template_descriptor);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_file.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_file.open(path_buffer, O_RDWR | O_TRUNC));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_file.enable_thread_safety());
    FT_ASSERT_EQ(2, static_cast<int>(source_file.write_buffer("xy", 2)));

    ft_file copied_file(source_file);

    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, copied_file._initialised_state);
    FT_ASSERT_EQ(FT_TRUE, copied_file.is_thread_safe());
    FT_ASSERT(copied_file.get_file_descriptor() >= 0);
    FT_ASSERT(source_file.get_file_descriptor() != copied_file.get_file_descriptor());
    FT_ASSERT_EQ(0, copied_file.seek(0, SEEK_SET));
    read_buffer[0] = '\0';
    read_buffer[1] = '\0';
    read_buffer[2] = '\0';
    FT_ASSERT_EQ(2, static_cast<int>(copied_file.read(read_buffer, 2)));
    FT_ASSERT_EQ(0, ft_strcmp(read_buffer, "xy"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied_file.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_file.destroy());
    unlink(path_buffer);
    return (1);
}

FT_TEST(test_cpp_class_file_move_constructor_preserves_open_handle)
{
    char path_buffer[64];
    char read_buffer[4];
    int32_t template_descriptor;
    ft_file source_file;

    file_copy_move_make_template(path_buffer);
    template_descriptor = mkstemp(path_buffer);
    FT_ASSERT(template_descriptor >= 0);
    close(template_descriptor);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_file.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_file.open(path_buffer, O_RDWR | O_TRUNC));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_file.enable_thread_safety());
    FT_ASSERT_EQ(3, static_cast<int>(source_file.write_buffer("abc", 3)));

    ft_file moved_file(static_cast<ft_file &&>(source_file));

    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source_file._initialised_state);
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, moved_file._initialised_state);
    FT_ASSERT_EQ(FT_TRUE, moved_file.is_thread_safe());
    FT_ASSERT(moved_file.get_file_descriptor() >= 0);
    FT_ASSERT_EQ(0, moved_file.seek(0, SEEK_SET));
    read_buffer[0] = '\0';
    read_buffer[1] = '\0';
    read_buffer[2] = '\0';
    read_buffer[3] = '\0';
    FT_ASSERT_EQ(3, static_cast<int>(moved_file.read(read_buffer, 3)));
    FT_ASSERT_EQ(0, ft_strcmp(read_buffer, "abc"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_file.destroy());
    unlink(path_buffer);
    return (1);
}
