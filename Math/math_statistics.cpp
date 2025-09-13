#include "math.hpp"

static void copy_array(double *destination, const double *source, int array_size)
{
    int array_index;

    array_index = 0;
    while (array_index < array_size)
    {
        destination[array_index] = source[array_index];
        array_index++;
    }
    return ;
}

static void sort_array(double *array, int array_size)
{
    int outer_index;
    int inner_index;
    double temporary_value;

    outer_index = 0;
    while (outer_index < array_size - 1)
    {
        inner_index = 0;
        while (inner_index < array_size - outer_index - 1)
        {
            if (array[inner_index] > array[inner_index + 1])
            {
                temporary_value = array[inner_index];
                array[inner_index] = array[inner_index + 1];
                array[inner_index + 1] = temporary_value;
            }
            inner_index++;
        }
        outer_index++;
    }
    return ;
}

double ft_mean(const double *values, int array_size)
{
    int array_index;
    double sum;

    if (array_size <= 0)
        return (0.0);
    array_index = 0;
    sum = 0.0;
    while (array_index < array_size)
    {
        sum += values[array_index];
        array_index++;
    }
    return (sum / array_size);
}

double ft_median(const double *values, int array_size)
{
    double *sorted_values;
    double median_value;
    double temporary_array[2];

    if (array_size <= 0)
        return (0.0);
    sorted_values = new double[array_size];
    copy_array(sorted_values, values, array_size);
    sort_array(sorted_values, array_size);
    if (array_size % 2 == 1)
        median_value = sorted_values[array_size / 2];
    else
    {
        temporary_array[0] = sorted_values[array_size / 2 - 1];
        temporary_array[1] = sorted_values[array_size / 2];
        median_value = ft_mean(temporary_array, 2);
    }
    delete [] sorted_values;
    return (median_value);
}

double ft_mode(const double *values, int array_size)
{
    double *sorted_values;
    int array_index;
    int current_count;
    int best_count;
    double mode_value;
    double epsilon;

    if (array_size <= 0)
        return (0.0);
    sorted_values = new double[array_size];
    copy_array(sorted_values, values, array_size);
    sort_array(sorted_values, array_size);
    array_index = 1;
    current_count = 1;
    best_count = 1;
    mode_value = sorted_values[0];
    epsilon = 0.000001;
    while (array_index < array_size)
    {
        if (math_absdiff(sorted_values[array_index],
                          sorted_values[array_index - 1]) <= epsilon)
        {
            current_count++;
            if (current_count > best_count)
            {
                best_count = current_count;
                mode_value = sorted_values[array_index];
            }
        }
        else
            current_count = 1;
        array_index++;
    }
    delete [] sorted_values;
    return (mode_value);
}

double ft_variance(const double *values, int array_size)
{
    int array_index;
    double mean_value;
    double sum;
    double difference;

    if (array_size <= 0)
        return (0.0);
    mean_value = ft_mean(values, array_size);
    array_index = 0;
    sum = 0.0;
    while (array_index < array_size)
    {
        difference = values[array_index] - mean_value;
        sum += difference * difference;
        array_index++;
    }
    return (sum / array_size);
}

double ft_stddev(const double *values, int array_size)
{
    double variance_value;

    variance_value = ft_variance(values, array_size);
    return (math_sqrt(variance_value));
}
