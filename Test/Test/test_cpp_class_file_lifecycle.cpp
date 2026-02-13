#include "../test_internal.hpp"
#include "../../CPP_class/class_file.hpp"
#include "../../System_utils/test_runner.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static void cleanup_file(const char *path)
{
    if (path == ft_nullptr)
        return ;
    unlink(path);
    return ;
}

FT_TEST(test_ft_file_default_constructor_starts_closed,
    "ft_file default constructor starts with invalid descriptor")
{
    ft_file file;

    FT_ASSERT_EQ(-1, file.get_fd());
    return (1);
}

FT_TEST(test_ft_file_open_close_lifecycle_cycle,
    "ft_file open and close leave the object reusable")
{
    const char *path;
    ft_file file;

    path = "tmp_ft_file_lifecycle_cycle.txt";
    cleanup_file(path);
    FT_ASSERT_EQ(0, file.open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644));
    FT_ASSERT(file.get_fd() >= 0);
    FT_ASSERT_EQ(5, static_cast<int>(file.write("hello")));
    file.close();
    FT_ASSERT_EQ(-1, file.get_fd());
    cleanup_file(path);
    return (1);
}

FT_TEST(test_ft_file_reopen_after_close_reads_previous_data,
    "ft_file can reopen after close and read persisted bytes")
{
    const char *path;
    ft_file file;
    char buffer[6];

    path = "tmp_ft_file_lifecycle_reopen.txt";
    cleanup_file(path);
    FT_ASSERT_EQ(0, file.open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644));
    FT_ASSERT_EQ(5, static_cast<int>(file.write("world")));
    file.close();
    FT_ASSERT_EQ(0, file.open(path, O_RDONLY));
    std::memset(buffer, 0, sizeof(buffer));
    FT_ASSERT_EQ(5, static_cast<int>(file.read(buffer, 5)));
    FT_ASSERT_EQ(0, std::strcmp(buffer, "world"));
    file.close();
    cleanup_file(path);
    return (1);
}

FT_TEST(test_ft_file_close_when_already_closed_keeps_invalid_fd,
    "ft_file close on unopened object keeps descriptor invalid")
{
    ft_file file;

    file.close();
    FT_ASSERT_EQ(-1, file.get_fd());
    return (1);
}

FT_TEST(test_ft_file_copy_to_with_buffer_requires_open_source,
    "ft_file copy_to_with_buffer fails when source descriptor is invalid")
{
    const char *destination_path;
    ft_file file;

    destination_path = "tmp_ft_file_lifecycle_copy_invalid.txt";
    cleanup_file(destination_path);
    FT_ASSERT_EQ(-1, file.copy_to_with_buffer(destination_path, 64));
    cleanup_file(destination_path);
    return (1);
}

