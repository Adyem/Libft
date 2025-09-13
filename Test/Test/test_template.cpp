#include "../../Template/vector.hpp"
#include "../../Template/map.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/unique_ptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../JSon/document.hpp"
#include "../../CMA/CMA.hpp"
#include <cstring>
#include <cstdio>
#include <vector>

int test_ft_vector_push_back(void)
{
    ft_vector<int> v;
    v.push_back(1);
    v.push_back(2);
    return (v.size() == 2 && v[0] == 1 && v[1] == 2);
}

int test_ft_vector_insert_erase(void)
{
    ft_vector<int> v;
    v.push_back(1);
    v.push_back(3);
    v.insert(v.begin() + 1, 2);
    bool ok = (v.size() == 3 && v[1] == 2);
    v.erase(v.begin() + 1);
    return (ok && v.size() == 2 && v[1] == 3);
}

int test_ft_map_insert_find(void)
{
    ft_map<int, const char*> m;
    m.insert(1, "one");
    auto p = m.find(1);
    return (p && std::strcmp(p->value, "one") == 0);
}

int test_ft_map_remove(void)
{
    ft_map<int, int> m;
    m.insert(1, 10);
    m.insert(2, 20);
    m.remove(1);
    return (m.find(1) == ft_nullptr && m.size() == 1);
}

int test_ft_shared_ptr_basic(void)
{
    ft_sharedptr<int> sp(new int(42));
    return (!sp.hasError() && sp.use_count() == 1 && *sp == 42);
}

int test_ft_unique_ptr_basic(void)
{
    ft_uniqueptr<int> up(new int(7));
    return (!up.hasError() && *up == 7);
}


int test_ft_vector_reserve_resize(void)
{
    ft_vector<int> v;
    v.reserve(5);
    if (v.capacity() < 5)
        return (0);
    v.resize(3, 7);
    return (v.size() == 3 && v[0] == 7 && v[1] == 7 && v[2] == 7);
}

int test_ft_vector_clear(void)
{
    ft_vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.clear();
    return (v.size() == 0);
}
int test_ft_vector_vs_std_push_back(void)
{
    ft_vector<int> ft;
    std::vector<int> stdv;
    int index_i = 0;
    while (index_i < 100)
    {
        ft.push_back(index_i);
        stdv.push_back(index_i);
        index_i++;
    }
    if (ft.size() != stdv.size())
        return (0);
    size_t index_ft = 0;
    while (index_ft < ft.size())
    {
        if (ft[index_ft] != stdv[index_ft])
            return (0);
        index_ft++;
    }
    return (1);
}

int test_ft_vector_vs_std_insert_erase(void)
{
    ft_vector<int> ft;
    std::vector<int> stdv;
    int index_i = 0;
    while (index_i < 5)
    {
        ft.push_back(index_i);
        stdv.push_back(index_i);
        index_i++;
    }
    ft.insert(ft.begin() + 2, 42);
    stdv.insert(stdv.begin() + 2, 42);
    ft.erase(ft.begin() + 1);
    stdv.erase(stdv.begin() + 1);
    if (ft.size() != stdv.size())
        return (0);
    size_t index_ft = 0;
    while (index_ft < ft.size())
    {
        if (ft[index_ft] != stdv[index_ft])
            return (0);
        index_ft++;
    }
    return (1);
}

int test_ft_vector_vs_std_reserve_resize(void)
{
    ft_vector<int> ft;
    std::vector<int> stdv;
    ft.reserve(10);
    stdv.reserve(10);
    ft.resize(6, 3);
    stdv.resize(6, 3);
    if (ft.size() != stdv.size())
        return (0);
    size_t index_ft = 0;
    while (index_ft < ft.size())
    {
        if (ft[index_ft] != stdv[index_ft])
            return (0);
        index_ft++;
    }
    return (ft.capacity() >= 10 && stdv.capacity() >= 10);
}


int test_ft_map_at(void)
{
    ft_map<int, const char*> m;
    m.insert(1, "one");
    return (std::strcmp(m.at(1), "one") == 0);
}

int test_ft_map_at_missing(void)
{
    ft_map<int, const char*> m;
    m.insert(1, "one");
    m.at(2);
    return (m.get_error() == UNORD_MAP_UNKNOWN);
}

int test_ft_map_clear_empty(void)
{
    ft_map<int, int> m;
    m.insert(1, 10);
    m.insert(2, 20);
    m.clear();
    return (m.empty() && m.size() == 0);
}

int test_ft_shared_ptr_array(void)
{
    ft_sharedptr<int> sp(3);
    if (sp.hasError())
        return (0);
    sp[0] = 1;
    sp[1] = 2;
    sp[2] = 3;
    return (sp.use_count() == 1 && sp[1] == 2);
}

int test_ft_shared_ptr_add_remove(void)
{
    ft_sharedptr<int> sp((size_t)0);
    sp.add(1);
    sp.add(2);
    sp.remove(0);
    return (sp[0] == 2 && sp.use_count() == 1);
}

int test_ft_shared_ptr_reset(void)
{
    ft_sharedptr<int> sp(new int(5));
    ft_sharedptr<int> other = sp;
    sp.reset();
    return (sp.get() == ft_nullptr && other.use_count() == 1 && *other == 5);
}

int test_ft_unique_ptr_array(void)
{
    ft_uniqueptr<int> up(3);
    if (up.hasError())
        return (0);
    up[0] = 1;
    up[1] = 2;
    up[2] = 3;
    return (up[2] == 3);
}

int test_ft_unique_ptr_release(void)
{
    ft_uniqueptr<int> up(new int(42));
    int* raw = up.release();
    bool ok = (raw != ft_nullptr && *raw == 42 && !up);
    delete raw;
    return (ok);
}

int test_ft_unique_ptr_swap(void)
{
    ft_uniqueptr<int> a(new int(1));
    ft_uniqueptr<int> b(new int(2));
    a.swap(b);
    return (*a == 2 && *b == 1);
}

int test_json_roundtrip_string(void)
{
    json_document document_one;
    json_group *group = document_one.create_group("group");
    json_item *item = document_one.create_item("key", "value");
    document_one.add_item(group, item);
    document_one.append_group(group);
    char *output = document_one.write_to_string();
    if (!output)
        return (0);
    json_document document_two;
    if (document_two.read_from_string(output) != 0)
    {
        cma_free(output);
        return (0);
    }
    cma_free(output);
    json_group *group_two = document_two.find_group("group");
    if (!group_two)
        return (0);
    json_item *item_two = document_two.find_item(group_two, "key");
    if (!item_two)
        return (0);
    return (std::strcmp(item_two->value, "value") == 0);
}

int test_json_roundtrip_file(void)
{
    const char *file_path = "Test_roundtrip.json";
    json_document document_one;
    json_group *group = document_one.create_group("group");
    json_item *item = document_one.create_item("key", "value");
    document_one.add_item(group, item);
    document_one.append_group(group);
    if (document_one.write_to_file(file_path) != 0)
        return (0);
    json_document document_two;
    if (document_two.read_from_file(file_path) != 0)
        return (0);
    std::remove(file_path);
    json_group *group_two = document_two.find_group("group");
    if (!group_two)
        return (0);
    json_item *item_two = document_two.find_item(group_two, "key");
    if (!item_two)
        return (0);
    return (std::strcmp(item_two->value, "value") == 0);
}


