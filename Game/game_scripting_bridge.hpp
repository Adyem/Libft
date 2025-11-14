#ifndef GAME_SCRIPTING_BRIDGE_HPP
# define GAME_SCRIPTING_BRIDGE_HPP

#include "game_state.hpp"
#include "game_world.hpp"
#include "../Template/function.hpp"
#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "../Template/shared_ptr.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../PThread/mutex.hpp"

class ft_game_script_context
{
    private:
        ft_game_state                         *_state;
        ft_sharedptr<ft_world>                _world;
        ft_map<ft_string, ft_string>          _variables;
        mutable int                            _error_code;

        void set_error(int error) const noexcept;

    public:
        ft_game_script_context() noexcept;
        ft_game_script_context(ft_game_state *state, const ft_sharedptr<ft_world> &world) noexcept;
        ~ft_game_script_context() noexcept;
        ft_game_script_context(const ft_game_script_context &other) noexcept;
        ft_game_script_context &operator=(const ft_game_script_context &other) noexcept;

        ft_game_state *get_state() const noexcept;
        const ft_sharedptr<ft_world> &get_world() const noexcept;

        void set_state(ft_game_state *state) noexcept;
        void set_world(const ft_sharedptr<ft_world> &world) noexcept;

        void set_variable(const ft_string &key, const ft_string &value) noexcept;
        const ft_string *get_variable(const ft_string &key) const noexcept;
        void remove_variable(const ft_string &key) noexcept;
        void clear_variables() noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

class ft_game_script_bridge
{
    private:
        ft_sharedptr<ft_world> _world;
        ft_map<ft_string, ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> > _callbacks;
        ft_string _language;
        int _max_operations;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error) const noexcept;
        static bool is_supported_language(const ft_string &language) noexcept;
        int execute_line(ft_game_script_context &context, const ft_string &line) noexcept;
        int handle_call(ft_game_script_context &context, const ft_vector<ft_string> &tokens) noexcept;
        int handle_set(ft_game_script_context &context, const ft_vector<ft_string> &tokens) noexcept;
        int handle_unset(ft_game_script_context &context, const ft_vector<ft_string> &tokens) noexcept;
        void tokenize_line(const ft_string &line, ft_vector<ft_string> &tokens) const noexcept;

    public:
        ft_game_script_bridge(const ft_sharedptr<ft_world> &world, const char *language = "lua") noexcept;
        ~ft_game_script_bridge() noexcept;
        ft_game_script_bridge(const ft_game_script_bridge &other) noexcept;
        ft_game_script_bridge &operator=(const ft_game_script_bridge &other) noexcept;
        ft_game_script_bridge(ft_game_script_bridge &&other) noexcept;
        ft_game_script_bridge &operator=(ft_game_script_bridge &&other) noexcept;

        void set_language(const char *language) noexcept;
        const ft_string &get_language() const noexcept;

        void set_max_operations(int limit) noexcept;
        int get_max_operations() const noexcept;

        int register_function(const ft_string &name, const ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> &callback) noexcept;
        int remove_function(const ft_string &name) noexcept;

        int execute(const ft_string &script, ft_game_state &state) noexcept;

        int check_sandbox_capabilities(const ft_string &script, ft_vector<ft_string> &violations) noexcept;
        int validate_dry_run(const ft_string &script, ft_vector<ft_string> &warnings) noexcept;
        int inspect_bytecode_budget(const ft_string &script, int &required_operations) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
