#ifndef CHARACTER_HPP
# define CHARACTER_HPP

#include "../Template/map.hpp"
#include "quest.hpp"
#include "achievement.hpp"
#include "reputation.hpp"
#include "buff.hpp"
#include "debuff.hpp"
#include "upgrade.hpp"
#include "inventory.hpp"
#include "experience_table.hpp"

struct ft_resistance
{
    int dr_percent;
    int dr_flat;
};

class ft_character
{
    protected:
        int _hit_points;
        int _armor;
        int _might;
        int _agility;
        int _endurance;
        int _reason;
        int _insigh;
        int _presence;
        int _coins;
        int _valor;
        int _experience;
        int _x;
        int _y;
        int _z;

		ft_experience_table		_experience_table;
        ft_resistance 			_fire_res;
        ft_resistance 			_frost_res;
        ft_resistance 			_lightning_res;
        ft_resistance 			_air_res;
        ft_resistance 			_earth_res;
        ft_resistance 			_chaos_res;
        ft_resistance 			_physical_res;
        ft_map<int, ft_buff>  	_buffs;
        ft_map<int, ft_debuff>	 _debuffs;
        ft_map<int, ft_upgrade> _upgrades;
        ft_map<int, ft_quest> 	_quests;
        ft_map<int, ft_achievement> _achievements;
        ft_reputation         	_reputation;
		ft_inventory			_inventory;
        mutable int           	_error;

        void    set_error(int err) const noexcept;

    public:
        ft_character() noexcept;
        virtual ~ft_character() = default;

        int get_hit_points() const noexcept;
        void set_hit_points(int hp) noexcept;

        bool is_alive() const noexcept;

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
        void add_coins(int coins) noexcept;
        void sub_coins(int coins) noexcept;

        int get_valor() const noexcept;
        void set_valor(int valor) noexcept;
        void add_valor(int valor) noexcept;
        void sub_valor(int valor) noexcept;

        int get_experience() const noexcept;
        void set_experience(int experience) noexcept;
        void add_experience(int experience) noexcept;
        void sub_experience(int experience) noexcept;

        int get_x() const noexcept;
        void set_x(int x) noexcept;

        int get_y() const noexcept;
        void set_y(int y) noexcept;

        int get_z() const noexcept;
        void set_z(int z) noexcept;

        void move(int dx, int dy, int dz) noexcept;

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

        ft_map<int, ft_buff>       &get_buffs() noexcept;
        const ft_map<int, ft_buff> &get_buffs() const noexcept;

        ft_map<int, ft_debuff>       &get_debuffs() noexcept;
        const ft_map<int, ft_debuff> &get_debuffs() const noexcept;
        ft_map<int, ft_upgrade>       &get_upgrades() noexcept;
        const ft_map<int, ft_upgrade> &get_upgrades() const noexcept;

        ft_map<int, ft_quest>       &get_quests() noexcept;
        const ft_map<int, ft_quest> &get_quests() const noexcept;

        ft_map<int, ft_achievement>       &get_achievements() noexcept;
        const ft_map<int, ft_achievement> &get_achievements() const noexcept;

        ft_reputation       &get_reputation() noexcept;
        const ft_reputation &get_reputation() const noexcept;

        ft_experience_table       &get_experience_table() noexcept;
        const ft_experience_table &get_experience_table() const noexcept;

        int get_level() const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
