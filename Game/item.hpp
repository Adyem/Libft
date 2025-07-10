#ifndef ITEM_HPP
# define ITEM_HPP

#include <cstdint>
#include "character.hpp"

// Stat identifiers (8-bit)
# define FT_STAT_MIGHT             0x01
# define FT_STAT_AGILITY           0x02
# define FT_STAT_ENDURANCE         0x03
# define FT_STAT_REASON            0x04
# define FT_STAT_INSIGH            0x05
# define FT_STAT_PRESENCE          0x06

struct ft_item_ability
{
    uint8_t    stat_id;
    int        main_modifier;
    int        bonus_modifier;
    int        duration;
    int        duration_modifier;
    int        stat_modifier;
};

class ft_item
{
    private:
        int _hit_points;
        int _armor;
        int _might;
        int _agility;
        int _endurance;
        int _reason;
        int _insigh;
        int _presence;
        int _coins;
        ft_resistance _fire_res;
        ft_resistance _frost_res;
        ft_resistance _lightning_res;
        ft_resistance _air_res;
        ft_resistance _earth_res;
        ft_resistance _chaos_res;
        ft_resistance _physical_res;
        ft_item_ability _ability_1;
        ft_item_ability _ability_2;

    public:
        ft_item() noexcept;
        virtual ~ft_item() = default;

        int get_hit_points() const noexcept;
        void set_hit_points(int hp) noexcept;

        int get_armor() const noexcept;
        void set_armor(int armor) noexcept;

        int get_might() const noexcept;
        void set_might(int might) noexcept;

        int get_agility() const noexcept;
        void set_agility(int agility) noexcept;

        int get_endurance() const noexcept;
        void set_endurance(int endurance) noexcept;

        int get_reason() const noexcept;
        void set_reason(int reason) noexcept;

        int get_insigh() const noexcept;
        void set_insigh(int insigh) noexcept;

        int get_presence() const noexcept;
        void set_presence(int presence) noexcept;

        int get_coins() const noexcept;
        void set_coins(int coins) noexcept;

        ft_resistance get_fire_res() const noexcept;
        void set_fire_res(int percent, int flat) noexcept;

        ft_resistance get_frost_res() const noexcept;
        void set_frost_res(int percent, int flat) noexcept;

        ft_resistance get_lightning_res() const noexcept;
        void set_lightning_res(int percent, int flat) noexcept;

        ft_resistance get_air_res() const noexcept;
        void set_air_res(int percent, int flat) noexcept;

        ft_resistance get_earth_res() const noexcept;
        void set_earth_res(int percent, int flat) noexcept;

        ft_resistance get_chaos_res() const noexcept;
        void set_chaos_res(int percent, int flat) noexcept;

        ft_resistance get_physical_res() const noexcept;
        void set_physical_res(int percent, int flat) noexcept;

        ft_item_ability get_ability_1() const noexcept;
        void set_ability_1(const ft_item_ability &ability) noexcept;

        ft_item_ability get_ability_2() const noexcept;
        void set_ability_2(const ft_item_ability &ability) noexcept;

        uint8_t get_ability_1_stat_id() const noexcept;
        void set_ability_1_stat_id(uint8_t id) noexcept;

        int get_ability_1_main_modifier() const noexcept;
        void set_ability_1_main_modifier(int mod) noexcept;

        int get_ability_1_bonus_modifier() const noexcept;
        void set_ability_1_bonus_modifier(int mod) noexcept;

        int get_ability_1_duration() const noexcept;
        void set_ability_1_duration(int duration) noexcept;

        int get_ability_1_duration_modifier() const noexcept;
        void set_ability_1_duration_modifier(int mod) noexcept;

        int get_ability_1_stat_modifier() const noexcept;
        void set_ability_1_stat_modifier(int mod) noexcept;

        uint8_t get_ability_2_stat_id() const noexcept;
        void set_ability_2_stat_id(uint8_t id) noexcept;

        int get_ability_2_main_modifier() const noexcept;
        void set_ability_2_main_modifier(int mod) noexcept;

        int get_ability_2_bonus_modifier() const noexcept;
        void set_ability_2_bonus_modifier(int mod) noexcept;

        int get_ability_2_duration() const noexcept;
        void set_ability_2_duration(int duration) noexcept;

        int get_ability_2_duration_modifier() const noexcept;
        void set_ability_2_duration_modifier(int mod) noexcept;

        int get_ability_2_stat_modifier() const noexcept;
        void set_ability_2_stat_modifier(int mod) noexcept;
};
#endif // ITEM_HPP
