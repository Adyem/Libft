#include "../test_internal.hpp"
#include "../../YAML/yaml.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <cerrno>
#include <memory>
#include <new>

#ifndef LIBFT_TEST_BUILD
#endif

static void yaml_value_deleter(yaml_value *value)
{
    if (value != ft_nullptr)
        yaml_free(value);
    return ;
}

FT_TEST(test_yaml_round_trip, "yaml round trip")
{
    yaml_value root;
    std::unique_ptr<yaml_value> name_value_guard;
    std::unique_ptr<yaml_value> numbers_value_guard;
    std::unique_ptr<yaml_value> number_one_guard;
    std::unique_ptr<yaml_value> number_two_guard;
    ft_string yaml_string;
    std::unique_ptr<yaml_value, void (*)(yaml_value *)> parsed_value_guard(ft_nullptr, &yaml_value_deleter);
    ft_string round_trip_string;

    if (root.initialize() != FT_ERR_SUCCESS)
        return (0);
    root.set_type(YAML_MAP);
    name_value_guard.reset(new (std::nothrow) yaml_value());
    if (name_value_guard.get() == ft_nullptr)
        return (0);
    if (name_value_guard->initialize() != FT_ERR_SUCCESS)
        return (0);
    ft_string name_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, name_key.initialize("name"));
    ft_string alice_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, alice_value.initialize("Alice"));
    name_value_guard->set_scalar(alice_value);
    root.add_map_item(name_key, name_value_guard.get());
    name_value_guard.release();

    numbers_value_guard.reset(new (std::nothrow) yaml_value());
    if (numbers_value_guard.get() == ft_nullptr)
        return (0);
    if (numbers_value_guard->initialize() != FT_ERR_SUCCESS)
        return (0);
    numbers_value_guard->set_type(YAML_LIST);

    number_one_guard.reset(new (std::nothrow) yaml_value());
    if (number_one_guard.get() == ft_nullptr)
        return (0);
    if (number_one_guard->initialize() != FT_ERR_SUCCESS)
        return (0);
    ft_string one_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, one_value.initialize("one"));
    number_one_guard->set_scalar(one_value);
    numbers_value_guard->add_list_item(number_one_guard.get());
    number_one_guard.release();

    number_two_guard.reset(new (std::nothrow) yaml_value());
    if (number_two_guard.get() == ft_nullptr)
        return (0);
    if (number_two_guard->initialize() != FT_ERR_SUCCESS)
        return (0);
    ft_string two_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, two_value.initialize("two"));
    number_two_guard->set_scalar(two_value);
    numbers_value_guard->add_list_item(number_two_guard.get());
    number_two_guard.release();

    ft_string numbers_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, numbers_key.initialize("numbers"));
    root.add_map_item(numbers_key, numbers_value_guard.get());
    numbers_value_guard.release();

    yaml_string = yaml_write_to_string(&root);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (0);

    parsed_value_guard.reset(yaml_read_from_string(yaml_string));
    if (parsed_value_guard.get() == ft_nullptr)
        return (0);

    round_trip_string = yaml_write_to_string(parsed_value_guard.get());
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(yaml_string.c_str(), round_trip_string.c_str()));
    return (1);
}

FT_TEST(test_yaml_write_to_file_reports_write_failure, "yaml_write_to_file reports write failure")
{
    yaml_value scalar;

    if (scalar.initialize() != FT_ERR_SUCCESS)
        return (0);
    ft_string content_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content_value.initialize("content"));
    scalar.set_scalar(content_value);
    FT_ASSERT_EQ(-1, yaml_write_to_file("/dev/full", &scalar));
    return (1);
}

FT_TEST(test_yaml_value_thread_safety_guard, "yaml_value thread guard handles nested locks")
{
    yaml_value value;
    bool lock_acquired_first;
    bool lock_acquired_second;
    int lock_error_first;
    int lock_error_second;
    int unlock_error_second;
    int unlock_error_first;

    if (value.initialize() != FT_ERR_SUCCESS)
        return (0);
    FT_ASSERT_EQ(false, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(true, value.is_thread_safe());
    lock_acquired_first = false;
    lock_acquired_second = false;
    lock_error_first = value.get_mutex_for_validation()->lock();
    if (lock_error_first == FT_ERR_SUCCESS)
        lock_acquired_first = true;
    lock_error_second = value.get_mutex_for_validation()->lock();
    if (lock_error_second == FT_ERR_SUCCESS)
        lock_acquired_second = true;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error_first);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error_second);
    FT_ASSERT_EQ(true, lock_acquired_first);
    FT_ASSERT_EQ(true, lock_acquired_second);
    unlock_error_second = value.get_mutex_for_validation()->unlock();
    unlock_error_first = value.get_mutex_for_validation()->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error_second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error_first);
    return (1);
}
