#include <cstdlib>
#include "test_scma_shared.hpp"

FT_TEST(test_scma_snapshot_tracks_writes, "scma snapshot tracked buffer receives subsequent writes")
{
    scma_handle handle;
    scma_test_pair pair_value;
    scma_test_pair snapshot_value;
    scma_test_pair updated_value;
    void *snapshot_buffer;
    scma_test_pair *snapshot_pair;
    ft_size_t snapshot_size;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(scma_test_pair))));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(scma_test_pair)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    pair_value.first = 1;
    pair_value.second = 2;
    FT_ASSERT_EQ(1, scma_write(handle, 0, &pair_value, static_cast<ft_size_t>(sizeof(scma_test_pair))));
    snapshot_size = 0;
    snapshot_buffer = scma_snapshot(handle, &snapshot_size);
    FT_ASSERT(snapshot_buffer != ft_nullptr);
    FT_ASSERT_EQ(static_cast<ft_size_t>(sizeof(scma_test_pair)), snapshot_size);
    snapshot_pair = static_cast<scma_test_pair *>(snapshot_buffer);
    snapshot_value.first = snapshot_pair->first;
    snapshot_value.second = snapshot_pair->second;
    FT_ASSERT_EQ(pair_value.first, snapshot_value.first);
    FT_ASSERT_EQ(pair_value.second, snapshot_value.second);
    updated_value.first = 11;
    updated_value.second = 22;
    FT_ASSERT_EQ(1, scma_write(handle, 0, &updated_value, static_cast<ft_size_t>(sizeof(scma_test_pair))));
    snapshot_value.first = snapshot_pair->first;
    snapshot_value.second = snapshot_pair->second;
    FT_ASSERT_EQ(updated_value.first, snapshot_value.first);
    FT_ASSERT_EQ(updated_value.second, snapshot_value.second);
    FT_ASSERT_EQ(1, scma_release_snapshot(snapshot_buffer));
    std::free(snapshot_buffer);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_snapshot_without_size_is_independent, "scma snapshot without size pointer remains static after writes")
{
    scma_handle handle;
    scma_test_pair pair_value;
    scma_test_pair snapshot_value;
    scma_test_pair updated_value;
    void *snapshot_buffer;
    scma_test_pair *snapshot_pair;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(scma_test_pair))));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(scma_test_pair)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    pair_value.first = 3;
    pair_value.second = 6;
    FT_ASSERT_EQ(1, scma_write(handle, 0, &pair_value, static_cast<ft_size_t>(sizeof(scma_test_pair))));
    snapshot_buffer = scma_snapshot(handle, ft_nullptr);
    FT_ASSERT(snapshot_buffer != ft_nullptr);
    snapshot_pair = static_cast<scma_test_pair *>(snapshot_buffer);
    snapshot_value.first = snapshot_pair->first;
    snapshot_value.second = snapshot_pair->second;
    FT_ASSERT_EQ(pair_value.first, snapshot_value.first);
    FT_ASSERT_EQ(pair_value.second, snapshot_value.second);
    updated_value.first = 7;
    updated_value.second = 9;
    FT_ASSERT_EQ(1, scma_write(handle, 0, &updated_value, static_cast<ft_size_t>(sizeof(scma_test_pair))));
    snapshot_value.first = snapshot_pair->first;
    snapshot_value.second = snapshot_pair->second;
    FT_ASSERT_EQ(pair_value.first, snapshot_value.first);
    FT_ASSERT_EQ(pair_value.second, snapshot_value.second);
    FT_ASSERT_EQ(1, scma_release_snapshot(ft_nullptr));
    std::free(snapshot_buffer);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_release_snapshot_validates_pointer, "scma release snapshot rejects mismatched pointer and clears active state")
{
    scma_handle handle;
    scma_test_pair pair_value;
    void *snapshot_buffer;
    void *foreign_buffer;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(scma_test_pair))));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(scma_test_pair)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    pair_value.first = 5;
    pair_value.second = 10;
    FT_ASSERT_EQ(1, scma_write(handle, 0, &pair_value, static_cast<ft_size_t>(sizeof(scma_test_pair))));
    snapshot_buffer = scma_snapshot(handle, ft_nullptr);
    FT_ASSERT(snapshot_buffer != ft_nullptr);
    foreign_buffer = std::malloc(sizeof(scma_test_pair));
    FT_ASSERT(foreign_buffer != ft_nullptr);
    FT_ASSERT_EQ(0, scma_release_snapshot(foreign_buffer));
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_errno);
    FT_ASSERT_EQ(1, scma_release_snapshot(ft_nullptr));
    std::free(snapshot_buffer);
    std::free(foreign_buffer);
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_get_stats_reports_snapshot_activity, "scma stats reflect active snapshot state")
{
    scma_handle handle;
    scma_test_pair pair_value;
    scma_stats stats;
    void *snapshot_buffer;

    FT_ASSERT_EQ(1, scma_test_initialize(static_cast<ft_size_t>(sizeof(scma_test_pair))));
    handle = scma_allocate(static_cast<ft_size_t>(sizeof(scma_test_pair)));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    pair_value.first = 12;
    pair_value.second = 24;
    FT_ASSERT_EQ(1, scma_write(handle, 0, &pair_value, static_cast<ft_size_t>(sizeof(scma_test_pair))));
    FT_ASSERT_EQ(1, scma_get_stats(&stats));
    FT_ASSERT_EQ(0, stats.snapshot_active);
    snapshot_buffer = scma_snapshot(handle, &stats.heap_capacity);
    FT_ASSERT(snapshot_buffer != ft_nullptr);
    FT_ASSERT_EQ(1, scma_get_stats(&stats));
    FT_ASSERT_EQ(1, stats.snapshot_active);
    FT_ASSERT_EQ(1, scma_release_snapshot(snapshot_buffer));
    FT_ASSERT_EQ(1, scma_get_stats(&stats));
    FT_ASSERT_EQ(0, stats.snapshot_active);
    std::free(snapshot_buffer);
    scma_shutdown();
    return (1);
}
