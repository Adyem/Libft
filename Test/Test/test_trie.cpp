#include "../test_internal.hpp"
#include "../../Template/trie.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_trie_insert_nullptr_sets_errno, "ft_trie insert nullptr key sets FT_ERR_INVALID_ARGUMENT")
{
    ft_trie<int> trie;
    int stored_value = 42;
    size_t valid_key_length = ft_strlen_size_t("valid");
    const char *valid_key = "valid";

    FT_ASSERT_EQ(0, trie.insert(valid_key, &stored_value));
    const auto *stored_node = trie.search(valid_key);
    FT_ASSERT(stored_node != ft_nullptr);
    FT_ASSERT_EQ(valid_key_length, stored_node->_key_length);
    FT_ASSERT_EQ(0, stored_node->_unset_value);
    FT_ASSERT_EQ(&stored_value, stored_node->_value_pointer);

    FT_ASSERT_EQ(1, trie.insert(ft_nullptr, &stored_value));
    FT_ASSERT_EQ(1, trie.get_error());
    FT_ASSERT_EQ(valid_key_length, stored_node->_key_length);
    FT_ASSERT_EQ(0, stored_node->_unset_value);
    FT_ASSERT_EQ(&stored_value, stored_node->_value_pointer);
    return (1);
}

FT_TEST(test_trie_thread_safety_controls_reset_errno,
        "ft_trie optional mutex guard toggles and resets errno on locks")
{
    ft_trie<int> trie;
    bool lock_acquired;

    FT_ASSERT_EQ(false, trie.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, trie.enable_thread_safety());
    FT_ASSERT_EQ(true, trie.is_thread_safe_enabled());
    lock_acquired = false;
    FT_ASSERT_EQ(0, trie.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    trie.unlock(lock_acquired);
    int stored_value = 7;
    FT_ASSERT_EQ(0, trie.insert("key", &stored_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, trie.get_error());
    const auto *node = trie.search("key");
    FT_ASSERT(node != ft_nullptr);
    FT_ASSERT_EQ(&stored_value, node->_value_pointer);
    trie.disable_thread_safety();
    FT_ASSERT_EQ(false, trie.is_thread_safe_enabled());
    return (1);
}
