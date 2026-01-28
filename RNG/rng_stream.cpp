#include "rng_stream.hpp"
#include "rng.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Math/math.hpp"
#include "../PThread/pthread.hpp"
#include <climits>
#include <limits>

static int rng_stream_capture_math_error(ft_size_t previous_depth)
{
    ft_size_t current_depth = ft_global_error_stack_depth();

    if (current_depth <= previous_depth)
        return (FT_ERR_SUCCESSS);
    int error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
        ft_global_error_stack_push(error_code);
    return (error_code);
}

void rng_stream::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
}

int rng_stream::lock_internal(bool *lock_acquired) const
{
    int mutex_result;
    int global_error;
    int operation_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    mutex_result = this->_mutex.lock(THREAD_ID);
    global_error = ft_global_error_stack_pop_newest();
    operation_error = global_error;
    if (global_error == FT_ERR_SUCCESSS)
        operation_error = mutex_result;
    else
        ft_global_error_stack_push(global_error);
    if (operation_error == FT_ERR_SUCCESSS && lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (operation_error);
}

int rng_stream::unlock_internal(bool lock_acquired) const
{
    int mutex_result;
    int global_error;
    int operation_error;

    if (!lock_acquired)
        return (FT_ERR_SUCCESSS);
    mutex_result = this->_mutex.unlock(THREAD_ID);
    global_error = ft_global_error_stack_pop_newest();
    operation_error = global_error;
    if (global_error == FT_ERR_SUCCESSS)
        operation_error = mutex_result;
    else
        ft_global_error_stack_push(global_error);
    return (operation_error);
}

pt_recursive_mutex *rng_stream::mutex_handle()
{
    return (&this->_mutex);
}

const pt_recursive_mutex *rng_stream::mutex_handle() const
{
    return (&this->_mutex);
}

ft_operation_error_stack *rng_stream::operation_error_stack_handle()
{
    return (&this->_operation_errors);
}

const ft_operation_error_stack *rng_stream::operation_error_stack_handle() const
{
    return (&this->_operation_errors);
}

rng_stream::rng_stream()
{
    std::random_device random_device;

    this->_engine.seed(random_device());
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

rng_stream::rng_stream(uint32_t seed_value)
{
    this->_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

rng_stream::rng_stream(const rng_stream &other)
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    lock_acquired = false;
    lock_error = other.lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    this->_engine = other._engine;
    unlock_error = other.unlock_internal(lock_acquired);
    this->record_operation_error(unlock_error);
    return ;
}

rng_stream &rng_stream::operator=(const rng_stream &other)
{
    if (this == &other)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (*this);
    }
    bool this_lock_acquired;
    bool other_lock_acquired;
    int lock_error;
    int unlock_error;

    this_lock_acquired = false;
    other_lock_acquired = false;
    if (this < &other)
    {
        lock_error = this->lock_internal(&this_lock_acquired);
        if (lock_error == FT_ERR_SUCCESSS)
            lock_error = other.lock_internal(&other_lock_acquired);
    }
    else
    {
        lock_error = other.lock_internal(&other_lock_acquired);
        if (lock_error == FT_ERR_SUCCESSS)
            lock_error = this->lock_internal(&this_lock_acquired);
    }
    if (lock_error != FT_ERR_SUCCESSS)
    {
        if (other_lock_acquired)
            other.unlock_internal(other_lock_acquired);
        if (this_lock_acquired)
            this->unlock_internal(this_lock_acquired);
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_engine = other._engine;
    unlock_error = FT_ERR_SUCCESSS;
    if (this < &other)
    {
        int other_unlock_error = other.unlock_internal(other_lock_acquired);
        int this_unlock_error = this->unlock_internal(this_lock_acquired);
        if (other_unlock_error != FT_ERR_SUCCESSS)
            unlock_error = other_unlock_error;
        else
            unlock_error = this_unlock_error;
    }
    else
    {
        int this_unlock_error = this->unlock_internal(this_lock_acquired);
        int other_unlock_error = other.unlock_internal(other_lock_acquired);
        if (this_unlock_error != FT_ERR_SUCCESSS)
            unlock_error = this_unlock_error;
        else
            unlock_error = other_unlock_error;
    }
    this->record_operation_error(unlock_error);
    return (*this);
}

rng_stream::rng_stream(rng_stream &&other) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    lock_acquired = false;
    lock_error = other.lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    this->_engine = other._engine;
    std::mt19937 default_engine;

    other._engine = default_engine;
    unlock_error = other.unlock_internal(lock_acquired);
    this->record_operation_error(unlock_error);
    return ;
}

rng_stream &rng_stream::operator=(rng_stream &&other) noexcept
{
    if (this == &other)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (*this);
    }
    bool this_lock_acquired;
    bool other_lock_acquired;
    int lock_error;
    int unlock_error;

    this_lock_acquired = false;
    other_lock_acquired = false;
    if (this < &other)
    {
        lock_error = this->lock_internal(&this_lock_acquired);
        if (lock_error == FT_ERR_SUCCESSS)
            lock_error = other.lock_internal(&other_lock_acquired);
    }
    else
    {
        lock_error = other.lock_internal(&other_lock_acquired);
        if (lock_error == FT_ERR_SUCCESSS)
            lock_error = this->lock_internal(&this_lock_acquired);
    }
    if (lock_error != FT_ERR_SUCCESSS)
    {
        if (other_lock_acquired)
            other.unlock_internal(other_lock_acquired);
        if (this_lock_acquired)
            this->unlock_internal(this_lock_acquired);
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_engine = other._engine;
    std::mt19937 default_engine;

    other._engine = default_engine;
    unlock_error = FT_ERR_SUCCESSS;
    if (this < &other)
    {
        int other_unlock_error = other.unlock_internal(other_lock_acquired);
        int this_unlock_error = this->unlock_internal(this_lock_acquired);
        if (other_unlock_error != FT_ERR_SUCCESSS)
            unlock_error = other_unlock_error;
        else
            unlock_error = this_unlock_error;
    }
    else
    {
        int this_unlock_error = this->unlock_internal(this_lock_acquired);
        int other_unlock_error = other.unlock_internal(other_lock_acquired);
        if (this_unlock_error != FT_ERR_SUCCESSS)
            unlock_error = this_unlock_error;
        else
            unlock_error = other_unlock_error;
    }
    this->record_operation_error(unlock_error);
    return (*this);
}

rng_stream::~rng_stream()
{
    return ;
}

void    rng_stream::reseed(uint32_t seed_value)
{
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        this->_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    this->record_operation_error(operation_error);
    return ;
}

void    rng_stream::reseed_from_string(const char *seed_string)
{
    if (seed_string == ft_nullptr)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    uint32_t derived_seed;

    derived_seed = ft_random_seed(seed_string);
    int error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(error_code);
        return ;
    }
    this->reseed(derived_seed);
    return ;
}

int rng_stream::random_int_unlocked()
{
    std::uniform_int_distribution<int> distribution(0, std::numeric_limits<int>::max());
    int random_value;

    random_value = distribution(this->_engine);
    return (random_value);
}

float   rng_stream::random_float_unlocked()
{
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    float random_value;

    random_value = distribution(this->_engine);
    return (random_value);
}

int rng_stream::random_int()
{
    int value;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    value = 0;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        value = this->random_int_unlocked();
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    this->record_operation_error(operation_error);
    return (value);
}

int rng_stream::dice_roll(int number, int faces)
{
    if (faces == 0 && number == 0)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (0);
    }
    if (faces < 1 || number < 1)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (faces == 1)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (number);
    }
    int result;
    int index;
    bool overflowed;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    result = 0;
    index = 0;
    overflowed = false;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
    {
        while (index < number)
        {
            int roll;

            roll = this->random_int_unlocked();
            if (result > INT_MAX - ((roll % faces) + 1))
            {
                overflowed = true;
                operation_error = FT_ERR_OUT_OF_RANGE;
                break ;
            }
            result = result + ((roll % faces) + 1);
            index = index + 1;
        }
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    if (overflowed)
    {
        this->record_operation_error(operation_error);
        return (-1);
    }
    if (operation_error == FT_ERR_SUCCESSS)
        this->record_operation_error(FT_ERR_SUCCESSS);
    else
        this->record_operation_error(operation_error);
    return (result);
}

float   rng_stream::random_float()
{
    float value;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    value = 0.0f;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        value = this->random_float_unlocked();
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    this->record_operation_error(operation_error);
    return (value);
}

float   rng_stream::random_normal()
{
    float uniform_one;
    float uniform_two;
    float radius;
    float angle;
    float result;
    const float pi_value = 3.14159265358979323846f;

    bool lock_acquired;
    int operation_error;
    int unlock_error;

    uniform_one = 0.0f;
    uniform_two = 0.0f;
    radius = 0.0f;
    angle = 0.0f;
    result = 0.0f;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
    {
        uniform_one = this->random_float_unlocked();
        if (uniform_one < 0.0000000001f)
            uniform_one = 0.0000000001f;
        uniform_two = this->random_float_unlocked();
    }
    if (operation_error == FT_ERR_SUCCESSS)
    {
        ft_size_t log_depth = ft_global_error_stack_depth();
        radius = static_cast<float>(math_sqrt(-2.0 * math_log(uniform_one)));
        int math_error = rng_stream_capture_math_error(log_depth);
        if (math_error != FT_ERR_SUCCESSS)
            operation_error = math_error;
    }
    if (operation_error == FT_ERR_SUCCESSS)
    {
        angle = 2.0f * pi_value * uniform_two;
        ft_size_t cos_depth = ft_global_error_stack_depth();
        result = radius * static_cast<float>(math_cos(angle));
        int math_error = rng_stream_capture_math_error(cos_depth);
        if (math_error != FT_ERR_SUCCESSS)
            operation_error = math_error;
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    this->record_operation_error(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (0.0f);
    return (result);
}

float   rng_stream::random_exponential(float lambda_value)
{
    float uniform_value;
    float result;

    if (lambda_value <= 0.0f)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (0.0f);
    }
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    uniform_value = 0.0f;
    result = 0.0f;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        uniform_value = this->random_float_unlocked();
    if (operation_error == FT_ERR_SUCCESSS)
    {
        if (uniform_value < 0.0000000001f)
            uniform_value = 0.0000000001f;
        ft_size_t log_depth = ft_global_error_stack_depth();
        result = static_cast<float>(-math_log(uniform_value)) / lambda_value;
        int math_error = rng_stream_capture_math_error(log_depth);
        if (math_error != FT_ERR_SUCCESSS)
            operation_error = math_error;
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    this->record_operation_error(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (0.0f);
    return (result);
}

int rng_stream::random_poisson(double lambda_value)
{
    double limit_value;
    double product_value;
    int count_value;

    if (lambda_value <= 0.0)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    ft_size_t exp_depth = ft_global_error_stack_depth();
    limit_value = math_exp(-lambda_value);
    int math_error = rng_stream_capture_math_error(exp_depth);
    if (math_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(math_error);
        return (0);
    }
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    product_value = 1.0;
    count_value = 0;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
    {
        while (product_value > limit_value)
        {
            double random_value;

            random_value = static_cast<double>(this->random_float_unlocked());
            product_value = product_value * random_value;
            count_value = count_value + 1;
        }
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    this->record_operation_error(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (0);
    return (count_value - 1);
}

int rng_stream::random_binomial(int trial_count, double success_probability)
{
    if (trial_count < 0)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (success_probability < 0.0)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (trial_count == 0)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (0);
    }
    if (success_probability > 1.0)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    const double probability_epsilon = std::numeric_limits<double>::epsilon();
    if (success_probability <= probability_epsilon)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (0);
    }
    if ((1.0 - success_probability) <= probability_epsilon)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (trial_count);
    }
    int trial_index;
    int success_count;

    trial_index = 0;
    success_count = 0;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
    {
        while (trial_index < trial_count)
        {
            double random_value;

            random_value = static_cast<double>(this->random_float_unlocked());
            if (random_value < success_probability)
                success_count = success_count + 1;
            trial_index = trial_index + 1;
        }
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    this->record_operation_error(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (0);
    return (success_count);
}

int rng_stream::random_geometric(double success_probability)
{
    if (success_probability <= 0.0)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (success_probability > 1.0)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    const double probability_epsilon = std::numeric_limits<double>::epsilon();
    if ((1.0 - success_probability) <= probability_epsilon)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (1);
    }
    int trial_count;
    int result;
    int found_success;

    trial_count = 1;
    found_success = 0;
    result = 0;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
    {
        while (1)
        {
            double random_value;

            random_value = static_cast<double>(this->random_float_unlocked());
            if (random_value < success_probability)
            {
                result = trial_count;
                found_success = 1;
                break ;
            }
            trial_count = trial_count + 1;
        }
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    if (found_success == 0 && operation_error == FT_ERR_SUCCESSS)
        operation_error = FT_ERR_INVALID_STATE;
    this->record_operation_error(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (0);
    return (result);
}

float   rng_stream::random_gamma(float shape, float scale)
{
    if (shape <= 0.0f || scale <= 0.0f)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (0.0f);
    }
    std::gamma_distribution<float> distribution(shape, scale);
    float sample_value;

    bool lock_acquired;
    int operation_error;
    int unlock_error;

    sample_value = 0.0f;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        sample_value = distribution(this->_engine);
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    this->record_operation_error(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (0.0f);
    return (sample_value);
}

float   rng_stream::random_beta(float alpha, float beta)
{
    if (alpha <= 0.0f || beta <= 0.0f)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (0.0f);
    }
    std::gamma_distribution<float> alpha_distribution(alpha, 1.0f);
    std::gamma_distribution<float> beta_distribution(beta, 1.0f);
    float alpha_sample;
    float beta_sample;
    float sum;
    float result;

    bool lock_acquired;
    int operation_error;
    int unlock_error;

    alpha_sample = 0.0f;
    beta_sample = 0.0f;
    result = 0.0f;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
    {
        alpha_sample = alpha_distribution(this->_engine);
        beta_sample = beta_distribution(this->_engine);
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    sum = alpha_sample + beta_sample;
    if (operation_error == FT_ERR_SUCCESSS)
    {
        if (sum <= 0.0f)
            operation_error = FT_ERR_INVALID_STATE;
    }
    if (operation_error == FT_ERR_SUCCESSS)
    {
        result = alpha_sample / sum;
        if (result < 0.0f)
            result = 0.0f;
        if (result > 1.0f)
            result = 1.0f;
    }
    this->record_operation_error(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (0.0f);
    return (result);
}

float   rng_stream::random_chi_squared(float degrees_of_freedom)
{
    if (degrees_of_freedom <= 0.0f)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (0.0f);
    }
    std::gamma_distribution<float> distribution(degrees_of_freedom * 0.5f, 2.0f);
    float sample_value;

    bool lock_acquired;
    int operation_error;
    int unlock_error;

    sample_value = 0.0f;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        sample_value = distribution(this->_engine);
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESSS)
        operation_error = unlock_error;
    if (operation_error == FT_ERR_SUCCESSS && sample_value < 0.0f)
        sample_value = 0.0f;
    this->record_operation_error(operation_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (0.0f);
    return (sample_value);
}
