#include "../../YAML/yaml.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <cerrno>
#include <memory>
#include <new>

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

    root.set_type(YAML_MAP);
    name_value_guard.reset(new (std::nothrow) yaml_value());
    if (name_value_guard.get() == ft_nullptr)
        return (0);
    name_value_guard->set_scalar("Alice");
    if (name_value_guard->get_error() != FT_ERR_SUCCESSS)
        return (0);
    root.add_map_item("name", name_value_guard.get());
    if (root.get_error() != FT_ERR_SUCCESSS)
        return (0);
    name_value_guard.release();

    numbers_value_guard.reset(new (std::nothrow) yaml_value());
    if (numbers_value_guard.get() == ft_nullptr)
        return (0);
    numbers_value_guard->set_type(YAML_LIST);
    if (numbers_value_guard->get_error() != FT_ERR_SUCCESSS)
        return (0);

    number_one_guard.reset(new (std::nothrow) yaml_value());
    if (number_one_guard.get() == ft_nullptr)
        return (0);
    number_one_guard->set_scalar("one");
    if (number_one_guard->get_error() != FT_ERR_SUCCESSS)
        return (0);
    numbers_value_guard->add_list_item(number_one_guard.get());
    if (numbers_value_guard->get_error() != FT_ERR_SUCCESSS)
        return (0);
    number_one_guard.release();

    number_two_guard.reset(new (std::nothrow) yaml_value());
    if (number_two_guard.get() == ft_nullptr)
        return (0);
    number_two_guard->set_scalar("two");
    if (number_two_guard->get_error() != FT_ERR_SUCCESSS)
        return (0);
    numbers_value_guard->add_list_item(number_two_guard.get());
    if (numbers_value_guard->get_error() != FT_ERR_SUCCESSS)
        return (0);
    number_two_guard.release();

    root.add_map_item("numbers", numbers_value_guard.get());
    if (root.get_error() != FT_ERR_SUCCESSS)
        return (0);
    numbers_value_guard.release();

    yaml_string = yaml_write_to_string(&root);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        return (0);

    parsed_value_guard.reset(yaml_read_from_string(yaml_string));
    if (parsed_value_guard.get() == ft_nullptr)
        return (0);
    if (parsed_value_guard->get_error() != FT_ERR_SUCCESSS)
        return (0);

    round_trip_string = yaml_write_to_string(parsed_value_guard.get());
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(yaml_string.c_str(), round_trip_string.c_str()));
    return (1);
}

FT_TEST(test_yaml_write_to_file_reports_write_failure, "yaml_write_to_file reports write failure")
{
    yaml_value scalar;

    scalar.set_scalar("content");
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, yaml_write_to_file("/dev/full", &scalar));
    FT_ASSERT_EQ(FT_ERR_FULL, ft_errno);
    return (1);
}

FT_TEST(test_yaml_value_thread_safety_guard, "yaml_value thread guard handles nested locks")
{
    yaml_value value;
    yaml_value::thread_guard guard(&value);
    yaml_value::thread_guard nested_guard(&value);

    if (value.get_error() != FT_ERR_SUCCESSS)
        return (0);
    FT_ASSERT_EQ(true, value.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, guard.get_status());
    FT_ASSERT_EQ(true, guard.lock_acquired());
    FT_ASSERT_EQ(0, nested_guard.get_status());
    FT_ASSERT_EQ(false, nested_guard.lock_acquired());
    return (1);
}
