#include "../test_internal.hpp"
#include "test_scma_shared.hpp"
#include "../../CPP_class/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int scma_expect_recursive_mutex_usable(pt_recursive_mutex *mutex_pointer)
{
    int lock_error;
    int unlock_error;

    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    FT_ASSERT_EQ(true, mutex_pointer->lockState());
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    FT_ASSERT_EQ(true, mutex_pointer->lockState());
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (FT_ERR_SUCCESS);
}

FT_TEST(test_scma_runtime_recursive_mutex_depth_is_balanced,
    "scma runtime mutex supports nested lock and unlock")
{
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(int)));
    mutex_pointer = scma_runtime_mutex();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_expect_recursive_mutex_usable(mutex_pointer));
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_accessor_methods_leave_runtime_mutex_unlocked,
    "scma accessor methods leave runtime recursive mutex unlocked on return")
{
    scma_handle handle;
    scma_handle_accessor<int> accessor;
    pt_recursive_mutex *mutex_pointer;
    int read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(int) * 2));
    handle = scma_allocate(sizeof(int) * 2);
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    FT_ASSERT_EQ(1, accessor.bind(handle));
    FT_ASSERT_EQ(1, accessor.write_struct(42));
    FT_ASSERT_EQ(1, accessor.read_struct(read_value));
    FT_ASSERT_EQ(42, read_value);
    FT_ASSERT_EQ(1, accessor.write_at(77, 1));
    FT_ASSERT_EQ(1, accessor.read_at(read_value, 1));
    FT_ASSERT_EQ(77, read_value);
    mutex_pointer = scma_runtime_mutex();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(0U, scma_mutex_lock_count());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_failure_paths_leave_runtime_mutex_unlocked,
    "scma failure paths leave runtime recursive mutex unlocked")
{
    scma_handle handle;
    scma_handle invalid_handle;
    scma_handle_accessor<int> accessor;
    pt_recursive_mutex *mutex_pointer;
    int read_value;

    invalid_handle.index = FT_SYSTEM_SIZE_MAX;
    invalid_handle.generation = FT_SYSTEM_SIZE_MAX;
    read_value = 0;
    FT_ASSERT_EQ(1, scma_test_initialize(sizeof(int)));
    handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, accessor.initialize());
    FT_ASSERT_EQ(0, accessor.bind(invalid_handle));
    FT_ASSERT_EQ(FT_ERR_INVALID_HANDLE, accessor.get_error());
    FT_ASSERT_EQ(0, accessor.read_at(read_value, 99));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, accessor.get_error());
    FT_ASSERT_EQ(0, scma_write(invalid_handle, 0, &read_value, sizeof(int)));
    mutex_pointer = scma_runtime_mutex();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(0U, scma_mutex_lock_count());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_late_allocate_failure_leaves_runtime_mutex_unlocked,
    "scma late allocation failure keeps runtime recursive mutex balanced")
{
    scma_handle stable_handle;
    scma_handle failed_handle;
    pt_recursive_mutex *mutex_pointer;
    int stored_value;
    int read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(128));
    stable_handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(stable_handle));
    stored_value = 42;
    FT_ASSERT_EQ(1, scma_write(stable_handle, 0, &stored_value, sizeof(int)));
    failed_handle = scma_allocate(FT_SYSTEM_SIZE_MAX);
    FT_ASSERT_EQ(0, scma_handle_is_valid(failed_handle));
    read_value = 0;
    FT_ASSERT_EQ(1, scma_read(stable_handle, 0, &read_value, sizeof(int)));
    FT_ASSERT_EQ(stored_value, read_value);
    mutex_pointer = scma_runtime_mutex();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(0U, scma_mutex_lock_count());
    scma_shutdown();
    return (1);
}

FT_TEST(test_scma_late_resize_failure_leaves_runtime_mutex_unlocked,
    "scma late resize failure keeps runtime recursive mutex balanced")
{
    scma_handle handle;
    pt_recursive_mutex *mutex_pointer;
    int stored_value;
    int read_value;

    FT_ASSERT_EQ(1, scma_test_initialize(128));
    handle = scma_allocate(sizeof(int));
    FT_ASSERT_EQ(1, scma_handle_is_valid(handle));
    stored_value = 99;
    FT_ASSERT_EQ(1, scma_write(handle, 0, &stored_value, sizeof(int)));
    FT_ASSERT_EQ(0, scma_resize(handle, FT_SYSTEM_SIZE_MAX));
    read_value = 0;
    FT_ASSERT_EQ(1, scma_read(handle, 0, &read_value, sizeof(int)));
    FT_ASSERT_EQ(stored_value, read_value);
    mutex_pointer = scma_runtime_mutex();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scma_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(0U, scma_mutex_lock_count());
    scma_shutdown();
    return (1);
}
