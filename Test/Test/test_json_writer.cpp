#include "../../JSon/json.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

static json_group *create_sample_group(void)
{
    json_group *group = json_create_json_group("sample");
    if (!group)
        return (ft_nullptr);
    json_item *item = json_create_item("key", "value");
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    return (group);
}

FT_TEST(test_json_write_to_string_initial_alloc_failure_sets_errno, "json_write_to_string reports errno when the initial buffer allocation fails")
{
    ft_errno = FT_ER_SUCCESSS;
    cma_set_alloc_limit(1);
    char *result = json_write_to_string(ft_nullptr);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, result);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_json_write_to_string_midway_failure_sets_errno, "json_write_to_string sets allocator errno on intermediate failures")
{
    json_group *group = create_sample_group();
    FT_ASSERT(group != ft_nullptr);
    ft_errno = FT_ERR_CONFIGURATION;
    cma_set_alloc_limit(16);
    char *result = json_write_to_string(group);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, result);
    FT_ASSERT_NE(FT_ERR_CONFIGURATION, ft_errno);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    json_free_groups(group);
    return (1);
}

FT_TEST(test_json_write_to_string_success_resets_errno, "json_write_to_string clears errno after a successful serialization")
{
    json_group *group = create_sample_group();
    FT_ASSERT(group != ft_nullptr);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    char *result = json_write_to_string(group);
    FT_ASSERT(result != ft_nullptr);
    const char *expected = "{\n  \"sample\": {\n    \"key\": \"value\"\n  }\n}\n";
    FT_ASSERT_EQ(0, ft_strcmp(expected, result));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    cma_free(result);
    json_free_groups(group);
    return (1);
}
