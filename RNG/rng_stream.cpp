#include "rng_stream.hpp"
#include "rng.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Math/math.hpp"
#include "../PThread/pthread.hpp"
#include <climits>
#include <limits>

static int rng_stream_capture_math_error(ft_size_t previous_depth)
{
    (void)previous_depth;
    return (FT_ERR_SUCCESS);
}

int rng_stream::lock_internal(bool *lock_acquired) const
{
    int mutex_result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    mutex_result = this->_mutex.lock();
    if (mutex_result == FT_ERR_SUCCESS && lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (mutex_result);
}

int rng_stream::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired)
        return (FT_ERR_SUCCESS);
    return (this->_mutex.unlock());
}

pt_recursive_mutex *rng_stream::mutex_handle()
{
    return (&this->_mutex);
}

const pt_recursive_mutex *rng_stream::mutex_handle() const
{
    return (&this->_mutex);
}

rng_stream::rng_stream()
{
    std::random_device random_device;

    this->_engine.seed(random_device());
    return ;
}

rng_stream::rng_stream(uint32_t seed_value)
{
    this->_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    return ;
}

rng_stream::rng_stream(const rng_stream &other)
{
    bool lock_acquired;

    lock_acquired = false;
    if (other.lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_engine = other._engine;
    (void)other.unlock_internal(lock_acquired);
    return ;
}

rng_stream &rng_stream::operator=(const rng_stream &other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    int lock_error;

    if (this == &other)
        return (*this);
    this_lock_acquired = false;
    other_lock_acquired = false;
    if (this < &other)
    {
        lock_error = this->lock_internal(&this_lock_acquired);
        if (lock_error == FT_ERR_SUCCESS)
            lock_error = other.lock_internal(&other_lock_acquired);
    }
    else
    {
        lock_error = other.lock_internal(&other_lock_acquired);
        if (lock_error == FT_ERR_SUCCESS)
            lock_error = this->lock_internal(&this_lock_acquired);
    }
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)other.unlock_internal(other_lock_acquired);
        (void)this->unlock_internal(this_lock_acquired);
        return (*this);
    }
    this->_engine = other._engine;
    if (this < &other)
    {
        (void)other.unlock_internal(other_lock_acquired);
        (void)this->unlock_internal(this_lock_acquired);
    }
    else
    {
        (void)this->unlock_internal(this_lock_acquired);
        (void)other.unlock_internal(other_lock_acquired);
    }
    return (*this);
}

rng_stream::rng_stream(rng_stream &&other) noexcept
{
    bool lock_acquired;
    std::mt19937 default_engine;

    lock_acquired = false;
    if (other.lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_engine = other._engine;
    other._engine = default_engine;
    (void)other.unlock_internal(lock_acquired);
    return ;
}

rng_stream &rng_stream::operator=(rng_stream &&other) noexcept
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    int lock_error;
    std::mt19937 default_engine;

    if (this == &other)
        return (*this);
    this_lock_acquired = false;
    other_lock_acquired = false;
    if (this < &other)
    {
        lock_error = this->lock_internal(&this_lock_acquired);
        if (lock_error == FT_ERR_SUCCESS)
            lock_error = other.lock_internal(&other_lock_acquired);
    }
    else
    {
        lock_error = other.lock_internal(&other_lock_acquired);
        if (lock_error == FT_ERR_SUCCESS)
            lock_error = this->lock_internal(&this_lock_acquired);
    }
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)other.unlock_internal(other_lock_acquired);
        (void)this->unlock_internal(this_lock_acquired);
        return (*this);
    }
    this->_engine = other._engine;
    other._engine = default_engine;
    if (this < &other)
    {
        (void)other.unlock_internal(other_lock_acquired);
        (void)this->unlock_internal(this_lock_acquired);
    }
    else
    {
        (void)this->unlock_internal(this_lock_acquired);
        (void)other.unlock_internal(other_lock_acquired);
    }
    return (*this);
}

rng_stream::~rng_stream()
{
    return ;
}

int rng_stream::reseed(uint32_t seed_value)
{
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        this->_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (operation_error);
}

int rng_stream::reseed_from_string(const char *seed_string)
{
    uint32_t derived_seed;

    if (seed_string == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    derived_seed = ft_random_seed(seed_string);
    return (this->reseed(derived_seed));
}

int rng_stream::random_int_unlocked()
{
    std::uniform_int_distribution<int> distribution(0, std::numeric_limits<int>::max());
    int random_value;

    random_value = distribution(this->_engine);
    return (random_value);
}

float rng_stream::random_float_unlocked()
{
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    float random_value;

    random_value = distribution(this->_engine);
    return (random_value);
}

Pair<int, int> rng_stream::random_int()
{
    int value;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    value = 0;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        value = this->random_int_unlocked();
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (Pair<int, int>(operation_error, value));
}

Pair<int, int> rng_stream::dice_roll(int number, int faces)
{
    int result;
    int index;
    bool overflowed;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    if (faces == 0 && number == 0)
        return (Pair<int, int>(FT_ERR_SUCCESS, 0));
    if (faces < 1 || number < 1)
        return (Pair<int, int>(FT_ERR_INVALID_ARGUMENT, 0));
    if (faces == 1)
        return (Pair<int, int>(FT_ERR_SUCCESS, number));
    result = 0;
    index = 0;
    overflowed = false;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
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
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    if (overflowed)
        return (Pair<int, int>(operation_error, 0));
    return (Pair<int, int>(operation_error, result));
}

Pair<int, float> rng_stream::random_float()
{
    float value;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    value = 0.0f;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        value = this->random_float_unlocked();
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (Pair<int, float>(operation_error, value));
}

Pair<int, float> rng_stream::random_normal()
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
    if (operation_error == FT_ERR_SUCCESS)
    {
        uniform_one = this->random_float_unlocked();
        if (uniform_one < 0.0000000001f)
            uniform_one = 0.0000000001f;
        uniform_two = this->random_float_unlocked();
    }
    if (operation_error == FT_ERR_SUCCESS)
    {
        radius = static_cast<float>(math_sqrt(-2.0 * math_log(uniform_one)));
        if (rng_stream_capture_math_error(0) != FT_ERR_SUCCESS)
            operation_error = FT_ERR_INTERNAL;
    }
    if (operation_error == FT_ERR_SUCCESS)
    {
        angle = 2.0f * pi_value * uniform_two;
        result = radius * static_cast<float>(math_cos(angle));
        if (rng_stream_capture_math_error(0) != FT_ERR_SUCCESS)
            operation_error = FT_ERR_INTERNAL;
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (Pair<int, float>(operation_error, result));
}

Pair<int, float> rng_stream::random_exponential(float lambda_value)
{
    float uniform_value;
    float result;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    if (lambda_value <= 0.0f)
        return (Pair<int, float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    uniform_value = 0.0f;
    result = 0.0f;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        uniform_value = this->random_float_unlocked();
    if (operation_error == FT_ERR_SUCCESS)
    {
        if (uniform_value < 0.0000000001f)
            uniform_value = 0.0000000001f;
        result = static_cast<float>(-math_log(uniform_value)) / lambda_value;
        if (rng_stream_capture_math_error(0) != FT_ERR_SUCCESS)
            operation_error = FT_ERR_INTERNAL;
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (Pair<int, float>(operation_error, result));
}

Pair<int, int> rng_stream::random_poisson(double lambda_value)
{
    double limit_value;
    double product_value;
    int count_value;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    if (lambda_value <= 0.0)
        return (Pair<int, int>(FT_ERR_INVALID_ARGUMENT, 0));
    limit_value = math_exp(-lambda_value);
    if (rng_stream_capture_math_error(0) != FT_ERR_SUCCESS)
        return (Pair<int, int>(FT_ERR_INTERNAL, 0));
    product_value = 1.0;
    count_value = 0;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
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
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (Pair<int, int>(operation_error, count_value - 1));
}

Pair<int, int> rng_stream::random_binomial(int trial_count, double success_probability)
{
    const double probability_epsilon = std::numeric_limits<double>::epsilon();
    int trial_index;
    int success_count;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    if (trial_count < 0)
        return (Pair<int, int>(FT_ERR_INVALID_ARGUMENT, 0));
    if (success_probability < 0.0)
        return (Pair<int, int>(FT_ERR_INVALID_ARGUMENT, 0));
    if (trial_count == 0)
        return (Pair<int, int>(FT_ERR_SUCCESS, 0));
    if (success_probability > 1.0)
        return (Pair<int, int>(FT_ERR_INVALID_ARGUMENT, 0));
    if (success_probability <= probability_epsilon)
        return (Pair<int, int>(FT_ERR_SUCCESS, 0));
    if ((1.0 - success_probability) <= probability_epsilon)
        return (Pair<int, int>(FT_ERR_SUCCESS, trial_count));
    trial_index = 0;
    success_count = 0;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
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
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (Pair<int, int>(operation_error, success_count));
}

Pair<int, int> rng_stream::random_geometric(double success_probability)
{
    const double probability_epsilon = std::numeric_limits<double>::epsilon();
    int trial_count;
    int result;
    int found_success;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    if (success_probability <= 0.0)
        return (Pair<int, int>(FT_ERR_INVALID_ARGUMENT, 0));
    if (success_probability > 1.0)
        return (Pair<int, int>(FT_ERR_INVALID_ARGUMENT, 0));
    if ((1.0 - success_probability) <= probability_epsilon)
        return (Pair<int, int>(FT_ERR_SUCCESS, 1));
    trial_count = 1;
    found_success = 0;
    result = 0;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
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
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    if (found_success == 0 && operation_error == FT_ERR_SUCCESS)
        operation_error = FT_ERR_INVALID_STATE;
    return (Pair<int, int>(operation_error, result));
}

Pair<int, float> rng_stream::random_gamma(float shape, float scale)
{
    std::gamma_distribution<float> distribution(shape, scale);
    float sample_value;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    if (shape <= 0.0f || scale <= 0.0f)
        return (Pair<int, float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    sample_value = 0.0f;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        sample_value = distribution(this->_engine);
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (Pair<int, float>(operation_error, sample_value));
}

Pair<int, float> rng_stream::random_beta(float alpha, float beta)
{
    std::gamma_distribution<float> alpha_distribution(alpha, 1.0f);
    std::gamma_distribution<float> beta_distribution(beta, 1.0f);
    float alpha_sample;
    float beta_sample;
    float sum;
    float result;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    if (alpha <= 0.0f || beta <= 0.0f)
        return (Pair<int, float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    alpha_sample = 0.0f;
    beta_sample = 0.0f;
    result = 0.0f;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
    {
        alpha_sample = alpha_distribution(this->_engine);
        beta_sample = beta_distribution(this->_engine);
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    sum = alpha_sample + beta_sample;
    if (operation_error == FT_ERR_SUCCESS && sum <= 0.0f)
        operation_error = FT_ERR_INVALID_STATE;
    if (operation_error == FT_ERR_SUCCESS)
    {
        result = alpha_sample / sum;
        if (result < 0.0f)
            result = 0.0f;
        if (result > 1.0f)
            result = 1.0f;
    }
    return (Pair<int, float>(operation_error, result));
}

Pair<int, float> rng_stream::random_chi_squared(float degrees_of_freedom)
{
    std::gamma_distribution<float> distribution(degrees_of_freedom * 0.5f, 2.0f);
    float sample_value;
    bool lock_acquired;
    int operation_error;
    int unlock_error;

    if (degrees_of_freedom <= 0.0f)
        return (Pair<int, float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    sample_value = 0.0f;
    lock_acquired = false;
    operation_error = this->lock_internal(&lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        sample_value = distribution(this->_engine);
    unlock_error = this->unlock_internal(lock_acquired);
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    if (operation_error == FT_ERR_SUCCESS && sample_value < 0.0f)
        sample_value = 0.0f;
    return (Pair<int, float>(operation_error, sample_value));
}
