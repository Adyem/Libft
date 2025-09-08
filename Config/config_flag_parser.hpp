#ifndef CONFIG_FLAG_PARSER_HPP
#define CONFIG_FLAG_PARSER_HPP

#include <cstddef>

class cnfg_flag_parser
{
    private:
        char    *_short_flags;
        char   **_long_flags;
        size_t  _short_flag_count;
        size_t  _long_flag_count;
        mutable int _error_code;
        void    free_flags();
        void    set_error(int error_code);

    public:
        cnfg_flag_parser();
        cnfg_flag_parser(int argument_count, char **argument_values);
        ~cnfg_flag_parser();
        bool    parse(int argument_count, char **argument_values);
        bool    has_short_flag(char flag);
        bool    has_long_flag(const char *flag);
        size_t  get_short_flag_count();
        size_t  get_long_flag_count();
        size_t  get_total_flag_count();
        int     get_error() const;
        const char  *get_error_str() const;
};

#endif
