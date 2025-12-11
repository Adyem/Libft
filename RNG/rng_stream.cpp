#include "rng_stream.hpp"
#include "rng.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Math/math.hpp"
#include <climits>
#include <limits>

rng_stream::rng_stream()
{
    std::random_device random_device;

    this->_engine.seed(random_device());
    this->_error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

rng_stream::rng_stream(uint32_t seed_value)
{
    this->_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    this->_error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

rng_stream::rng_stream(const rng_stream &other)
{
    std::lock_guard<std::mutex> lock_guard(other._mutex);

    this->_engine = other._engine;
    this->_error_code = other._error_code;
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

rng_stream &rng_stream::operator=(const rng_stream &other)
{
    if (this == &other)
        return (*this);
    std::lock(this->_mutex, other._mutex);
    std::lock_guard<std::mutex> this_lock(this->_mutex, std::adopt_lock);
    std::lock_guard<std::mutex> other_lock(other._mutex, std::adopt_lock);
    this->_engine = other._engine;
    this->_error_code = other._error_code;
    ft_errno = FT_ERR_SUCCESSS;
    return (*this);
}

rng_stream::rng_stream(rng_stream &&other) noexcept
{
    std::lock_guard<std::mutex> lock_guard(other._mutex);

    this->_engine = other._engine;
    this->_error_code = other._error_code;
    std::mt19937 default_engine;

    other._engine = default_engine;
    other._error_code = FT_ERR_SUCCESSS;
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

rng_stream &rng_stream::operator=(rng_stream &&other) noexcept
{
    if (this == &other)
        return (*this);
    std::lock(this->_mutex, other._mutex);
    std::lock_guard<std::mutex> this_lock(this->_mutex, std::adopt_lock);
    std::lock_guard<std::mutex> other_lock(other._mutex, std::adopt_lock);
    this->_engine = other._engine;
    this->_error_code = other._error_code;
    std::mt19937 default_engine;

    other._engine = default_engine;
    other._error_code = FT_ERR_SUCCESSS;
    ft_errno = FT_ERR_SUCCESSS;
    return (*this);
}

rng_stream::~rng_stream()
{
    return ;
}

void    rng_stream::reseed(uint32_t seed_value)
{
    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        this->_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void    rng_stream::reseed_from_string(const char *seed_string)
{
    if (seed_string == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    uint32_t derived_seed;

    derived_seed = ft_random_seed(seed_string);
    if (ft_errno != FT_ERR_SUCCESSS)
    {
        this->set_error(ft_errno);
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

    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        value = this->random_int_unlocked();
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (value);
}

int rng_stream::dice_roll(int number, int faces)
{
    if (faces == 0 && number == 0)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    if (faces < 1 || number < 1)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (faces == 1)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (number);
    }
    int result;
    int index;
    int overflowed;

    result = 0;
    index = 0;
    overflowed = 0;
    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        while (index < number)
        {
            int roll;

            roll = this->random_int_unlocked();
            if (result > INT_MAX - ((roll % faces) + 1))
            {
                overflowed = 1;
                break ;
            }
            result = result + ((roll % faces) + 1);
            index = index + 1;
        }
    }
    if (overflowed == 1)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

float   rng_stream::random_float()
{
    float value;

    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        value = this->random_float_unlocked();
    }
    this->set_error(FT_ERR_SUCCESSS);
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

    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        uniform_one = this->random_float_unlocked();
        if (uniform_one < 0.0000000001f)
            uniform_one = 0.0000000001f;
        uniform_two = this->random_float_unlocked();
    }
    radius = static_cast<float>(math_sqrt(-2.0 * math_log(uniform_one)));
    if (ft_errno != FT_ERR_SUCCESSS)
    {
        this->set_error(ft_errno);
        return (0.0f);
    }
    angle = 2.0f * pi_value * uniform_two;
    result = radius * static_cast<float>(math_cos(angle));
    if (ft_errno != FT_ERR_SUCCESSS)
    {
        this->set_error(ft_errno);
        return (0.0f);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

float   rng_stream::random_exponential(float lambda_value)
{
    float uniform_value;
    float result;

    if (lambda_value <= 0.0f)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0.0f);
    }
    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        uniform_value = this->random_float_unlocked();
    }
    if (uniform_value < 0.0000000001f)
        uniform_value = 0.0000000001f;
    result = static_cast<float>(-math_log(uniform_value)) / lambda_value;
    if (ft_errno != FT_ERR_SUCCESSS)
    {
        this->set_error(ft_errno);
        return (0.0f);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

int rng_stream::random_poisson(double lambda_value)
{
    double limit_value;
    double product_value;
    int count_value;

    if (lambda_value <= 0.0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    limit_value = math_exp(-lambda_value);
    if (ft_errno != FT_ERR_SUCCESSS)
    {
        this->set_error(ft_errno);
        return (0);
    }
    product_value = 1.0;
    count_value = 0;
    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        while (product_value > limit_value)
        {
            double random_value;

            random_value = static_cast<double>(this->random_float_unlocked());
            product_value = product_value * random_value;
            count_value = count_value + 1;
        }
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (count_value - 1);
}

int rng_stream::random_binomial(int trial_count, double success_probability)
{
    if (trial_count < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (success_probability < 0.0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (trial_count == 0)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    if (success_probability > 1.0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    const double probability_epsilon = std::numeric_limits<double>::epsilon();
    if (success_probability <= probability_epsilon)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    if ((1.0 - success_probability) <= probability_epsilon)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (trial_count);
    }
    int trial_index;
    int success_count;

    trial_index = 0;
    success_count = 0;
    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        while (trial_index < trial_count)
        {
            double random_value;

            random_value = static_cast<double>(this->random_float_unlocked());
            if (random_value < success_probability)
                success_count = success_count + 1;
            trial_index = trial_index + 1;
        }
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (success_count);
}

int rng_stream::random_geometric(double success_probability)
{
    if (success_probability <= 0.0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (success_probability > 1.0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    const double probability_epsilon = std::numeric_limits<double>::epsilon();
    if ((1.0 - success_probability) <= probability_epsilon)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (1);
    }
    int trial_count;
    int result;
    int found_success;

    trial_count = 1;
    found_success = 0;
    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

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
    if (found_success == 0)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (0);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

float   rng_stream::random_gamma(float shape, float scale)
{
    if (shape <= 0.0f || scale <= 0.0f)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0.0f);
    }
    std::gamma_distribution<float> distribution(shape, scale);
    float sample_value;

    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        sample_value = distribution(this->_engine);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (sample_value);
}

float   rng_stream::random_beta(float alpha, float beta)
{
    if (alpha <= 0.0f || beta <= 0.0f)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0.0f);
    }
    std::gamma_distribution<float> alpha_distribution(alpha, 1.0f);
    std::gamma_distribution<float> beta_distribution(beta, 1.0f);
    float alpha_sample;
    float beta_sample;
    float sum;
    float result;

    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        alpha_sample = alpha_distribution(this->_engine);
        beta_sample = beta_distribution(this->_engine);
    }
    sum = alpha_sample + beta_sample;
    if (sum <= 0.0f)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (0.0f);
    }
    result = alpha_sample / sum;
    if (result < 0.0f)
        result = 0.0f;
    if (result > 1.0f)
        result = 1.0f;
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

float   rng_stream::random_chi_squared(float degrees_of_freedom)
{
    if (degrees_of_freedom <= 0.0f)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0.0f);
    }
    std::gamma_distribution<float> distribution(degrees_of_freedom * 0.5f, 2.0f);
    float sample_value;

    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        sample_value = distribution(this->_engine);
    }
    if (sample_value < 0.0f)
        sample_value = 0.0f;
    this->set_error(FT_ERR_SUCCESSS);
    return (sample_value);
}

void    rng_stream::set_error(int error_code) const
{
    std::lock_guard<std::mutex> lock_guard(this->_mutex);

    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int rng_stream::get_error() const
{
    std::lock_guard<std::mutex> lock_guard(this->_mutex);
    int error_code;

    error_code = this->_error_code;
    return (error_code);
}

const char  *rng_stream::get_error_str() const
{
    int error_code;

    {
        std::lock_guard<std::mutex> lock_guard(this->_mutex);

        error_code = this->_error_code;
    }
    return (ft_strerror(error_code));
}
