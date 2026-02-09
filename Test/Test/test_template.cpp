#include "../../Template/vector.hpp"
#include "../../Template/map.hpp"
#include "../../Template/set.hpp"
#include "../../Template/stack.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/unique_ptr.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../JSon/document.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstring>
#include <cstdio>
#include <vector>

class vector_destructor_tracker
{
    private:
        static int _live_count;
        int _value;
        mutable int _error_code;

        void set_error(int error_code) const;

    public:
        vector_destructor_tracker();
        explicit vector_destructor_tracker(int value);
        vector_destructor_tracker(const vector_destructor_tracker &other);
        vector_destructor_tracker(vector_destructor_tracker &&other) noexcept;
        vector_destructor_tracker &operator=(const vector_destructor_tracker &other);
        vector_destructor_tracker &operator=(vector_destructor_tracker &&other) noexcept;
        ~vector_destructor_tracker();

        static void reset();
        static int live_count();
        int get_error() const;
        const char *get_error_str() const;
};

int vector_destructor_tracker::_live_count = 0;

vector_destructor_tracker::vector_destructor_tracker()
    : _value(0), _error_code(FT_ERR_SUCCESSS)
{
    ++_live_count;
    return ;
}

vector_destructor_tracker::vector_destructor_tracker(int value)
    : _value(value), _error_code(FT_ERR_SUCCESSS)
{
    ++_live_count;
    return ;
}

vector_destructor_tracker::vector_destructor_tracker(const vector_destructor_tracker &other)
    : _value(other._value), _error_code(FT_ERR_SUCCESSS)
{
    ++_live_count;
    return ;
}

vector_destructor_tracker::vector_destructor_tracker(vector_destructor_tracker &&other) noexcept
    : _value(other._value), _error_code(FT_ERR_SUCCESSS)
{
    ++_live_count;
    return ;
}

vector_destructor_tracker &vector_destructor_tracker::operator=(const vector_destructor_tracker &other)
{
    if (this != &other)
        this->_value = other._value;
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

vector_destructor_tracker &vector_destructor_tracker::operator=(vector_destructor_tracker &&other) noexcept
{
    if (this != &other)
        this->_value = other._value;
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

vector_destructor_tracker::~vector_destructor_tracker()
{
    --_live_count;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void vector_destructor_tracker::reset()
{
    _live_count = 0;
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

int vector_destructor_tracker::live_count()
{
    return (_live_count);
}

void vector_destructor_tracker::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

int vector_destructor_tracker::get_error() const
{
    return (this->_error_code);
}

const char *vector_destructor_tracker::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

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
    return (p != m.end() && std::strcmp(p->value, "one") == 0);
}

int test_ft_map_remove(void)
{
    ft_map<int, int> m;
    m.insert(1, 10);
    m.insert(2, 20);
    m.remove(1);
    return (m.find(1) == m.end() && m.size() == 1);
}

int test_ft_shared_ptr_basic(void)
{
    ft_sharedptr<int> sp(new int(42));
    bool operations_ok = (sp.use_count() == 1 && *sp == 42);

    return (operations_ok && ft_global_error_stack_peek_last_error() == FT_ERR_SUCCESSS);
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

FT_TEST(test_ft_vector_erase_releases_resources, "ft_vector erase destroys overwritten elements")
{
    vector_destructor_tracker::reset();

    {
        ft_vector<vector_destructor_tracker> vector_instance;

        vector_instance.push_back(vector_destructor_tracker(1));
        vector_instance.push_back(vector_destructor_tracker(2));
        vector_instance.push_back(vector_destructor_tracker(3));
        FT_ASSERT_EQ(static_cast<size_t>(3), vector_instance.size());
        vector_instance.erase(vector_instance.begin());
        FT_ASSERT_EQ(static_cast<size_t>(2), vector_instance.size());
    }

    FT_ASSERT_EQ(0, vector_destructor_tracker::live_count());
    return (1);
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
    return (m.get_error() == FT_ERR_INTERNAL);
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
    if (ft_global_error_stack_peek_last_error() != FT_ERR_SUCCESSS)
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

FT_TEST(test_ft_vector_resets_errno_after_successful_push, "ft_vector clears errno after successful push_back")
{
    ft_vector<int> vector_instance;

    ft_errno = FT_ERR_SUCCESSS;
    vector_instance[0];
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, vector_instance.get_error());
    vector_instance.push_back(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, vector_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(5, vector_instance[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, vector_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_set_resets_errno_after_successful_insert, "ft_set clears errno after successful insert")
{
    ft_set<int> set_instance;

    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(ft_nullptr, set_instance.find(42));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, set_instance.get_error());
    set_instance.insert(42);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, set_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    int *found = set_instance.find(42);
    FT_ASSERT(found != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, set_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_stack_resets_errno_after_successful_push, "ft_stack clears errno after successful push")
{
    ft_stack<int> stack_instance;

    ft_errno = FT_ERR_SUCCESSS;
    stack_instance.pop();
    FT_ASSERT_EQ(FT_ERR_EMPTY, stack_instance.get_error());
    stack_instance.push(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, stack_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(7, stack_instance.top());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, stack_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_map_grows_from_zero_capacity, "ft_map grows when constructed with zero capacity")
{
    ft_map<int, int> map_instance(0);

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.get_error());
    map_instance.insert(42, 7);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.get_error());
    FT_ASSERT_EQ(static_cast<size_t>(1), map_instance.size());
    Pair<int, int> *found_entry = map_instance.find(42);
    FT_ASSERT(found_entry != map_instance.end());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, map_instance.get_error());
    FT_ASSERT_EQ(7, found_entry->value);
    return (1);
}
