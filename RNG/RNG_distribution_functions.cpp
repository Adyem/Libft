#include "rng.hpp"
#include <cmath>
#include <cstddef>
#include <limits>

struct rng_gamma_context
{
    double shape;
    double scale;
};

struct rng_beta_context
{
    double alpha;
    double beta;
};

typedef double (*rng_integrand)(double value, void *context);

static int rng_adjust_segment_total(int segment_total)
{
    if ((segment_total & 1) == 1)
        segment_total = segment_total + 1;
    if (segment_total < 256)
        segment_total = 256;
    if (segment_total > 8192)
        segment_total = 8192;
    return (segment_total);
}

static double rng_simpson_integral(double start, double end, int segment_total, rng_integrand function, void *context)
{
    double step;
    int index;
    double sum;

    if (segment_total <= 0)
        return (0.0);
    step = (end - start) / static_cast<double>(segment_total);
    sum = 0.0;
    index = 0;
    while (index <= segment_total)
    {
        double point;
        double weight;
        double value;

        point = start + step * static_cast<double>(index);
        if (index == 0 || index == segment_total)
            weight = 1.0;
        else if ((index & 1) == 0)
            weight = 2.0;
        else
            weight = 4.0;
        value = function(point, context);
        if (std::isfinite(value) == false)
        {
            double adjustment;
            double adjusted_point;

            adjustment = step * 0.5;
            if (adjustment <= 0.0)
                adjustment = 0.0000000001;
            adjusted_point = point;
            if (index == 0)
            {
                adjusted_point = point + adjustment;
                if (adjusted_point > end)
                    adjusted_point = end;
            }
            else if (index == segment_total)
            {
                adjusted_point = point - adjustment;
                if (adjusted_point < start)
                    adjusted_point = start;
            }
            value = function(adjusted_point, context);
            if (std::isfinite(value) == false)
                value = 0.0;
        }
        sum = sum + weight * value;
        index = index + 1;
    }
    return ((step / 3.0) * sum);
}

static double rng_gamma_pdf_integrand(double value, void *context)
{
    rng_gamma_context *parameters;

    parameters = static_cast<rng_gamma_context *>(context);
    return (rng_gamma_pdf(parameters->shape, parameters->scale, value));
}

static double rng_beta_pdf_integrand(double value, void *context)
{
    rng_beta_context *parameters;

    parameters = static_cast<rng_beta_context *>(context);
    return (rng_beta_pdf(parameters->alpha, parameters->beta, value));
}

double rng_gamma_pdf(double shape, double scale, double value)
{
    double log_component;
    double result;
    double epsilon;

    if (shape <= 0.0 || scale <= 0.0)
    {
        return (0.0);
    }
    if (value < 0.0)
    {
        return (0.0);
    }
    epsilon = 0.000000000001;
    if (std::fabs(value) <= epsilon)
    {
        if (shape < 1.0)
        {
            return (std::numeric_limits<double>::infinity());
        }
        if (std::fabs(shape - 1.0) <= epsilon)
        {
            return (1.0 / scale);
        }
        return (0.0);
    }
    log_component = (shape - 1.0) * std::log(value) - (value / scale);
    log_component = log_component - shape * std::log(scale);
    log_component = log_component - std::lgamma(shape);
    result = std::exp(log_component);
    if (std::isfinite(result) == false)
    {
        return (0.0);
    }
    return (result);
}

double rng_gamma_cdf(double shape, double scale, double value)
{
    rng_gamma_context context;
    double integral;
    int segment_total;
    double normalized_value;
    double result;

    if (shape <= 0.0 || scale <= 0.0)
    {
        return (0.0);
    }
    if (value <= 0.0)
    {
        return (0.0);
    }
    context.shape = shape;
    context.scale = scale;
    normalized_value = value / scale;
    if (normalized_value < 0.0)
        normalized_value = 0.0;
    segment_total = static_cast<int>(normalized_value * 128.0) + 256;
    segment_total = rng_adjust_segment_total(segment_total);
    integral = rng_simpson_integral(0.0, value, segment_total, rng_gamma_pdf_integrand, &context);
    result = integral;
    if (result < 0.0)
        result = 0.0;
    if (result > 1.0)
        result = 1.0;
    return (result);
}

double rng_beta_pdf(double alpha, double beta, double value)
{
    double log_denominator;
    double log_component;
    double result;
    double epsilon;

    if (alpha <= 0.0 || beta <= 0.0)
    {
        return (0.0);
    }
    epsilon = 0.000000000001;
    if (value <= 0.0)
    {
        if (alpha < 1.0)
        {
            return (std::numeric_limits<double>::infinity());
        }
        if (std::fabs(alpha - 1.0) <= epsilon)
        {
            return (beta);
        }
        return (0.0);
    }
    if (value >= 1.0)
    {
        if (beta < 1.0)
        {
            return (std::numeric_limits<double>::infinity());
        }
        if (std::fabs(beta - 1.0) <= epsilon)
        {
            return (alpha);
        }
        return (0.0);
    }
    log_denominator = std::lgamma(alpha) + std::lgamma(beta) - std::lgamma(alpha + beta);
    log_component = (alpha - 1.0) * std::log(value) + (beta - 1.0) * std::log(1.0 - value);
    result = std::exp(log_component - log_denominator);
    if (std::isfinite(result) == false)
    {
        return (0.0);
    }
    return (result);
}

double rng_beta_cdf(double alpha, double beta, double value)
{
    rng_beta_context context;
    double integral;
    int segment_total;
    double range_end;
    double result;

    if (alpha <= 0.0 || beta <= 0.0)
    {
        return (0.0);
    }
    if (value <= 0.0)
    {
        return (0.0);
    }
    if (value >= 1.0)
    {
        return (1.0);
    }
    context.alpha = alpha;
    context.beta = beta;
    segment_total = static_cast<int>((alpha + beta) * 64.0) + 256;
    segment_total = rng_adjust_segment_total(segment_total);
    range_end = value;
    integral = rng_simpson_integral(0.0, range_end, segment_total, rng_beta_pdf_integrand, &context);
    result = integral;
    if (result < 0.0)
        result = 0.0;
    if (result > 1.0)
        result = 1.0;
    return (result);
}

double rng_chi_squared_pdf(double degrees_of_freedom, double value)
{
    double result;

    if (degrees_of_freedom <= 0.0)
    {
        return (0.0);
    }
    result = rng_gamma_pdf(degrees_of_freedom * 0.5, 2.0, value);
    return (result);
}

double rng_chi_squared_cdf(double degrees_of_freedom, double value)
{
    double result;

    if (degrees_of_freedom <= 0.0)
    {
        return (0.0);
    }
    result = rng_gamma_cdf(degrees_of_freedom * 0.5, 2.0, value);
    return (result);
}
