#include "../test_internal.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/Template/map.hpp"
#include "../../Modules/Template/unordered_map.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_vector_push_back_allocation_failure_sets_errno)
{
    ft_vector<int> vector_instance;
    size_t fill_index;
    size_t inline_capacity_limit;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_instance.initialize());
    fill_index = 0;
    inline_capacity_limit = 8;
    while (fill_index < inline_capacity_limit)
    {
        vector_instance.push_back(static_cast<int>(fill_index));
        FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_instance.get_error());
        fill_index += 1;
    }

    cma_set_alloc_limit(1);
    vector_instance.push_back(42);
    int push_error_code;

    push_error_code = vector_instance.get_error();
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, push_error_code);

    size_t vector_size_after_failure;

    vector_size_after_failure = vector_instance.size();
    FT_ASSERT_EQ(inline_capacity_limit, vector_size_after_failure);

    return (1);
}

FT_TEST(test_ft_map_resize_allocation_failure_sets_errno)
{
    ft_map<int, int> map_instance(1);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());

    map_instance.insert(1, 1);

    cma_set_alloc_limit(1);
    map_instance.insert(2, 2);
    cma_set_alloc_limit(0);

    size_t expected_map_size;

    expected_map_size = 1;
    FT_ASSERT_EQ(expected_map_size, map_instance.size());

    return (1);
}

FT_TEST(test_ft_unordered_map_initial_allocation_failure_sets_errno)
{
    ft_unordered_map<int, int> limited_map(32);
    int32_t initialize_error_code;

    cma_set_alloc_limit(1);
    initialize_error_code = limited_map.initialize();
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, initialize_error_code);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, limited_map.initialize());
    limited_map.insert(1, 1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, limited_map.get_error());
    FT_ASSERT_EQ(static_cast<size_t>(1), limited_map.size());

    return (1);
}
