#ifndef OBSERVABILITY_GAME_METRICS_HPP
#define OBSERVABILITY_GAME_METRICS_HPP

struct ft_game_observability_labels
{
    const char *event_name;
    const char *entity;
    const char *attribute;
};

struct ft_game_observability_sample
{
    ft_game_observability_labels labels;
    long long delta_value;
    long long total_value;
    const char *unit;
    int error_code;
    const char *error_tag;
    bool success;
};

typedef void (*ft_game_observability_exporter)(const ft_game_observability_sample &sample);

int observability_game_metrics_initialize(ft_game_observability_exporter exporter);
int observability_game_metrics_shutdown(void);
int observability_game_metrics_get_error(void);
const char *observability_game_metrics_get_error_str(void);
void observability_game_metrics_record(const ft_game_observability_sample &sample);

#endif
