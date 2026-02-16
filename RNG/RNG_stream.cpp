#include "rng_stream.hpp"
#include "rng.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Math/math.hpp"
#include "../PThread/pthread.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <climits>
#include <limits>
#include <new>

static int rng_stream_capture_math_error(ft_size_t previous_depth)
{
    (void)previous_depth;
    return (FT_ERR_SUCCESS);
}

void rng_stream::abort_lifecycle_error(const char *method_name, const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "rng_stream lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void rng_stream::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == rng_stream::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

rng_stream::rng_stream()
    : _engine()
    , _mutex(ft_nullptr)
    , _initialized_state(rng_stream::_state_uninitialized)
{
    std::random_device random_device;

    this->_engine.seed(random_device());
    return ;
}

rng_stream::rng_stream(uint32_t seed_value)
    : _engine()
    , _mutex(ft_nullptr)
    , _initialized_state(rng_stream::_state_uninitialized)
{
    this->_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    return ;
}


rng_stream::~rng_stream()
{
    if (this->_initialized_state == rng_stream::_state_uninitialized)
    {
        this->abort_lifecycle_error("rng_stream::~rng_stream",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == rng_stream::_state_initialized)
        (void)this->destroy();
    return ;
}

int rng_stream::enable_thread_safety()
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("rng_stream::enable_thread_safety");
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

int rng_stream::disable_thread_safety()
{
    int destroy_error;

    this->abort_if_not_initialized("rng_stream::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool rng_stream::is_thread_safe() const
{
    this->abort_if_not_initialized("rng_stream::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int rng_stream::initialize()
{
    if (this->_initialized_state == rng_stream::_state_initialized)
    {
        this->abort_lifecycle_error("rng_stream::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = rng_stream::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int rng_stream::initialize(const rng_stream &other)
{
    int initialize_error;
    int this_lock_error;
    int other_lock_error;
    int this_unlock_error;
    int other_unlock_error;
    int final_error;

    if (other._initialized_state != rng_stream::_state_initialized)
    {
        other.abort_lifecycle_error("rng_stream::initialize(const rng_stream &) source",
            "source is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == rng_stream::_state_initialized)
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
    if (this < &other)
    {
        if (this->_mutex != ft_nullptr)
            this_lock_error = this->_mutex->lock();
        if (this_lock_error == FT_ERR_SUCCESS && other._mutex != ft_nullptr)
            other_lock_error = other._mutex->lock();
    }
    else
    {
        if (other._mutex != ft_nullptr)
            other_lock_error = other._mutex->lock();
        if (other_lock_error == FT_ERR_SUCCESS && this->_mutex != ft_nullptr)
            this_lock_error = this->_mutex->lock();
    }
    if (this_lock_error != FT_ERR_SUCCESS || other_lock_error != FT_ERR_SUCCESS)
    {
        if (other._mutex != ft_nullptr && other_lock_error == FT_ERR_SUCCESS)
            (void)other._mutex->unlock();
        if (this->_mutex != ft_nullptr && this_lock_error == FT_ERR_SUCCESS)
            (void)this->_mutex->unlock();
        (void)this->destroy();
        if (this_lock_error != FT_ERR_SUCCESS)
            return (this_lock_error);
        return (other_lock_error);
    }
    this->_engine = other._engine;
    this_unlock_error = FT_ERR_SUCCESS;
    other_unlock_error = FT_ERR_SUCCESS;
    if (this->_mutex != ft_nullptr)
        this_unlock_error = this->_mutex->unlock();
    if (other._mutex != ft_nullptr)
        other_unlock_error = other._mutex->unlock();
    final_error = this_unlock_error;
    if (final_error == FT_ERR_SUCCESS)
        final_error = other_unlock_error;
    if (final_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (final_error);
    }
    return (FT_ERR_SUCCESS);
}

int rng_stream::initialize(rng_stream &&other)
{
    int initialize_error;
    int this_lock_error;
    int other_lock_error;
    int this_unlock_error;
    int other_unlock_error;
    int final_error;
    std::mt19937 default_engine;

    if (other._initialized_state != rng_stream::_state_initialized)
    {
        other.abort_lifecycle_error("rng_stream::initialize(rng_stream &&) source",
            "source is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == rng_stream::_state_initialized)
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
    if (this < &other)
    {
        if (this->_mutex != ft_nullptr)
            this_lock_error = this->_mutex->lock();
        if (this_lock_error == FT_ERR_SUCCESS && other._mutex != ft_nullptr)
            other_lock_error = other._mutex->lock();
    }
    else
    {
        if (other._mutex != ft_nullptr)
            other_lock_error = other._mutex->lock();
        if (other_lock_error == FT_ERR_SUCCESS && this->_mutex != ft_nullptr)
            this_lock_error = this->_mutex->lock();
    }
    if (this_lock_error != FT_ERR_SUCCESS || other_lock_error != FT_ERR_SUCCESS)
    {
        if (other._mutex != ft_nullptr && other_lock_error == FT_ERR_SUCCESS)
            (void)other._mutex->unlock();
        if (this->_mutex != ft_nullptr && this_lock_error == FT_ERR_SUCCESS)
            (void)this->_mutex->unlock();
        (void)this->destroy();
        if (this_lock_error != FT_ERR_SUCCESS)
            return (this_lock_error);
        return (other_lock_error);
    }
    this->_engine = other._engine;
    other._engine = default_engine;
    this_unlock_error = FT_ERR_SUCCESS;
    other_unlock_error = FT_ERR_SUCCESS;
    if (this->_mutex != ft_nullptr)
        this_unlock_error = this->_mutex->unlock();
    if (other._mutex != ft_nullptr)
        other_unlock_error = other._mutex->unlock();
    final_error = this_unlock_error;
    if (final_error == FT_ERR_SUCCESS)
        final_error = other_unlock_error;
    if (final_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (final_error);
    }
    return (FT_ERR_SUCCESS);
}

int rng_stream::destroy()
{
    int disable_error;

    if (this->_initialized_state != rng_stream::_state_initialized)
    {
        this->abort_lifecycle_error("rng_stream::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    disable_error = this->disable_thread_safety();
    this->_initialized_state = rng_stream::_state_destroyed;
    return (disable_error);
}

int rng_stream::reseed(uint32_t seed_value)
{
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::reseed");
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
    if (operation_error == FT_ERR_SUCCESS)
        this->_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (operation_error);
}

int rng_stream::reseed_from_string(const char *seed_string)
{
    uint32_t derived_seed;

    this->abort_if_not_initialized("rng_stream::reseed_from_string");
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
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::random_int");
    value = 0;
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
    if (operation_error == FT_ERR_SUCCESS)
        value = this->random_int_unlocked();
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (Pair<int, int>(operation_error, value));
}

Pair<int, int> rng_stream::dice_roll(int number, int faces)
{
    int result;
    int index;
    bool overflowed;
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::dice_roll");
    if (faces == 0 && number == 0)
        return (Pair<int, int>(FT_ERR_SUCCESS, 0));
    if (faces < 1 || number < 1)
        return (Pair<int, int>(FT_ERR_INVALID_ARGUMENT, 0));
    if (faces == 1)
        return (Pair<int, int>(FT_ERR_SUCCESS, number));
    result = 0;
    index = 0;
    overflowed = false;
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
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
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    if (overflowed)
        return (Pair<int, int>(operation_error, 0));
    return (Pair<int, int>(operation_error, result));
}

Pair<int, float> rng_stream::random_float()
{
    float value;
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::random_float");
    value = 0.0f;
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
    if (operation_error == FT_ERR_SUCCESS)
        value = this->random_float_unlocked();
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
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
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::random_normal");
    uniform_one = 0.0f;
    uniform_two = 0.0f;
    radius = 0.0f;
    angle = 0.0f;
    result = 0.0f;
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
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
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (Pair<int, float>(operation_error, result));
}

Pair<int, float> rng_stream::random_exponential(float lambda_value)
{
    float uniform_value;
    float result;
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::random_exponential");
    if (lambda_value <= 0.0f)
        return (Pair<int, float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    uniform_value = 0.0f;
    result = 0.0f;
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
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
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (Pair<int, float>(operation_error, result));
}

Pair<int, int> rng_stream::random_poisson(double lambda_value)
{
    double limit_value;
    double product_value;
    int count_value;
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::random_poisson");
    if (lambda_value <= 0.0)
        return (Pair<int, int>(FT_ERR_INVALID_ARGUMENT, 0));
    limit_value = math_exp(-lambda_value);
    if (rng_stream_capture_math_error(0) != FT_ERR_SUCCESS)
        return (Pair<int, int>(FT_ERR_INTERNAL, 0));
    product_value = 1.0;
    count_value = 0;
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
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
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    return (Pair<int, int>(operation_error, count_value - 1));
}

Pair<int, int> rng_stream::random_binomial(int trial_count, double success_probability)
{
    const double probability_epsilon = std::numeric_limits<double>::epsilon();
    int trial_index;
    int success_count;
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::random_binomial");
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
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
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
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
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
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::random_geometric");
    if (success_probability <= 0.0)
        return (Pair<int, int>(FT_ERR_INVALID_ARGUMENT, 0));
    if (success_probability > 1.0)
        return (Pair<int, int>(FT_ERR_INVALID_ARGUMENT, 0));
    if ((1.0 - success_probability) <= probability_epsilon)
        return (Pair<int, int>(FT_ERR_SUCCESS, 1));
    trial_count = 1;
    found_success = 0;
    result = 0;
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
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
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
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
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::random_gamma");
    if (shape <= 0.0f || scale <= 0.0f)
        return (Pair<int, float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    sample_value = 0.0f;
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
    if (operation_error == FT_ERR_SUCCESS)
        sample_value = distribution(this->_engine);
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
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
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::random_beta");
    if (alpha <= 0.0f || beta <= 0.0f)
        return (Pair<int, float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    alpha_sample = 0.0f;
    beta_sample = 0.0f;
    result = 0.0f;
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
    if (operation_error == FT_ERR_SUCCESS)
    {
        alpha_sample = alpha_distribution(this->_engine);
        beta_sample = beta_distribution(this->_engine);
    }
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
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
    int operation_error;
    int unlock_error;

    this->abort_if_not_initialized("rng_stream::random_chi_squared");
    if (degrees_of_freedom <= 0.0f)
        return (Pair<int, float>(FT_ERR_INVALID_ARGUMENT, 0.0f));
    sample_value = 0.0f;
    if (this->_mutex == ft_nullptr)
        operation_error = FT_ERR_SUCCESS;
    else
        operation_error = this->_mutex->lock();
    if (operation_error == FT_ERR_SUCCESS)
        sample_value = distribution(this->_engine);
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
    if (operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    if (operation_error == FT_ERR_SUCCESS && sample_value < 0.0f)
        sample_value = 0.0f;
    return (Pair<int, float>(operation_error, sample_value));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *rng_stream::get_mutex_for_validation() const
{
    return (this->_mutex);
}
#endif
