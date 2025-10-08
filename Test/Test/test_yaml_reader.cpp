#include "../../YAML/yaml.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/system_utils.hpp"
#include <cstdio>

FT_TEST(test_yaml_reader_malformed_structure_sets_errno, "yaml_reader reports malformed map indentation")
{
    ft_string malformed = "root:\nvalue";

    ft_errno = ER_SUCCESS;
    yaml_value *result = yaml_read_from_string(malformed);
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_yaml_reader_list_with_premature_eof_sets_errno, "yaml_reader detects premature eof in list item")
{
    ft_string premature = "-\n";

    ft_errno = ER_SUCCESS;
    yaml_value *result = yaml_read_from_string(premature);
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_yaml_reader_allocation_failure_sets_errno, "yaml_reader propagates allocation failures")
{
    ft_string simple = "root: value";
    yaml_value *result;

    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(200);
    result = yaml_read_from_string(simple);
    cma_set_alloc_limit(0);
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_yaml_reader_scalar_set_scalar_failure, "yaml_reader handles scalar assignment failures")
{
    ft_string content(32, 'A');
    yaml_value *result;

    FT_ASSERT_EQ(ER_SUCCESS, content.get_error());
    cma_set_alloc_limit(32);
    ft_errno = ER_SUCCESS;
    result = yaml_read_from_string(content);
    cma_set_alloc_limit(0);
    if (result != ft_nullptr)
    {
        yaml_free(result);
    }
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_yaml_reader_list_inline_map_entries, "yaml_reader parses inline maps inside lists")
{
    ft_string content = "- name: foo\n  value: 42\n- name: bar\n  value: 84\n";
    yaml_value *root;
    ft_errno = ER_SUCCESS;
    root = yaml_read_from_string(content);
    FT_ASSERT(root != ft_nullptr);
    FT_ASSERT_EQ(YAML_LIST, root->get_type());
    const ft_vector<yaml_value*> &items = root->get_list();
    FT_ASSERT_EQ(ER_SUCCESS, items.get_error());
    FT_ASSERT_EQ(static_cast<size_t>(2), items.size());

    yaml_value *first = items[0];
    yaml_value *second = items[1];

    FT_ASSERT(first != ft_nullptr);
    FT_ASSERT(second != ft_nullptr);
    FT_ASSERT_EQ(YAML_MAP, first->get_type());
    FT_ASSERT_EQ(YAML_MAP, second->get_type());

    const ft_map<ft_string, yaml_value*> &first_map = first->get_map();
    const ft_map<ft_string, yaml_value*> &second_map = second->get_map();

    yaml_value *first_name = first_map.at("name");
    yaml_value *first_value = first_map.at("value");
    yaml_value *second_name = second_map.at("name");
    yaml_value *second_value = second_map.at("value");

    FT_ASSERT(first_name != ft_nullptr);
    FT_ASSERT(first_value != ft_nullptr);
    FT_ASSERT(second_name != ft_nullptr);
    FT_ASSERT(second_value != ft_nullptr);
    FT_ASSERT_EQ(YAML_SCALAR, first_name->get_type());
    FT_ASSERT_EQ(YAML_SCALAR, first_value->get_type());
    FT_ASSERT_EQ(YAML_SCALAR, second_name->get_type());
    FT_ASSERT_EQ(YAML_SCALAR, second_value->get_type());
    FT_ASSERT_EQ(ft_string("foo"), first_name->get_scalar());
    FT_ASSERT_EQ(ft_string("42"), first_value->get_scalar());
    FT_ASSERT_EQ(ft_string("bar"), second_name->get_scalar());
    FT_ASSERT_EQ(ft_string("84"), second_value->get_scalar());

    yaml_free(root);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_yaml_read_from_file_propagates_read_error, "yaml_read_from_file reports su_fread failures")
{
    const char *file_path = "yaml_reader_forced_failure.yaml";
    int file_descriptor;
    yaml_value *result;

    file_descriptor = su_open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    FT_ASSERT(file_descriptor >= 0);
    FT_ASSERT_EQ(0, su_close(file_descriptor));
    su_force_fread_failure(FT_ERR_IO);
    ft_errno = ER_SUCCESS;
    result = yaml_read_from_file(file_path);
    su_clear_forced_fread_failure();
    std::remove(file_path);
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    return (1);
}
