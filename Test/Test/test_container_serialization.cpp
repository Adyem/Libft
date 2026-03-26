#include "../test_internal.hpp"
#include "../../Template/container_serialization.hpp"
#include "../../Template/vector.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../JSon/json.hpp"
#include "../../YAML/yaml.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_container_serialization_json_round_trip_strings)
{
    ft_vector<ft_string> input_values;
    ft_vector<ft_string> output_values;
    ft_string first_value;
    ft_string second_value;
    json_group *group;

    group = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, input_values.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, output_values.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_value.initialize("alpha"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_value.initialize("beta"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, input_values.push_back(first_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, input_values.push_back(second_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_vector_serialize_json(input_values, "values", group));
    FT_ASSERT(group != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_vector_deserialize_json(group, output_values));
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), output_values.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, output_values[0].get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, output_values[1].get_error());
    FT_ASSERT_EQ(0, ft_strcmp(output_values[0].c_str(), "alpha"));
    FT_ASSERT_EQ(0, ft_strcmp(output_values[1].c_str(), "beta"));
    json_free_groups(group);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, output_values.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, input_values.destroy());
    return (1);
}

FT_TEST(test_container_serialization_yaml_round_trip_ints)
{
    ft_vector<int32_t> input_values;
    ft_vector<int32_t> output_values;
    yaml_value *serialized_value;

    serialized_value = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, input_values.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, output_values.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, input_values.push_back(10));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, input_values.push_back(20));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_vector_serialize_yaml(input_values, serialized_value));
    FT_ASSERT(serialized_value != ft_nullptr);
    FT_ASSERT_EQ(YAML_LIST, serialized_value->get_type());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_vector_deserialize_yaml(*serialized_value, output_values));
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), output_values.size());
    FT_ASSERT_EQ(10, output_values[0]);
    FT_ASSERT_EQ(20, output_values[1]);
    yaml_free(serialized_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, output_values.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, input_values.destroy());
    return (1);
}
