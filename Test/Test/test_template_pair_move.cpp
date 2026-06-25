#include "../test_internal.hpp"
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/CPP_class/class_data_buffer.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/vector.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_pair_copy_constructor_clones_values)
{
    Pair<int32_t, int32_t> original_pair(3, 5);
    Pair<int32_t, int32_t> copied_pair(original_pair);

    FT_ASSERT_EQ(3, copied_pair.key);
    FT_ASSERT_EQ(5, copied_pair.value);
    FT_ASSERT_EQ(3, copied_pair.get_key());
    FT_ASSERT_EQ(5, copied_pair.get_value());
    return (1);
}

FT_TEST(test_template_pair_move_constructor_transfers_values)
{
    Pair<int32_t, int32_t> source_pair(7, 9);
    Pair<int32_t, int32_t> moved_pair(ft_move(source_pair));

    FT_ASSERT_EQ(7, moved_pair.key);
    FT_ASSERT_EQ(9, moved_pair.value);
    FT_ASSERT_EQ(7, moved_pair.get_key());
    FT_ASSERT_EQ(9, moved_pair.get_value());
    return (1);
}

FT_TEST(test_template_pair_move_assignment_updates_values)
{
    Pair<int32_t, int32_t> destination_pair(2, 4);
    Pair<int32_t, int32_t> source_pair(12, 14);

    destination_pair = ft_move(source_pair);

    FT_ASSERT_EQ(12, destination_pair.key);
    FT_ASSERT_EQ(14, destination_pair.value);
    FT_ASSERT_EQ(12, destination_pair.get_key());
    FT_ASSERT_EQ(14, destination_pair.get_value());
    return (1);
}

FT_TEST(test_template_pair_move_uses_member_move_for_lifecycle_values)
{
    DataBuffer source_buffer;
    Pair<int32_t, DataBuffer> source_pair;
    Pair<int32_t, DataBuffer> moved_pair;
    int32_t read_value;

    read_value = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, (source_buffer << 55).get_error());
    source_pair.key = 42;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pair.value.move(source_buffer));

    Pair<int32_t, DataBuffer> local_pair(ft_move(source_pair));
    moved_pair = ft_move(local_pair);

    FT_ASSERT_EQ(42, moved_pair.key);
    FT_ASSERT_EQ(FT_TRUE, moved_pair.value.is_thread_safe());
    moved_pair.value >> read_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_pair.value.get_operation_error());
    FT_ASSERT_EQ(55, read_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_pair.value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_buffer.destroy());
    return (1);
}
