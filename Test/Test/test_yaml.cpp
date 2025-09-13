#include "../../YAML/yaml.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_yaml_round_trip, "yaml round trip")
{
    yaml_value root;
    root.set_type(YAML_MAP);

    yaml_value *name_value = new yaml_value();
    name_value->set_scalar("Alice");
    root.add_map_item("name", name_value);

    yaml_value *numbers_value = new yaml_value();
    numbers_value->set_type(YAML_LIST);

    yaml_value *number_one = new yaml_value();
    number_one->set_scalar("one");
    numbers_value->add_list_item(number_one);

    yaml_value *number_two = new yaml_value();
    number_two->set_scalar("two");
    numbers_value->add_list_item(number_two);

    root.add_map_item("numbers", numbers_value);

    FT_ASSERT_EQ(ER_SUCCESS, root.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, numbers_value->get_error());

    ft_string yaml_string = yaml_write_to_string(&root);
    FT_ASSERT_EQ(ER_SUCCESS, yaml_string.get_error());

    yaml_value *parsed = yaml_read_from_string(yaml_string);
    FT_ASSERT(parsed != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, parsed->get_error());

    ft_string round_trip = yaml_write_to_string(parsed);
    FT_ASSERT_EQ(ER_SUCCESS, round_trip.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(yaml_string.c_str(), round_trip.c_str()));
    yaml_free(parsed);
    return (1);
}
