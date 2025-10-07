#include "../../YAML/yaml.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CMA/CMA.hpp"

FT_TEST(test_yaml_reader_malformed_structure_sets_errno, "yaml_reader reports malformed map indentation")
{
    ft_string malformed = "root:\nvalue";

    ft_errno = ER_SUCCESS;
    yaml_value *result = yaml_read_from_string(malformed);
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_yaml_reader_list_with_premature_eof_sets_errno, "yaml_reader detects premature eof in list item")
{
    ft_string premature = "-\n";

    ft_errno = ER_SUCCESS;
    yaml_value *result = yaml_read_from_string(premature);
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_yaml_reader_allocation_failure_sets_errno, "yaml_reader propagates allocation failures")
{
    ft_string simple = "root: value";
    yaml_value *result;

    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(200);
    result = yaml_read_from_string(simple);
    cma_set_alloc_limit(0);
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}

FT_TEST(test_yaml_reader_scalar_set_scalar_failure, "yaml_reader handles scalar assignment failures")
{
    ft_string content(32, 'A');
    yaml_value *result;

    FT_ASSERT_EQ(ER_SUCCESS, content.get_error());
    cma_set_alloc_limit(32);
    ft_errno = ER_SUCCESS;
    result = yaml_read_from_string(content);
    cma_set_alloc_limit(0);
    if (result != ft_nullptr)
    {
        yaml_free(result);
    }
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(STRING_MEM_ALLOC_FAIL, ft_errno);
    return (1);
}
