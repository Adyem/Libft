#ifndef GAME_CHARACTER_HPP
# define GAME_CHARACTER_HPP

#include "../Template/map.hpp"
#include "game_quest.hpp"
#include "game_achievement.hpp"
#include "game_reputation.hpp"
#include "game_buff.hpp"
#include "game_debuff.hpp"
#include "game_skill.hpp"
#include "game_upgrade.hpp"
#include "game_inventory.hpp"
#include "game_equipment.hpp"
#include "game_experience_table.hpp"
#include <cstdint>

#define FT_DAMAGE_PHYSICAL 0
#define FT_DAMAGE_MAGICAL 1
#define FT_ARMOR_POINT_REDUCTION 0.95
#define FT_DAMAGE_RULE_FLAT 0
#define FT_DAMAGE_RULE_SCALED 1
#define FT_DAMAGE_RULE_BUFFER 2
#define FT_DAMAGE_RULE_MAGIC_SHIELD 3

#ifndef FT_PHYSICAL_DAMAGE_REDUCTION
# define FT_PHYSICAL_DAMAGE_REDUCTION FT_DAMAGE_RULE_FLAT
#endif
#ifndef FT_MAGIC_DAMAGE_REDUCTION
# define FT_MAGIC_DAMAGE_REDUCTION FT_DAMAGE_RULE_FLAT
#endif

struct json_group;

struct ft_resistance
{
    int dr_percent;
    int dr_flat;
};

class ft_character
{
    protected:
        int _hit_points;
        int _physical_armor;
        int _magic_armor;
        int _current_physical_armor;
        int _current_magic_armor;
        double _physical_damage_multiplier;
        double _magic_damage_multiplier;
        uint8_t _damage_rule;
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

        ft_experience_table        _experience_table;
        ft_resistance             _fire_res;
        ft_resistance             _frost_res;
        ft_resistance             _lightning_res;
        ft_resistance             _air_res;
        ft_resistance             _earth_res;
        ft_resistance             _chaos_res;
        ft_resistance             _physical_res;
        ft_map<int, ft_skill>     _skills;
        ft_map<int, ft_buff>      _buffs;
        ft_map<int, ft_debuff>     _debuffs;
        ft_map<int, ft_upgrade> _upgrades;
        ft_map<int, ft_quest>     _quests;
        ft_map<int, ft_achievement> _achievements;
        ft_reputation             _reputation;
        ft_inventory            _inventory;
        ft_equipment            _equipment;
        mutable int               _error;

        void    set_error(int err) const noexcept;
        void    apply_modifier(const ft_item_modifier &mod, int sign) noexcept;
        long long apply_skill_modifiers(long long damage) const noexcept;

    public:
        ft_character() noexcept;
        virtual ~ft_character() noexcept;
        ft_character(const ft_character &other) noexcept;
        ft_character &operator=(const ft_character &other) noexcept;
        ft_character(ft_character &&other) noexcept;
        ft_character &operator=(ft_character &&other) noexcept;

        int get_hit_points() const noexcept;
        void set_hit_points(int hp) noexcept;

        bool is_alive() const noexcept;

        int get_physical_armor() const noexcept;
        void set_physical_armor(int armor) noexcept;
        int get_magic_armor() const noexcept;
        void set_magic_armor(int armor) noexcept;
        int get_current_physical_armor() const noexcept;
        void set_current_physical_armor(int armor) noexcept;
        int get_current_magic_armor() const noexcept;
        void set_current_magic_armor(int armor) noexcept;
        void restore_physical_armor() noexcept;
        void restore_magic_armor() noexcept;
        void restore_armor() noexcept;
        void set_damage_rule(uint8_t rule) noexcept;
        uint8_t get_damage_rule() const noexcept;
        void take_damage(long long damage, uint8_t type) noexcept;
        void take_damage_flat(long long damage, uint8_t type) noexcept;
        void take_damage_scaled(long long damage, uint8_t type) noexcept;
        void take_damage_buffer(long long damage, uint8_t type) noexcept;
        void take_damage_magic_shield(long long damage, uint8_t type) noexcept;

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
        ft_map<int, ft_skill>       &get_skills() noexcept;
        const ft_map<int, ft_skill> &get_skills() const noexcept;
        ft_skill *get_skill(int id) noexcept;
        const ft_skill *get_skill(int id) const noexcept;
        int add_skill(const ft_skill &skill) noexcept;
        void remove_skill(int id) noexcept;

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

        int equip_item(int slot, const ft_sharedptr<ft_item> &item) noexcept;
        void unequip_item(int slot) noexcept;
        ft_sharedptr<ft_item> get_equipped_item(int slot) noexcept;
        ft_sharedptr<ft_item> get_equipped_item(int slot) const noexcept;

        int get_level() const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

json_group *serialize_character(const ft_character &character);
int deserialize_character(ft_character &character, json_group *group);

#endif
