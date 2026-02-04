#include "../../CPP_class/class_istream.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../CPP_class/class_stringbuf.hpp"
#include "../../Errno/errno.hpp"
#include "../../PThread/recursive_mutex.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstddef>

class ft_test_simple_istream : public ft_istream
{
    private:
        char _fill_character;

    public:
        ft_test_simple_istream(char fill_character) noexcept
            : ft_istream()
            , _fill_character(fill_character)
        {
            return ;
        }

    protected:
        std::size_t do_read(char *buffer, std::size_t count) override
        {
            if (buffer == ft_nullptr || count == 0)
            {
                ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
                return (0);
            }
            buffer[0] = this->_fill_character;
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (1);
        }
};

FT_TEST(test_ft_istream_read_unlocks_mutex,
        "ft_istream read unlocks its mutex")
{
    ft_test_simple_istream stream('r');
    char buffer[2];
    pt_recursive_mutex *mutex;

    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_SINGLE_ERROR_READ(stream, buffer, 1);
    mutex = stream.get_mutex_for_validation();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ('r', buffer[0]);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_istream_bad_unlocks_mutex,
        "ft_istream bad unlocks its mutex")
{
    ft_test_simple_istream stream('b');
    bool bad_result;
    pt_recursive_mutex *mutex;

    FT_ASSERT_SINGLE_ERROR_BAD(bad_result, stream);
    mutex = stream.get_mutex_for_validation();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ(false, bad_result);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_istream_gcount_unlocks_mutex,
        "ft_istream gcount unlocks its mutex")
{
    ft_test_simple_istream stream('g');
    char buffer[2];
    std::size_t count_value;
    pt_recursive_mutex *mutex;

    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_SINGLE_ERROR_READ(stream, buffer, 1);
    FT_ASSERT_SINGLE_ERROR_GCOUNT(count_value, stream);
    mutex = stream.get_mutex_for_validation();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ(1, count_value);
    FT_ASSERT_EQ('g', buffer[0]);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_read_unlocks_mutex,
        "ft_stringbuf read unlocks its mutex")
{
    ft_string source("mutex");
    ft_stringbuf buffer(source);
    char storage[2];
    pt_recursive_mutex *mutex;

    storage[0] = '\0';
    storage[1] = '\0';
    FT_ASSERT_SINGLE_ERROR_READ(buffer, storage, 1);
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ('m', storage[0]);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_is_valid_unlocks_mutex,
        "ft_stringbuf is_valid unlocks its mutex")
{
    ft_string source("mutex");
    ft_stringbuf buffer(source);
    bool valid_result;
    pt_recursive_mutex *mutex;

    FT_ASSERT_SINGLE_GLOBAL_ERROR(valid_result = buffer.is_valid());
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ(true, valid_result);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_str_unlocks_mutex,
        "ft_stringbuf str unlocks its mutex")
{
    ft_string source("mutex");
    ft_stringbuf buffer(source);
    ft_string snapshot;
    pt_recursive_mutex *mutex;

    FT_ASSERT_SINGLE_ERROR_STR(snapshot, buffer);
    FT_ASSERT_EQ(true, snapshot.size() > 0);
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_istream_read_invalid_argument_unlocks_mutex,
        "ft_istream read invalid argument unlocks its mutex")
{
    ft_test_simple_istream stream('x');
    pt_recursive_mutex *mutex;

    FT_ASSERT_SINGLE_GLOBAL_ERROR(stream.read(ft_nullptr, 1));
    int read_error = ft_global_error_stack_last_error();
    mutex = stream.get_mutex_for_validation();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, read_error);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_read_invalid_argument_unlocks_mutex,
        "ft_stringbuf read invalid argument unlocks its mutex")
{
    ft_string source("mutex");
    ft_stringbuf buffer(source);
    pt_recursive_mutex *mutex;

    FT_ASSERT_SINGLE_GLOBAL_ERROR(buffer.read(ft_nullptr, 1));
    int read_error = ft_global_error_stack_last_error();
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, read_error);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_istream_gcount_after_error_unlocks_mutex,
        "ft_istream gcount unlocks its mutex after an error")
{
    ft_test_simple_istream stream('x');
    std::size_t count_value;
    pt_recursive_mutex *mutex;

    FT_ASSERT_SINGLE_GLOBAL_ERROR(stream.read(ft_nullptr, 1));
    FT_ASSERT_SINGLE_ERROR_GCOUNT(count_value, stream);
    mutex = stream.get_mutex_for_validation();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ(0, count_value);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_global_error_stack_last_error());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_is_valid_after_error_unlocks_mutex,
        "ft_stringbuf is_valid unlocks its mutex after an error")
{
    ft_string source("mutex");
    ft_stringbuf buffer(source);
    pt_recursive_mutex *mutex;
    bool valid_result;

    FT_ASSERT_SINGLE_GLOBAL_ERROR(buffer.read(ft_nullptr, 1));
    FT_ASSERT_SINGLE_GLOBAL_ERROR(valid_result = buffer.is_valid());
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ(false, valid_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_global_error_stack_last_error());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_str_after_error_unlocks_mutex,
        "ft_stringbuf str unlocks its mutex even after prior errors")
{
    ft_string source("mutex");
    ft_stringbuf buffer(source);
    ft_string snapshot;
    pt_recursive_mutex *mutex;

    FT_ASSERT_SINGLE_GLOBAL_ERROR(buffer.read(ft_nullptr, 1));
    FT_ASSERT_SINGLE_ERROR_STR(snapshot, buffer);
    FT_ASSERT_EQ(true, snapshot.size() > 0);
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_istream_read_twice_unlocks_mutex,
        "ft_istream read twice unlocks its mutex")
{
    ft_test_simple_istream stream('t');
    char buffer[2];
    pt_recursive_mutex *mutex;

    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_SINGLE_ERROR_READ(stream, buffer, 1);
    FT_ASSERT_SINGLE_ERROR_READ(stream, buffer, 1);
    mutex = stream.get_mutex_for_validation();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ('t', buffer[0]);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_istream_bad_after_read_unlocks_mutex,
        "ft_istream bad after read unlocks its mutex")
{
    ft_test_simple_istream stream('b');
    char buffer[2];
    bool bad_result;
    pt_recursive_mutex *mutex;

    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_SINGLE_ERROR_READ(stream, buffer, 1);
    FT_ASSERT_SINGLE_ERROR_BAD(bad_result, stream);
    mutex = stream.get_mutex_for_validation();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ(false, bad_result);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_istream_gcount_after_success_unlocks_mutex,
        "ft_istream gcount after read unlocks its mutex")
{
    ft_test_simple_istream stream('g');
    char buffer[2];
    std::size_t count_value;
    pt_recursive_mutex *mutex;

    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_SINGLE_ERROR_READ(stream, buffer, 1);
    FT_ASSERT_SINGLE_ERROR_GCOUNT(count_value, stream);
    mutex = stream.get_mutex_for_validation();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ(1, count_value);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_istream_read_zero_count_unlocks_mutex,
        "ft_istream read with zero count unlocks its mutex")
{
    ft_test_simple_istream stream('z');
    char buffer[1];
    std::size_t count_value;
    pt_recursive_mutex *mutex;

    buffer[0] = '\0';
    FT_ASSERT_SINGLE_ERROR_READ(stream, buffer, 0);
    FT_ASSERT_SINGLE_ERROR_GCOUNT(count_value, stream);
    mutex = stream.get_mutex_for_validation();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ(0, count_value);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_read_twice_unlocks_mutex,
        "ft_stringbuf read twice unlocks its mutex")
{
    ft_string source("twice");
    ft_stringbuf buffer(source);
    char storage[2];
    pt_recursive_mutex *mutex;

    storage[0] = '\0';
    storage[1] = '\0';
    FT_ASSERT_SINGLE_ERROR_READ(buffer, storage, 1);
    FT_ASSERT_SINGLE_ERROR_READ(buffer, storage, 1);
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ('t', storage[0]);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_read_zero_count_unlocks_mutex,
        "ft_stringbuf read with zero count unlocks its mutex")
{
    ft_string source("mutex");
    ft_stringbuf buffer(source);
    char storage[1];
    pt_recursive_mutex *mutex;

    storage[0] = '\0';
    FT_ASSERT_SINGLE_ERROR_READ(buffer, storage, 0);
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ('\0', storage[0]);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_is_valid_after_read_unlocks_mutex,
        "ft_stringbuf is_valid after read unlocks its mutex")
{
    ft_string source("mutex");
    ft_stringbuf buffer(source);
    char storage[2];
    bool bad_result;
    pt_recursive_mutex *mutex;

    storage[0] = '\0';
    storage[1] = '\0';
    FT_ASSERT_SINGLE_ERROR_READ(buffer, storage, 1);
    FT_ASSERT_SINGLE_ERROR_BAD(bad_result, buffer);
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    FT_ASSERT_EQ(false, bad_result);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_str_multiple_unlocks_mutex,
        "ft_stringbuf str twice unlocks its mutex")
{
    ft_string source("mutex");
    ft_stringbuf buffer(source);
    ft_string first_snapshot;
    ft_string second_snapshot;
    pt_recursive_mutex *mutex;

    FT_ASSERT_SINGLE_ERROR_STR(first_snapshot, buffer);
    FT_ASSERT_SINGLE_ERROR_STR(second_snapshot, buffer);
    FT_ASSERT_EQ(first_snapshot.size(), second_snapshot.size());
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_str_after_read_unlocks_mutex,
        "ft_stringbuf str after read unlocks mutex")
{
    ft_string source("mutex");
    ft_stringbuf buffer(source);
    char storage[2];
    ft_string snapshot;
    pt_recursive_mutex *mutex;

    storage[0] = '\0';
    storage[1] = '\0';
    FT_ASSERT_SINGLE_ERROR_READ(buffer, storage, 1);
    FT_ASSERT_SINGLE_ERROR_STR(snapshot, buffer);
    FT_ASSERT_EQ(true, snapshot.size() > 0);
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_stringbuf_bad_after_str_unlocks_mutex,
        "ft_stringbuf bad after str unlocks its mutex")
{
    ft_string source("mutex");
    ft_stringbuf buffer(source);
    ft_string snapshot;
    bool bad_result;
    pt_recursive_mutex *mutex;

    FT_ASSERT_SINGLE_ERROR_STR(snapshot, buffer);
    FT_ASSERT_SINGLE_ERROR_BAD(bad_result, buffer);
    FT_ASSERT_EQ(false, bad_result);
    mutex = buffer.get_mutex_for_testing();
    FT_ASSERT(mutex != ft_nullptr);
    FT_ASSERT_EQ(false, mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}
