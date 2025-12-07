#include "../../HTML/document.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_html_document_thread_guard_resets_errno,
        "html_document::thread_guard resets errno to success when locking")
{
    html_document document;

    FT_ASSERT_EQ(true, document.is_thread_safe_enabled());
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    {
        html_document::thread_guard guard(&document);

        FT_ASSERT_EQ(0, guard.get_status());
        FT_ASSERT_EQ(true, guard.lock_acquired());
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    }
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_html_document_thread_guard_tolerates_null_document,
        "html_document::thread_guard allows null documents without locking")
{
    ft_errno = FT_ERR_INTERNAL;
    html_document::thread_guard guard(ft_nullptr);

    FT_ASSERT_EQ(0, guard.get_status());
    FT_ASSERT_EQ(false, guard.lock_acquired());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_html_document_handles_mutex_allocation_failure,
        "html_document handles mutex allocation failures and remains usable")
{
    cma_set_alloc_limit(1);
    html_document failure_document;
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, failure_document.get_error());
    FT_ASSERT_EQ(false, failure_document.is_thread_safe_enabled());
    ft_errno = FT_ERR_INTERNAL;
    {
        html_document::thread_guard guard(&failure_document);

        FT_ASSERT_EQ(0, guard.get_status());
        FT_ASSERT_EQ(false, guard.lock_acquired());
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    }
    html_node *node = failure_document.create_node("div", "text");

    FT_ASSERT(node != ft_nullptr);
    html_free_nodes(node);
    return (1);
}
