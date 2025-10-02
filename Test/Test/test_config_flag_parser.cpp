#include "../../Config/flag_parser.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"

FT_TEST(test_cnfg_parse_flags_collects_unique_alpha, "cnfg_parse_flags collects unique alphabetical short flags")
{
    char program_argument[] = "program";
    char short_flags_one[] = "-av";
    char short_flags_two[] = "-b";
    char duplicate_short_flag[] = "-a";
    char numeric_argument[] = "-1";
    char long_flag_argument[] = "--long";
    char mixed_argument[] = "-C9";
    char *arguments[] = {
        program_argument,
        short_flags_one,
        short_flags_two,
        duplicate_short_flag,
        numeric_argument,
        long_flag_argument,
        mixed_argument
    };
    ft_errno = FT_EALLOC;
    char *parsed_flags = cnfg_parse_flags(7, arguments);
    FT_ASSERT(parsed_flags != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(parsed_flags[0] == 'a');
    FT_ASSERT(parsed_flags[1] == 'v');
    FT_ASSERT(parsed_flags[2] == 'b');
    FT_ASSERT(parsed_flags[3] == 'C');
    FT_ASSERT(parsed_flags[4] == '\0');
    cma_free(parsed_flags);
    return (1);
}

FT_TEST(test_cnfg_parse_flags_null_arguments_set_errno, "cnfg_parse_flags rejects null argument arrays")
{
    ft_errno = ER_SUCCESS;
    char *parsed_flags = cnfg_parse_flags(2, ft_nullptr);
    FT_ASSERT(parsed_flags == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cnfg_parse_long_flags_deduplicates, "cnfg_parse_long_flags ignores duplicates and short arguments")
{
    char program_argument[] = "program";
    char long_flag_one[] = "--alpha";
    char long_flag_two[] = "--beta";
    char duplicate_long_flag[] = "--alpha";
    char short_argument[] = "-z";
    char *arguments[] = {
        program_argument,
        long_flag_one,
        long_flag_two,
        duplicate_long_flag,
        short_argument
    };
    ft_errno = FT_EINVAL;
    char **parsed_flags = cnfg_parse_long_flags(5, arguments);
    FT_ASSERT(parsed_flags != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(parsed_flags[0] != ft_nullptr);
    FT_ASSERT(ft_strcmp(parsed_flags[0], "alpha") == 0);
    FT_ASSERT(parsed_flags[1] != ft_nullptr);
    FT_ASSERT(ft_strcmp(parsed_flags[1], "beta") == 0);
    FT_ASSERT(parsed_flags[2] == ft_nullptr);
    size_t index = 0;
    while (parsed_flags[index])
    {
        cma_free(parsed_flags[index]);
        ++index;
    }
    cma_free(parsed_flags);
    return (1);
}

FT_TEST(test_cnfg_parse_long_flags_null_arguments_set_errno, "cnfg_parse_long_flags rejects null argument arrays")
{
    ft_errno = ER_SUCCESS;
    char **parsed_flags = cnfg_parse_long_flags(2, ft_nullptr);
    FT_ASSERT(parsed_flags == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cnfg_flag_parser_parse_populates_counts, "cnfg_flag_parser parses and tracks short and long flags")
{
    char program_argument[] = "program";
    char short_flag_a[] = "-a";
    char long_flag_long[] = "--long";
    char long_flag_extra[] = "--extra";
    char short_flag_b[] = "-b";
    char *arguments[] = {
        program_argument,
        short_flag_a,
        long_flag_long,
        long_flag_extra,
        short_flag_b
    };
    cnfg_flag_parser parser;
    ft_errno = FT_EINVAL;
    bool parse_result = parser.parse(5, arguments);
    FT_ASSERT(parse_result);
    FT_ASSERT_EQ(ER_SUCCESS, parser.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(parser.get_short_flag_count() == 2);
    FT_ASSERT(parser.get_long_flag_count() == 2);
    FT_ASSERT(parser.get_total_flag_count() == 4);
    FT_ASSERT(parser.has_short_flag('a'));
    FT_ASSERT(parser.has_short_flag('b'));
    FT_ASSERT(!parser.has_short_flag('c'));
    FT_ASSERT(parser.has_long_flag("long"));
    FT_ASSERT(parser.has_long_flag("extra"));
    FT_ASSERT(!parser.has_long_flag("missing"));
    return (1);
}

FT_TEST(test_cnfg_flag_parser_failure_resets_state, "cnfg_flag_parser clears flags when parsing fails")
{
    char program_argument[] = "program";
    char short_flag_a[] = "-a";
    char *valid_arguments[] = {
        program_argument,
        short_flag_a
    };
    cnfg_flag_parser parser;
    bool initial_parse = parser.parse(2, valid_arguments);
    FT_ASSERT(initial_parse);
    FT_ASSERT(parser.get_total_flag_count() == 1);
    bool parse_result = parser.parse(2, ft_nullptr);
    FT_ASSERT(!parse_result);
    FT_ASSERT_EQ(FT_EINVAL, parser.get_error());
    FT_ASSERT(parser.get_short_flag_count() == 0);
    FT_ASSERT(parser.get_long_flag_count() == 0);
    FT_ASSERT(parser.get_total_flag_count() == 0);
    FT_ASSERT(!parser.has_short_flag('a'));
    FT_ASSERT(!parser.has_long_flag("anything"));
    return (1);
}
