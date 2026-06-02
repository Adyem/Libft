#include "../test_internal.hpp"
#include "../../Modules/Template/trie.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/unordered_map.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static int trie_expect_sigabrt(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static ft_bool g_trie_get_error_returned = FT_FALSE;
static int32_t g_trie_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_trie_get_error_str_returned = FT_FALSE;
static const char *g_trie_get_error_str_result = ft_nullptr;

static void trie_get_error_uninitialised_operation(void)
{
    ft_trie<int> trie_value;

    g_trie_get_error_result = trie_value.get_error();
    g_trie_get_error_returned = FT_TRUE;
    return ;
}

static void trie_get_error_str_uninitialised_operation(void)
{
    ft_trie<int> trie_value;

    g_trie_get_error_str_result = trie_value.get_error_str();
    g_trie_get_error_str_returned = FT_TRUE;
    return ;
}

FT_TEST(test_ft_trie_move_constructor_preserves_entries_and_thread_safety)
{
    ft_trie<int> source_trie;
    ft_trie<int> *moved_trie_pointer;
    int stored_value;

    stored_value = 42;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.insert("answer", &stored_value, -1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.get_error());
    FT_ASSERT_EQ(FT_TRUE, source_trie.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.get_error());

    moved_trie_pointer = new ft_trie<int>();
    FT_ASSERT(moved_trie_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_trie_pointer->move(source_trie));
    FT_ASSERT_EQ(FT_TRUE, moved_trie_pointer->is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_trie_pointer->get_error());
    const auto *found_value = moved_trie_pointer->search("answer");
    FT_ASSERT(found_value != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_trie_pointer->get_error());
    FT_ASSERT_EQ(42, *found_value->_value_pointer);
    FT_ASSERT_EQ((ft_size_t)6, found_value->_key_length);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_trie_pointer->destroy());
    delete moved_trie_pointer;
    return (1);
}

FT_TEST(test_ft_trie_move_method_preserves_entries_and_disabled_thread_safety)
{
    ft_trie<int> source_trie;
    ft_trie<int> destination_trie;
    int stored_value;

    stored_value = 7;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.insert("key", &stored_value, 9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.get_error());
    FT_ASSERT_EQ(FT_FALSE, source_trie.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_trie.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_trie.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_trie.move(source_trie));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_trie.get_error());
    FT_ASSERT_EQ(FT_FALSE, destination_trie.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_trie.get_error());
    const auto *found_value = destination_trie.search("key");
    FT_ASSERT(found_value != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_trie.get_error());
    FT_ASSERT_EQ(7, *found_value->_value_pointer);
    FT_ASSERT_EQ(9, found_value->_unset_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_trie.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_trie.destroy());
    return (1);
}

FT_TEST(test_ft_trie_error_queries_follow_lifecycle_contract)
{
    ft_trie<int> trie_value;

    g_trie_get_error_returned = FT_FALSE;
    g_trie_get_error_result = FT_ERR_SUCCESS;
    g_trie_get_error_str_returned = FT_FALSE;
    g_trie_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, trie_expect_sigabrt(
        trie_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_trie_get_error_returned);
    FT_ASSERT_EQ(1, trie_expect_sigabrt(
        trie_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_trie_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, trie_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, trie_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, trie_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, trie_value.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(trie_value.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}
