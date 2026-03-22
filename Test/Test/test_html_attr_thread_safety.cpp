#include "../test_internal.hpp"
#include "../../HTML/html_parser.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static void release_html_string(char *string)
{
    if (!string)
        return ;
    int release_result;

    release_result = cma_checked_free(string);
    if (release_result != 0)
        return ;
    return ;
}

static void release_html_attribute(html_attr *attribute)
{
    if (!attribute)
        return ;
    release_html_string(attribute->key);
    release_html_string(attribute->value);
    html_attr_teardown_thread_safety(attribute);
    delete attribute;
    return ;
}

FT_TEST(test_html_attr_creation_enables_thread_safety)
{
    html_attr *attribute;

    attribute = html_create_attr("id", "root");
    FT_ASSERT(attribute != ft_nullptr);
    FT_ASSERT_EQ(false, html_attr_is_thread_safe_enabled(attribute));
    FT_ASSERT_EQ(0, html_attr_prepare_thread_safety(attribute));
    FT_ASSERT_EQ(true, html_attr_is_thread_safe_enabled(attribute));
    release_html_attribute(attribute);
    return (1);
}

FT_TEST(test_html_attr_prepare_thread_safety_restores_mutex)
{
    html_attr *attribute;

    attribute = html_create_attr("class", "item");
    FT_ASSERT(attribute != ft_nullptr);
    html_attr_teardown_thread_safety(attribute);
    FT_ASSERT_EQ(false, html_attr_is_thread_safe_enabled(attribute));
    FT_ASSERT_EQ(0, html_attr_prepare_thread_safety(attribute));
    FT_ASSERT_EQ(true, html_attr_is_thread_safe_enabled(attribute));
    release_html_attribute(attribute);
    return (1);
}

FT_TEST(test_html_attr_lock_resets_errno)
{
    html_attr *attribute;
    ft_bool    lock_acquired;

    attribute = html_create_attr("data", "value");
    FT_ASSERT(attribute != ft_nullptr);
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, html_attr_lock(attribute, &lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    html_attr_unlock(attribute, lock_acquired);
    release_html_attribute(attribute);
    return (1);
}

FT_TEST(test_html_attr_teardown_clears_mutex)
{
    html_attr *attribute;

    attribute = html_create_attr("role", "button");
    FT_ASSERT(attribute != ft_nullptr);
    FT_ASSERT(attribute->mutex == ft_nullptr);
    FT_ASSERT_EQ(false, html_attr_is_thread_safe_enabled(attribute));
    FT_ASSERT_EQ(0, html_attr_prepare_thread_safety(attribute));
    FT_ASSERT(attribute->mutex != ft_nullptr);
    FT_ASSERT_EQ(true, html_attr_is_thread_safe_enabled(attribute));
    html_attr_teardown_thread_safety(attribute);
    FT_ASSERT_EQ(false, html_attr_is_thread_safe_enabled(attribute));
    FT_ASSERT(attribute->mutex == ft_nullptr);
    release_html_attribute(attribute);
    return (1);
}

FT_TEST(test_html_attr_lock_after_teardown_skips_mutex)
{
    html_attr *attribute;
    ft_bool    lock_acquired;
    int        lock_result;

    attribute = html_create_attr("lang", "en");
    FT_ASSERT(attribute != ft_nullptr);
    html_attr_teardown_thread_safety(attribute);
    FT_ASSERT_EQ(false, html_attr_is_thread_safe_enabled(attribute));
    lock_acquired = FT_TRUE;
    lock_result = html_attr_lock(attribute, &lock_acquired);
    FT_ASSERT_EQ(0, lock_result);
    FT_ASSERT_EQ(FT_FALSE, lock_acquired);
    release_html_attribute(attribute);
    return (1);
}

FT_TEST(test_html_attr_lock_null_sets_errno)
{
    ft_bool lock_acquired;
    int  lock_result;

    lock_acquired = FT_TRUE;
    lock_result = html_attr_lock(ft_nullptr, &lock_acquired);
    FT_ASSERT_EQ(3, lock_result);
    FT_ASSERT_EQ(FT_FALSE, lock_acquired);
    return (1);
}

FT_TEST(test_html_attr_unlock_null_resets_errno)
{
    html_attr_unlock(ft_nullptr, FT_TRUE);
    return (1);
}
