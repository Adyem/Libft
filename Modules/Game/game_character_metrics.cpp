#include "game_character.hpp"
#include "../Observability/observability_game_metrics.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"
#include "../Template/shared_ptr.hpp"
#include "game_achievement.hpp"
#include "game_buff.hpp"
#include "game_debuff.hpp"
#include "game_quest.hpp"
#include "game_skill.hpp"
#include "game_upgrade.hpp"

void game_character::emit_game_metric(const char *event_name, const char *attribute,
    int64_t delta_value, int64_t total_value, const char *unit) const noexcept
{
    ft_game_observability_sample sample;

    if (event_name == ft_nullptr || attribute == ft_nullptr || unit == ft_nullptr)
        return ;
    if (delta_value == 0)
        return ;
    sample.labels.event_name = event_name;
    sample.labels.entity = "game_character";
    sample.labels.attribute = attribute;
    sample.delta_value = delta_value;
    sample.total_value = total_value;
    sample.unit = unit;
    sample.error_code = FT_ERR_SUCCESS;
    sample.error_tag = ft_nullptr;
    sample.success = FT_TRUE;
    observability_game_metrics_record(sample);
    return ;
}
