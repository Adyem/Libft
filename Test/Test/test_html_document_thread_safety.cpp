#include "../test_internal.hpp"
#include "../../Modules/HTML/html_document.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/HTML/html_parser.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_html_document_manual_thread_safety_lifecycle)
{
    html_document document;
    int initialize_error;

    initialize_error = document.initialize();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, initialize_error);
    FT_ASSERT_EQ(false, document.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, document.enable_thread_safety());
    FT_ASSERT_EQ(true, document.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, document.disable_thread_safety());
    FT_ASSERT_EQ(false, document.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, document.destroy());
    return (1);
}

FT_TEST(test_html_document_create_node_without_thread_safety)
{
    html_document document;
    html_node *node;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, document.initialize());
    FT_ASSERT_EQ(false, document.is_thread_safe());
    node = document.create_node("div", "text");
    FT_ASSERT(node != ft_nullptr);
    html_free_nodes(node);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, document.destroy());
    return (1);
}

FT_TEST(test_html_document_enable_thread_safety_allocation_failure)
{
    html_document document;
    int thread_safety_error;
    html_node *node;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, document.initialize());
    cma_set_alloc_limit(1);
    thread_safety_error = document.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, thread_safety_error);
    FT_ASSERT_EQ(false, document.is_thread_safe());
    node = document.create_node("div", "text");
    FT_ASSERT(node != ft_nullptr);
    html_free_nodes(node);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, document.destroy());
    return (1);
}
