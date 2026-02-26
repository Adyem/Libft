#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static ft_size_t string_current_allocated_bytes(void)
{
    ft_size_t allocation_count;
    ft_size_t free_count;
    ft_size_t current_bytes;
    ft_size_t peak_bytes;
    int32_t status;

    allocation_count = 0;
    free_count = 0;
    current_bytes = 0;
    peak_bytes = 0;
    status = cma_get_extended_stats(&allocation_count, &free_count,
            &current_bytes, &peak_bytes);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, status);
    return (current_bytes);
}

FT_TEST(test_ft_string_append_char_allocation_failure,
    "ft_string append(char) reports allocation failure and keeps mutex unlocked")
{
    ft_string string_value;
    int32_t append_error;
    pt_recursive_mutex *mutex_pointer;

    (void)string_value.initialize();
    mutex_pointer = string_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    cma_set_alloc_limit(1);
    append_error = string_value.append('x');
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, append_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_ft_string_append_cstring_allocation_failure,
    "ft_string append(cstring) reports allocation failure")
{
    ft_string string_value;
    int32_t append_error;

    (void)string_value.initialize();
    cma_set_alloc_limit(1);
    append_error = string_value.append("abcd");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, append_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_assign_count_allocation_failure,
    "ft_string assign(count, char) reports allocation failure")
{
    ft_string string_value;
    int32_t assign_error;

    (void)string_value.initialize();
    cma_set_alloc_limit(1);
    assign_error = string_value.assign(8, 'a');
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, assign_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_assign_cstring_allocation_failure,
    "ft_string assign(cstring, length) reports allocation failure")
{
    ft_string string_value;
    int32_t assign_error;

    (void)string_value.initialize();
    cma_set_alloc_limit(1);
    assign_error = string_value.assign("abcdef", 6);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, assign_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_resize_allocation_failure,
    "ft_string resize reports allocation failure")
{
    ft_string string_value;
    int32_t resize_error;

    (void)string_value.initialize();
    cma_set_alloc_limit(1);
    resize_error = string_value.resize(64);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, resize_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_resize_length_allocation_failure,
    "ft_string resize_length reports allocation failure")
{
    ft_string string_value;
    int32_t resize_error;

    (void)string_value.initialize();
    cma_set_alloc_limit(1);
    resize_error = string_value.resize_length(12);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, resize_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_constructor_cstring_allocation_failure,
    "ft_string cstring constructor reports allocation failure")
{
    ft_string string_value;
    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, string_value.initialize("long_payload"));
    cma_set_alloc_limit(0);

    (void)string_value;
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_copy_constructor_allocation_failure,
    "ft_string copy constructor reports allocation failure")
{
    ft_string source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize("copy_payload"));

    cma_set_alloc_limit(1);
    ft_string copy_value(source);
    cma_set_alloc_limit(0);

    (void)copy_value;
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_copy_assignment_allocation_failure,
    "ft_string copy assignment reports allocation failure")
{
    ft_string source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize("copy_payload"));
    ft_string destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize("seed"));

    cma_set_alloc_limit(1);
    destination = source;
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_operator_plus_allocation_failure,
    "ft_string operator+ reports allocation failure")
{
    ft_string left;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left.initialize("left_payload"));
    ft_string right;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right.initialize("right_payload"));
    ft_string result;
    ft_string_proxy proxy_result;

    cma_set_alloc_limit(1);
    proxy_result = left + right;
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, proxy_result.get_error());
    result = proxy_result;
    (void)result;
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_allocation_failure_recovery_after_reset_limit,
    "ft_string recovers after allocation limit is reset")
{
    ft_string string_value;
    int32_t append_error;

    (void)string_value.initialize();
    cma_set_alloc_limit(1);
    append_error = string_value.append("abcd");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, append_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.append("ok"));
    FT_ASSERT(string_value == "ok");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_late_append_growth_failure_releases_memory_after_destroy,
    "ft_string late append growth failure leaves no residual allocation after destroy")
{
    ft_size_t baseline_bytes;
    ft_size_t bytes_after_destroy;
    ft_string string_value;
    int32_t append_error;

    baseline_bytes = string_current_allocated_bytes();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.append("warmup"));
    cma_set_alloc_limit(32);
    append_error = string_value.append(
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, append_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    bytes_after_destroy = string_current_allocated_bytes();
    FT_ASSERT_EQ(baseline_bytes, bytes_after_destroy);
    return (1);
}

FT_TEST(test_ft_string_operator_plus_late_allocation_failure_releases_memory,
    "ft_string operator+ late allocation failure does not leak temporary buffers")
{
    ft_size_t baseline_bytes;
    ft_size_t bytes_after_scope;

    baseline_bytes = string_current_allocated_bytes();
    {
        ft_string left;
        FT_ASSERT_EQ(FT_ERR_SUCCESS, left.initialize("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMN"));
        ft_string right;
        FT_ASSERT_EQ(FT_ERR_SUCCESS, right.initialize("opqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));
        ft_string result;
        ft_string_proxy proxy_result;

        cma_set_alloc_limit(64);
        proxy_result = left + right;
        cma_set_alloc_limit(0);
        FT_ASSERT_EQ(FT_ERR_NO_MEMORY, proxy_result.get_error());
        result = proxy_result;
        FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
        (void)result;
    }
    bytes_after_scope = string_current_allocated_bytes();
    FT_ASSERT_EQ(baseline_bytes, bytes_after_scope);
    return (1);
}
