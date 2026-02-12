#include "game_character.hpp"
#include "../Observability/observability_game_metrics.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

void ft_character::emit_game_metric(const char *event_name, const char *attribute,
    long long delta_value, long long total_value, const char *unit) const noexcept
{
    ft_game_observability_sample sample;

    if (event_name == ft_nullptr || attribute == ft_nullptr || unit == ft_nullptr)
        return ;
    if (delta_value == 0)
        return ;
    sample.labels.event_name = event_name;
    sample.labels.entity = "ft_character";
    sample.labels.attribute = attribute;
    sample.delta_value = delta_value;
    sample.total_value = total_value;
    sample.unit = unit;
    sample.error_code = FT_ERR_SUCCESS;
    sample.error_tag = ft_nullptr;
    sample.success = true;
    observability_game_metrics_record(sample);
    return ;
}
