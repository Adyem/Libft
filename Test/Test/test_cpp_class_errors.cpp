#include "../../CPP_class/class_file.hpp"
#include "../../CPP_class/class_ofstream.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <fcntl.h>
#include <unistd.h>

FT_TEST(test_ft_file_error_resets, "ft_file resets error state after success")
{
    const char *filename = "test_ft_file_error_resets.txt";
    ft_file file;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, file.read(ft_nullptr, 1));
    FT_ASSERT_EQ(FT_EINVAL, file.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(0, file.open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644));
    FT_ASSERT_EQ(ER_SUCCESS, file.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    file.close();
    ::unlink(filename);
    return (1);
}

FT_TEST(test_ft_ofstream_error_resets, "ft_ofstream resets error state after success")
{
    const char *filename = "test_ft_ofstream_error_resets.txt";
    ft_ofstream stream;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(1, stream.open(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, stream.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(0, stream.open(filename));
    FT_ASSERT_EQ(ER_SUCCESS, stream.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(5, stream.write("reset"));
    FT_ASSERT_EQ(ER_SUCCESS, stream.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    stream.close();
    FT_ASSERT_EQ(ER_SUCCESS, stream.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ::unlink(filename);
    return (1);
}
