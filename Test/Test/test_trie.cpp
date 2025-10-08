#include "../../Template/trie.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_trie_insert_nullptr_sets_errno, "ft_trie insert nullptr key sets FT_ERR_INVALID_ARGUMENT")
{
    ft_trie<int> trie;
    int stored_value = 42;
    size_t valid_key_length = ft_strlen_size_t("valid");
    const char *valid_key = "valid";

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, trie.insert(valid_key, &stored_value));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    const auto *stored_node = trie.search(valid_key);
    FT_ASSERT(stored_node != ft_nullptr);
    FT_ASSERT_EQ(valid_key_length, stored_node->_key_length);
    FT_ASSERT_EQ(0, stored_node->_unset_value);
    FT_ASSERT_EQ(&stored_value, stored_node->_value_pointer);

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(1, trie.insert(ft_nullptr, &stored_value));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(1, trie.get_error());
    FT_ASSERT_EQ(valid_key_length, stored_node->_key_length);
    FT_ASSERT_EQ(0, stored_node->_unset_value);
    FT_ASSERT_EQ(&stored_value, stored_node->_value_pointer);
    return (1);
}
