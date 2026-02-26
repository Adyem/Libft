#ifndef OBSERVABILITY_NETWORKING_METRICS_HPP
#define OBSERVABILITY_NETWORKING_METRICS_HPP

#include <cstddef>

struct ft_networking_observability_labels
{
    const char *component;
    const char *operation;
    const char *target;
    const char *resource;
};

struct ft_networking_observability_sample
{
    ft_networking_observability_labels labels;
    long long duration_ms;
    size_t request_bytes;
    size_t response_bytes;
    int status_code;
    int error_code;
    const char *error_tag;
    bool success;
};

typedef void (*ft_networking_observability_exporter)(const ft_networking_observability_sample &sample);

int observability_networking_metrics_initialize(ft_networking_observability_exporter exporter);
int observability_networking_metrics_shutdown(void);
void observability_networking_metrics_record(const ft_networking_observability_sample &sample);
int observability_networking_metrics_enable_thread_safety(void);
int observability_networking_metrics_disable_thread_safety(void);

#endif
