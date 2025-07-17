#ifndef CHARACTER_HPP
# define CHARACTER_HPP

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
        int _x;
        int _y;
        int _z;
        ft_resistance _fire_res;
        ft_resistance _frost_res;
        ft_resistance _lightning_res;
        ft_resistance _air_res;
        ft_resistance _earth_res;
        ft_resistance _chaos_res;
        ft_resistance _physical_res;

    public:
        ft_character() noexcept;
        virtual ~ft_character() = default;

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

        int get_valor() const noexcept;
        void set_valor(int valor) noexcept;

        int get_x() const noexcept;
        void set_x(int x) noexcept;

        int get_y() const noexcept;
        void set_y(int y) noexcept;

        int get_z() const noexcept;
        void set_z(int z) noexcept;

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
};

#endif
