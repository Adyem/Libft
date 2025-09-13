#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdio>

static void create_test_file(void)
{
    FILE *file = ft_fopen("test_file_io.txt", "w");
    if (file)
    {
        std::fputs("Line1\nLine2\n", file);
        ft_fclose(file);
    }
    return ;
}

FT_TEST(test_fopen_valid, "ft_fopen and ft_fclose basic")
{
    FILE *file;

    create_test_file();
    file = ft_fopen("test_file_io.txt", "r");
    FT_ASSERT(file != ft_nullptr);
    FT_ASSERT_EQ(0, ft_fclose(file));
    return (1);
}

FT_TEST(test_fopen_invalid, "ft_fopen invalid path")
{
    FT_ASSERT_EQ(ft_nullptr, ft_fopen("missing_file.txt", "r"));
    return (1);
}

FT_TEST(test_fopen_null, "ft_fopen with null")
{
    FT_ASSERT_EQ(ft_nullptr, ft_fopen(ft_nullptr, "r"));
    FT_ASSERT_EQ(ft_nullptr, ft_fopen("test_file_io.txt", ft_nullptr));
    return (1);
}

FT_TEST(test_fclose_null, "ft_fclose with null")
{
    FT_ASSERT_EQ(EOF, ft_fclose(ft_nullptr));
    return (1);
}

FT_TEST(test_fgets_basic, "ft_fgets basic")
{
    char buffer[16];
    FILE *file;

    create_test_file();
    file = ft_fopen("test_file_io.txt", "r");
    if (file == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(buffer, ft_fgets(buffer, 16, file));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "Line1\n"));
    FT_ASSERT_EQ(buffer, ft_fgets(buffer, 16, file));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "Line2\n"));
    ft_fclose(file);
    return (1);
}

FT_TEST(test_fgets_edge_cases, "ft_fgets edge cases")
{
    char buffer[8];
    FILE *file;

    create_test_file();
    file = ft_fopen("test_file_io.txt", "r");
    if (file == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(buffer, ft_fgets(buffer, 5, file));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "Line"));
    FT_ASSERT_EQ(ft_nullptr, ft_fgets(ft_nullptr, 5, file));
    FT_ASSERT_EQ(ft_nullptr, ft_fgets(buffer, 0, file));
    FT_ASSERT_EQ(ft_nullptr, ft_fgets(buffer, 5, ft_nullptr));
    ft_fclose(file);
    return (1);
}
