#include "observability_histogram.hpp"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <inttypes.h>
#include "../Basic/class_nullptr.hpp"

static int32_t observability_histogram_export_emit(
    ft_observability_histogram_exporter exporter, void *user_data,
    const char *chunk) noexcept
{
    if (exporter == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    exporter(chunk, user_data);
    return (FT_ERR_SUCCESS);
}

static int32_t observability_histogram_export_emit_cstring(
    ft_observability_histogram_exporter exporter, void *user_data,
    const char *string) noexcept
{
    if (string == ft_nullptr)
        return (observability_histogram_export_emit(exporter, user_data, ""));
    return (observability_histogram_export_emit(exporter, user_data, string));
}

static int32_t observability_histogram_export_emit_number(
    ft_observability_histogram_exporter exporter, void *user_data,
    double value) noexcept
{
    char number_buffer[64];
    int32_t number_length;

    number_length = std::snprintf(number_buffer, sizeof(number_buffer), "%.17g", value);
    if (number_length < 0)
        return (FT_ERR_SYSTEM);
    if (static_cast<size_t>(number_length) >= sizeof(number_buffer))
        return (FT_ERR_OUT_OF_RANGE);
    return (observability_histogram_export_emit(exporter, user_data, number_buffer));
}

static int32_t observability_histogram_export_emit_uint64(
    ft_observability_histogram_exporter exporter, void *user_data,
    uint64_t value) noexcept
{
    char number_buffer[32];
    int32_t number_length;

    number_length = std::snprintf(number_buffer, sizeof(number_buffer), "%" PRIu64,
            static_cast<uint64_t>(value));
    if (number_length < 0)
        return (FT_ERR_SYSTEM);
    if (static_cast<size_t>(number_length) >= sizeof(number_buffer))
        return (FT_ERR_OUT_OF_RANGE);
    return (observability_histogram_export_emit(exporter, user_data, number_buffer));
}

static int32_t observability_histogram_export_json_string(
    ft_observability_histogram_exporter exporter, void *user_data,
    const char *string) noexcept
{
    int32_t error_code;
    ft_size_t index;

    error_code = observability_histogram_export_emit(exporter, user_data, "\"");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (string == ft_nullptr)
        return (observability_histogram_export_emit(exporter, user_data, "\""));
    index = 0;
    while (string[index] != '\0')
    {
        if (string[index] == '"' || string[index] == '\\')
        {
            const char *escape_string;

            escape_string = "\\";
            error_code = observability_histogram_export_emit(exporter, user_data, "\\");
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
            if (string[index] == '"')
                escape_string = "\"";
            error_code = observability_histogram_export_emit(exporter, user_data,
                    escape_string);
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        else if (string[index] == '\b')
        {
            error_code = observability_histogram_export_emit(exporter, user_data, "\\b");
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        else if (string[index] == '\f')
        {
            error_code = observability_histogram_export_emit(exporter, user_data, "\\f");
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        else if (string[index] == '\n')
        {
            error_code = observability_histogram_export_emit(exporter, user_data, "\\n");
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        else if (string[index] == '\r')
        {
            error_code = observability_histogram_export_emit(exporter, user_data, "\\r");
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        else if (string[index] == '\t')
        {
            error_code = observability_histogram_export_emit(exporter, user_data, "\\t");
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        else
        {
            char character_buffer[2];

            character_buffer[0] = string[index];
            character_buffer[1] = '\0';
            error_code = observability_histogram_export_emit(exporter, user_data,
                    character_buffer);
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        index += 1;
    }
    return (observability_histogram_export_emit(exporter, user_data, "\""));
}

int32_t observability_histogram_initialize(ft_observability_histogram *histogram,
    const char *name, const char *unit,
    ft_observability_histogram_bucket *buckets, ft_size_t bucket_count) noexcept
{
    ft_size_t index;

    if (histogram == ft_nullptr || buckets == ft_nullptr || bucket_count == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    index = 0;
    while (index < bucket_count)
    {
        if (index > 0 && buckets[index].upper_bound < buckets[index - 1].upper_bound)
            return (FT_ERR_INVALID_ARGUMENT);
        buckets[index].count = 0;
        index += 1;
    }
    histogram->name = name;
    histogram->unit = unit;
    histogram->buckets = buckets;
    histogram->bucket_count = bucket_count;
    histogram->sum = 0.0;
    histogram->count = 0;
    return (FT_ERR_SUCCESS);
}

void observability_histogram_clear(ft_observability_histogram *histogram) noexcept
{
    ft_size_t index;

    if (histogram == ft_nullptr || histogram->buckets == ft_nullptr)
        return ;
    histogram->sum = 0.0;
    histogram->count = 0;
    index = 0;
    while (index < histogram->bucket_count)
    {
        histogram->buckets[index].count = 0;
        index += 1;
    }
    return ;
}

int32_t observability_histogram_record(ft_observability_histogram *histogram,
    double value) noexcept
{
    ft_size_t bucket_index;
    ft_size_t index;

    if (histogram == ft_nullptr || histogram->buckets == ft_nullptr
        || histogram->bucket_count == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    if (std::isnan(value))
        return (FT_ERR_INVALID_ARGUMENT);
    bucket_index = histogram->bucket_count - 1;
    index = 0;
    while (index < histogram->bucket_count)
    {
        if (value <= histogram->buckets[index].upper_bound)
        {
            bucket_index = index;
            break ;
        }
        index += 1;
    }
    index = bucket_index;
    while (index < histogram->bucket_count)
    {
        histogram->buckets[index].count += 1;
        index += 1;
    }
    histogram->sum += value;
    histogram->count += 1;
    return (FT_ERR_SUCCESS);
}

int32_t observability_histogram_export_text(
    const ft_observability_histogram *histogram,
    ft_observability_histogram_exporter exporter, void *user_data) noexcept
{
    ft_size_t index;
    int32_t error_code;

    if (histogram == ft_nullptr || histogram->buckets == ft_nullptr
        || histogram->bucket_count == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    error_code = observability_histogram_export_emit(exporter, user_data, "name=");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit_cstring(exporter, user_data,
            histogram->name);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit(exporter, user_data, " unit=");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit_cstring(exporter, user_data,
            histogram->unit);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit(exporter, user_data, " count=");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit_uint64(exporter, user_data,
            histogram->count);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit(exporter, user_data, " sum=");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit_number(exporter, user_data,
            histogram->sum);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit(exporter, user_data,
            " buckets=[");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    index = 0;
    while (index < histogram->bucket_count)
    {
        if (index > 0)
        {
            error_code = observability_histogram_export_emit(exporter, user_data, ",");
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        error_code = observability_histogram_export_emit(exporter, user_data, "{le=");
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = observability_histogram_export_emit_number(exporter, user_data,
                histogram->buckets[index].upper_bound);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = observability_histogram_export_emit(exporter, user_data, ",count=");
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = observability_histogram_export_emit_uint64(exporter, user_data,
                histogram->buckets[index].count);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = observability_histogram_export_emit(exporter, user_data, "}");
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        index += 1;
    }
    error_code = observability_histogram_export_emit(exporter, user_data, "]");
    return (error_code);
}

int32_t observability_histogram_export_json(
    const ft_observability_histogram *histogram,
    ft_observability_histogram_exporter exporter, void *user_data) noexcept
{
    ft_size_t index;
    int32_t error_code;

    if (histogram == ft_nullptr || histogram->buckets == ft_nullptr
        || histogram->bucket_count == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    error_code = observability_histogram_export_emit(exporter, user_data, "{");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit(exporter, user_data, "\"name\":");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_json_string(exporter, user_data,
            histogram->name);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit(exporter, user_data, ",\"unit\":");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_json_string(exporter, user_data,
            histogram->unit);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit(exporter, user_data,
            ",\"count\":");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit_uint64(exporter, user_data,
            histogram->count);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit(exporter, user_data,
            ",\"sum\":");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit_number(exporter, user_data,
            histogram->sum);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = observability_histogram_export_emit(exporter, user_data,
            ",\"buckets\":[");
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    index = 0;
    while (index < histogram->bucket_count)
    {
        if (index > 0)
        {
            error_code = observability_histogram_export_emit(exporter, user_data, ",");
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        error_code = observability_histogram_export_emit(exporter, user_data, "{");
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = observability_histogram_export_emit(exporter, user_data,
                "\"upper_bound\":");
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = observability_histogram_export_emit_number(exporter, user_data,
                histogram->buckets[index].upper_bound);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = observability_histogram_export_emit(exporter, user_data,
                ",\"count\":");
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = observability_histogram_export_emit_uint64(exporter, user_data,
                histogram->buckets[index].count);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = observability_histogram_export_emit(exporter, user_data, "}");
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        index += 1;
    }
    error_code = observability_histogram_export_emit(exporter, user_data, "]}");
    return (error_code);
}
