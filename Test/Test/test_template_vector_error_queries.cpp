#include "../test_internal.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

class vector_error_query_probe
{
    public:
        int32_t _value;

        vector_error_query_probe() noexcept
            : _value(0)
        {
            return ;
        }

        vector_error_query_probe(const vector_error_query_probe &other) noexcept = delete;
        vector_error_query_probe(vector_error_query_probe &&other) noexcept = delete;

        ~vector_error_query_probe() noexcept
        {
            return ;
        }

        void set_value(int32_t value) noexcept
        {
            this->_value = value;
            return ;
        }

        int32_t initialize(const vector_error_query_probe &other) noexcept
        {
            if (other._value < 0)
                return (FT_ERR_NO_MEMORY);
            this->_value = other._value;
            return (FT_ERR_SUCCESS);
        }
};

FT_TEST(test_template_vector_push_back_initialise_failure_sets_instance_error)
{
    ft_vector<vector_error_query_probe> vector_instance;
    vector_error_query_probe failing_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_instance.initialize());
    failing_value.set_value(-1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_instance.push_back(failing_value));
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_instance.get_error());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), vector_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_instance.destroy());
    return (1);
}

FT_TEST(test_template_vector_resize_initialise_failure_sets_instance_error)
{
    ft_vector<vector_error_query_probe> vector_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(1));
    vector_instance.resize(1024);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, vector_instance.get_error());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), vector_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vector_instance.destroy());
    return (1);
}
