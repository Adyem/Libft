#include "../../Template/trie.hpp"
#include "../../System_utils/test_runner.hpp"
#include <type_traits>

FT_TEST(test_ft_trie_copy_operations_deleted,
        "ft_trie copy constructor and assignment are deleted to avoid mutex sharing")
{
    bool copy_constructible;
    bool copy_assignable;

    copy_constructible = std::is_copy_constructible<ft_trie<int> >::value;
    copy_assignable = std::is_copy_assignable<ft_trie<int> >::value;
    FT_ASSERT(!copy_constructible);
    FT_ASSERT(!copy_assignable);
    return (1);
}

FT_TEST(test_ft_trie_move_operations_deleted,
        "ft_trie move constructor and assignment are deleted to keep mutex fresh")
{
    bool move_constructible;
    bool move_assignable;

    move_constructible = std::is_move_constructible<ft_trie<int> >::value;
    move_assignable = std::is_move_assignable<ft_trie<int> >::value;
    FT_ASSERT(!move_constructible);
    FT_ASSERT(!move_assignable);
    return (1);
}
