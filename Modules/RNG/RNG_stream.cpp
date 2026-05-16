#include "rng_stream.hpp"
#include "../PThread/pthread_internal.hpp"
#include "rng.hpp"
#include "rng_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Math/math.hpp"
#include "../PThread/pthread.hpp"
#include <climits>
#include <limits>
#include <new>

static int32_t rng_stream_capture_math_error(ft_size_t previous_depth)
{
    (void)previous_depth;
    return (FT_ERR_SUCCESS);
}

rng_stream::rng_stream() noexcept
    : _engine()
    , _mutex(ft_nullptr)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

rng_stream::~rng_stream() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t rng_stream::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int32_t rng_stream::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool rng_stream::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t rng_stream::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "rng_stream::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        ft_seed_random_engine_with_entropy();
    this->_engine = g_random_engine;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t rng_stream::initialize(const rng_stream &other) noexcept
{
    int32_t initialize_error;
    int32_t this_lock_error;
    int32_t other_lock_error;
    ft_bool lock_this_first;
    ft_bool this_locked;
    ft_bool other_locked;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "rng_stream::initialize(const rng_stream &) source",
            "source is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            initialize_error = this->destroy();
            if (initialize_error != FT_ERR_SUCCESS)
                return (initialize_error);
        }
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this_lock_error = FT_ERR_SUCCESS;
    other_lock_error = FT_ERR_SUCCESS;
    lock_this_first = FT_FALSE;
    if (this < &other)
        lock_this_first = FT_TRUE;
    this_locked = FT_FALSE;
    other_locked = FT_FALSE;
    if (lock_this_first == FT_TRUE)
    {
        if (this->_mutex != ft_nullptr)
        {
            this_lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
            if (this_lock_error == FT_ERR_SUCCESS)
                this_locked = FT_TRUE;
        }
        if (this_lock_error == FT_ERR_SUCCESS && other._mutex != ft_nullptr)
        {
            other_lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
            if (other_lock_error == FT_ERR_SUCCESS)
                other_locked = FT_TRUE;
        }
    }
    else
    {
        if (other._mutex != ft_nullptr)
        {
            other_lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
            if (other_lock_error == FT_ERR_SUCCESS)
                other_locked = FT_TRUE;
        }
        if (other_lock_error == FT_ERR_SUCCESS && this->_mutex != ft_nullptr)
        {
            this_lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
            if (this_lock_error == FT_ERR_SUCCESS)
                this_locked = FT_TRUE;
        }
    }
    if (this_lock_error != FT_ERR_SUCCESS || other_lock_error != FT_ERR_SUCCESS)
    {
        if (this_locked == FT_TRUE)
            (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        if (other_locked == FT_TRUE)
            (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
        (void)this->destroy();
        if (this_lock_error != FT_ERR_SUCCESS)
            return (this_lock_error);
        return (other_lock_error);
    }
    this->_engine = other._engine;
    if (lock_this_first == FT_TRUE)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    }
    else
    {
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    }
    return (FT_ERR_SUCCESS);
}

int32_t rng_stream::initialize(rng_stream &&other) noexcept
{
    int32_t initialize_error;
    int32_t this_lock_error;
    int32_t other_lock_error;
    ft_bool lock_this_first;
    ft_bool this_locked;
    ft_bool other_locked;
    std::mt19937 default_engine;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "rng_stream::initialize(rng_stream &&) source",
            "source is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            initialize_error = this->destroy();
            if (initialize_error != FT_ERR_SUCCESS)
                return (initialize_error);
        }
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this_lock_error = FT_ERR_SUCCESS;
    other_lock_error = FT_ERR_SUCCESS;
    lock_this_first = FT_FALSE;
    if (this < &other)
        lock_this_first = FT_TRUE;
    this_locked = FT_FALSE;
    other_locked = FT_FALSE;
    if (lock_this_first == FT_TRUE)
    {
        if (this->_mutex != ft_nullptr)
        {
            this_lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
            if (this_lock_error == FT_ERR_SUCCESS)
                this_locked = FT_TRUE;
        }
        if (this_lock_error == FT_ERR_SUCCESS && other._mutex != ft_nullptr)
        {
            other_lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
            if (other_lock_error == FT_ERR_SUCCESS)
                other_locked = FT_TRUE;
        }
    }
    else
    {
        if (other._mutex != ft_nullptr)
        {
            other_lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
            if (other_lock_error == FT_ERR_SUCCESS)
                other_locked = FT_TRUE;
        }
        if (other_lock_error == FT_ERR_SUCCESS && this->_mutex != ft_nullptr)
        {
            this_lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
            if (this_lock_error == FT_ERR_SUCCESS)
                this_locked = FT_TRUE;
        }
    }
    if (this_lock_error != FT_ERR_SUCCESS || other_lock_error != FT_ERR_SUCCESS)
    {
        if (this_locked == FT_TRUE)
            (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        if (other_locked == FT_TRUE)
            (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
        (void)this->destroy();
        if (this_lock_error != FT_ERR_SUCCESS)
            return (this_lock_error);
        return (other_lock_error);
    }
    this->_engine = other._engine;
    other._engine = default_engine;
    if (lock_this_first == FT_TRUE)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    }
    else
    {
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    }
    return (FT_ERR_SUCCESS);
}

uint32_t rng_stream::move(rng_stream &other) noexcept
{
    return (static_cast<uint32_t>(this->initialize(
            static_cast<rng_stream &&>(other))));
}

int32_t rng_stream::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

int32_t rng_stream::reseed(uint32_t seed_value) noexcept
{
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::reseed");
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (operation_error == FT_ERR_SUCCESS)
        this->_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (operation_error);
}

int32_t rng_stream::reseed_from_string(const char *seed_string) noexcept
{
    uint32_t derived_seed;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::reseed_from_string");
    if (seed_string == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    derived_seed = ft_random_seed(seed_string);
    return (this->reseed(derived_seed));
}

int32_t rng_stream::random_int_unlocked()
{
    std::uniform_int_distribution<int32_t> distribution(0, std::numeric_limits<int32_t>::max());
    int32_t random_value;

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

Pair<int32_t,  int32_t> rng_stream::random_int()
{
    int32_t value;
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::random_int");
    value = 0;
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (operation_error == FT_ERR_SUCCESS)
        value = this->random_int_unlocked();
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (Pair<int32_t,  int32_t>(operation_error, value));
}

Pair<int32_t,  int32_t> rng_stream::dice_roll(int32_t number, int32_t faces)
{
    int32_t result;
    int32_t index;
    ft_bool overflowed;
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::dice_roll");
    if (faces == 0 && number == 0)
        return (Pair<int32_t,  int32_t>(FT_ERR_SUCCESS, 0));
    if (faces < 1 || number < 1)
        return (Pair<int32_t,  int32_t>(FT_ERR_INVALID_ARGUMENT, 0));
    if (faces == 1)
        return (Pair<int32_t,  int32_t>(FT_ERR_SUCCESS, number));
    result = 0;
    index = 0;
    overflowed = FT_FALSE;
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (operation_error == FT_ERR_SUCCESS)
    {
        while (index < number)
        {
            int32_t roll_value;

            roll_value = this->random_int_unlocked();
            if (result > INT_MAX - ((roll_value % faces) + 1))
            {
                overflowed = FT_TRUE;
                operation_error = FT_ERR_OUT_OF_RANGE;
                break ;
            }
            result = result + ((roll_value % faces) + 1);
            index = index + 1;
        }
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (overflowed)
        return (Pair<int32_t,  int32_t>(operation_error, 0));
    return (Pair<int32_t,  int32_t>(operation_error, result));
}

Pair<int32_t,  float> rng_stream::random_float()
{
    float value;
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::random_float");
    value = 0.0f;
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (operation_error == FT_ERR_SUCCESS)
        value = this->random_float_unlocked();
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (Pair<int32_t,  float>(operation_error, value));
}

Pair<int32_t,  float> rng_stream::random_normal()
{
    float uniform_one;
    float uniform_two;
    float radius;
    float angle;
    float result;
    const float PI_VALUE = 3.14159265358979323846f;
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::random_normal");
    uniform_one = 0.0f;
    uniform_two = 0.0f;
    radius = 0.0f;
    angle = 0.0f;
    result = 0.0f;
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
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
        angle = 2.0f * PI_VALUE * uniform_two;
        result = radius * static_cast<float>(math_cos(angle));
        if (rng_stream_capture_math_error(0) != FT_ERR_SUCCESS)
            operation_error = FT_ERR_INTERNAL;
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (Pair<int32_t,  float>(operation_error, result));
}

Pair<int32_t,  float> rng_stream::random_exponential(float lambda_value)
{
    float uniform_value;
    float result;
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::random_exponential");
    if (lambda_value <= 0.0f)
        return (Pair<int32_t,  float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    uniform_value = 0.0f;
    result = 0.0f;
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
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
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (Pair<int32_t,  float>(operation_error, result));
}

Pair<int32_t,  int32_t> rng_stream::random_poisson(double lambda_value)
{
    double limit_value;
    double product_value;
    int32_t count_value;
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::random_poisson");
    if (lambda_value <= 0.0)
        return (Pair<int32_t,  int32_t>(FT_ERR_INVALID_ARGUMENT, 0));
    limit_value = math_exp(-lambda_value);
    if (rng_stream_capture_math_error(0) != FT_ERR_SUCCESS)
        return (Pair<int32_t,  int32_t>(FT_ERR_INTERNAL, 0));
    product_value = 1.0;
    count_value = 0;
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
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
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (Pair<int32_t,  int32_t>(operation_error, count_value - 1));
}

Pair<int32_t,  int32_t> rng_stream::random_binomial(int32_t trial_count, double success_probability)
{
    const double PROBABILITY_EPSILON = std::numeric_limits<double>::epsilon();
    int32_t trial_index;
    int32_t success_count;
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::random_binomial");
    if (trial_count < 0)
        return (Pair<int32_t,  int32_t>(FT_ERR_INVALID_ARGUMENT, 0));
    if (success_probability < 0.0)
        return (Pair<int32_t,  int32_t>(FT_ERR_INVALID_ARGUMENT, 0));
    if (trial_count == 0)
        return (Pair<int32_t,  int32_t>(FT_ERR_SUCCESS, 0));
    if (success_probability > 1.0)
        return (Pair<int32_t,  int32_t>(FT_ERR_INVALID_ARGUMENT, 0));
    if (success_probability <= PROBABILITY_EPSILON)
        return (Pair<int32_t,  int32_t>(FT_ERR_SUCCESS, 0));
    if ((1.0 - success_probability) <= PROBABILITY_EPSILON)
        return (Pair<int32_t,  int32_t>(FT_ERR_SUCCESS, trial_count));
    trial_index = 0;
    success_count = 0;
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
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
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (Pair<int32_t,  int32_t>(operation_error, success_count));
}

Pair<int32_t,  int32_t> rng_stream::random_geometric(double success_probability)
{
    const double PROBABILITY_EPSILON = std::numeric_limits<double>::epsilon();
    int32_t trial_count;
    int32_t result;
    int32_t found_success;
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::random_geometric");
    if (success_probability <= 0.0)
        return (Pair<int32_t,  int32_t>(FT_ERR_INVALID_ARGUMENT, 0));
    if (success_probability > 1.0)
        return (Pair<int32_t,  int32_t>(FT_ERR_INVALID_ARGUMENT, 0));
    if ((1.0 - success_probability) <= PROBABILITY_EPSILON)
        return (Pair<int32_t,  int32_t>(FT_ERR_SUCCESS, 1));
    trial_count = 1;
    found_success = 0;
    result = 0;
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
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
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (found_success == 0 && operation_error == FT_ERR_SUCCESS)
        operation_error = FT_ERR_INVALID_STATE;
    return (Pair<int32_t,  int32_t>(operation_error, result));
}

Pair<int32_t,  float> rng_stream::random_gamma(float shape, float scale)
{
    std::gamma_distribution<float> distribution(shape, scale);
    float sample_value;
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::random_gamma");
    if (shape <= 0.0f || scale <= 0.0f)
        return (Pair<int32_t,  float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    sample_value = 0.0f;
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (operation_error == FT_ERR_SUCCESS)
        sample_value = distribution(this->_engine);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (Pair<int32_t,  float>(operation_error, sample_value));
}

Pair<int32_t,  float> rng_stream::random_beta(float alpha, float beta)
{
    std::gamma_distribution<float> alpha_distribution(alpha, 1.0f);
    std::gamma_distribution<float> beta_distribution(beta, 1.0f);
    float alpha_sample;
    float beta_sample;
    float sum;
    float result;
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::random_beta");
    if (alpha <= 0.0f || beta <= 0.0f)
        return (Pair<int32_t,  float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    alpha_sample = 0.0f;
    beta_sample = 0.0f;
    result = 0.0f;
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (operation_error == FT_ERR_SUCCESS)
    {
        alpha_sample = alpha_distribution(this->_engine);
        beta_sample = beta_distribution(this->_engine);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
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
    return (Pair<int32_t,  float>(operation_error, result));
}

Pair<int32_t,  float> rng_stream::random_chi_squared(float degrees_of_freedom)
{
    std::gamma_distribution<float> distribution(degrees_of_freedom * 0.5f, 2.0f);
    float sample_value;
    int32_t operation_error;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "rng_stream::random_chi_squared");
    if (degrees_of_freedom <= 0.0f)
        return (Pair<int32_t,  float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    sample_value = 0.0f;
    operation_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (operation_error == FT_ERR_SUCCESS)
        sample_value = distribution(this->_engine);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (operation_error == FT_ERR_SUCCESS && sample_value < 0.0f)
        sample_value = 0.0f;
    return (Pair<int32_t,  float>(operation_error, sample_value));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *rng_stream::get_mutex_for_validation() const
{
    return (this->_mutex);
}
#endif
