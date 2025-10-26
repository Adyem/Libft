#include "../../Template/vector.hpp"
#include "../../Template/map.hpp"
#include "../../Template/unordered_map.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_vector_push_back_allocation_failure_sets_errno,
        "ft_vector push_back surfaces allocation failure through error code")
{
    ft_vector<int> vector_instance;
    size_t fill_index;
    size_t inline_capacity_limit;

    ft_errno = ER_SUCCESS;
    fill_index = 0;
    inline_capacity_limit = 8;
    while (fill_index < inline_capacity_limit)
    {
        vector_instance.push_back(static_cast<int>(fill_index));
        FT_ASSERT_EQ(ER_SUCCESS, vector_instance.get_error());
        fill_index += 1;
    }

    cma_set_alloc_limit(1);
    vector_instance.push_back(42);
    int push_error_code;

    push_error_code = vector_instance.get_error();
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, push_error_code);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);

    size_t vector_size_after_failure;

    vector_size_after_failure = vector_instance.size();
    FT_ASSERT_EQ(inline_capacity_limit, vector_size_after_failure);

    ft_errno = ER_SUCCESS;
    return (1);
}

FT_TEST(test_ft_map_resize_allocation_failure_sets_errno,
        "ft_map resize failure propagates allocation error through _error")
{
    ft_map<int, int> map_instance(1);

    FT_ASSERT_EQ(ER_SUCCESS, map_instance.get_error());

    map_instance.insert(1, 1);
    FT_ASSERT_EQ(ER_SUCCESS, map_instance.get_error());

    cma_set_alloc_limit(1);
    map_instance.insert(2, 2);
    int map_insert_error;

    map_insert_error = map_instance.get_error();
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, map_insert_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);

    size_t expected_map_size;

    expected_map_size = 1;
    FT_ASSERT_EQ(expected_map_size, map_instance.size());

    ft_errno = ER_SUCCESS;
    return (1);
}

FT_TEST(test_ft_unordered_map_initial_allocation_failure_sets_errno,
        "ft_unordered_map constructor reports allocation failure")
{
    ft_unordered_map<int, int> empty_map(0);

    FT_ASSERT_EQ(ER_SUCCESS, empty_map.get_error());

    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(1);
    ft_unordered_map<int, int> limited_map(32);
    int construction_error_code;

    construction_error_code = limited_map.get_error();
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, construction_error_code);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    FT_ASSERT(limited_map.has_valid_storage() == false);

    ft_errno = ER_SUCCESS;
    return (1);
}
