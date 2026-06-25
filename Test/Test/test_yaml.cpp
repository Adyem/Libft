#include "../test_internal.hpp"
#include "../../Modules/YAML/yaml.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include <cerrno>
#include <memory>
#include <new>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_yaml_round_trip)
{
    yaml_value root;
    std::unique_ptr<yaml_value> name_value_guard;
    std::unique_ptr<yaml_value> role_value_guard;
    ft_string *yaml_string;
    ft_string *round_trip_string;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, root.initialize());
    root.set_type(YAML_MAP);
    name_value_guard.reset(new (std::nothrow) yaml_value());
    FT_ASSERT(name_value_guard.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, name_value_guard->initialize());
    ft_string name_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, name_key.initialize("name"));
    ft_string alice_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, alice_value.initialize("Alice"));
    name_value_guard->set_scalar(alice_value);
    root.add_map_item(name_key, name_value_guard.get());
    name_value_guard.release();

    role_value_guard.reset(new (std::nothrow) yaml_value());
    FT_ASSERT(role_value_guard.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, role_value_guard->initialize());
    ft_string role_key;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, role_key.initialize("role"));
    ft_string role_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, role_value.initialize("ranger"));
    role_value_guard->set_scalar(role_value);
    root.add_map_item(role_key, role_value_guard.get());
    role_value_guard.release();

    yaml_string = yaml_write_to_string(&root);
    FT_ASSERT(yaml_string != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, yaml_string->is_initialised());

    round_trip_string = yaml_write_to_string(&root);
    FT_ASSERT(round_trip_string != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, round_trip_string->is_initialised());
    FT_ASSERT_EQ(0, ft_strcmp(yaml_string->c_str(), round_trip_string->c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, yaml_string->destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, round_trip_string->destroy());
    delete yaml_string;
    delete round_trip_string;
    return (1);
}

FT_TEST(test_yaml_write_to_file_reports_write_failure)
{
    yaml_value scalar;

    if (scalar.initialize() != FT_ERR_SUCCESS)
        return (0);
    ft_string content_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content_value.initialize("content"));
    scalar.set_scalar(content_value);
    FT_ASSERT_EQ(FT_ERR_IO, yaml_write_to_file("/dev/full", &scalar));
    return (1);
}

FT_TEST(test_yaml_value_thread_safety_guard)
{
    yaml_value value;

    if (value.initialize() != FT_ERR_SUCCESS)
        return (0);
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}
