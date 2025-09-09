#include "flag_parser.hpp"
#include "config.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include <cstring>

cnfg_flag_parser::cnfg_flag_parser()
{
    this->_short_flags = ft_nullptr;
    this->_long_flags = ft_nullptr;
    this->_short_flag_count = 0;
    this->_long_flag_count = 0;
    this->set_error(ER_SUCCESS);
    return ;
}

cnfg_flag_parser::cnfg_flag_parser(int argument_count, char **argument_values)
{
    this->_short_flags = ft_nullptr;
    this->_long_flags = ft_nullptr;
    this->_short_flag_count = 0;
    this->_long_flag_count = 0;
    this->set_error(ER_SUCCESS);
    this->parse(argument_count, argument_values);
    return ;
}

cnfg_flag_parser::~cnfg_flag_parser()
{
    this->free_flags();
    return ;
}

void cnfg_flag_parser::free_flags()
{
    if (this->_short_flags)
        cma_free(this->_short_flags);
    if (this->_long_flags)
    {
        size_t long_flag_index = 0;
        while (this->_long_flags[long_flag_index])
        {
            cma_free(this->_long_flags[long_flag_index]);
            ++long_flag_index;
        }
        cma_free(this->_long_flags);
    }
    this->_short_flags = ft_nullptr;
    this->_long_flags = ft_nullptr;
    this->_short_flag_count = 0;
    this->_long_flag_count = 0;
    return ;
}

bool cnfg_flag_parser::parse(int argument_count, char **argument_values)
{
    this->free_flags();
    this->set_error(ER_SUCCESS);
    this->_short_flags = cnfg_parse_flags(argument_count, argument_values);
    if (ft_errno != ER_SUCCESS)
    {
        this->set_error(ft_errno);
        this->free_flags();
        return (false);
    }
    this->_long_flags = cnfg_parse_long_flags(argument_count, argument_values);
    if (ft_errno != ER_SUCCESS)
    {
        this->set_error(ft_errno);
        this->free_flags();
        return (false);
    }
    size_t flag_index = 0;
    if (this->_short_flags)
    {
        while (this->_short_flags[flag_index])
            ++flag_index;
        this->_short_flag_count = flag_index;
    }
    size_t long_flag_index = 0;
    if (this->_long_flags)
    {
        while (this->_long_flags[long_flag_index])
            ++long_flag_index;
        this->_long_flag_count = long_flag_index;
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

bool cnfg_flag_parser::has_short_flag(char flag)
{
    if (!this->_short_flags)
        return (false);
    if (std::strchr(this->_short_flags, flag))
        return (true);
    return (false);
}

bool cnfg_flag_parser::has_long_flag(const char *flag)
{
    if (!this->_long_flags || !flag)
        return (false);
    size_t flag_index = 0;
    while (this->_long_flags[flag_index])
    {
        if (std::strcmp(this->_long_flags[flag_index], flag) == 0)
            return (true);
        ++flag_index;
    }
    return (false);
}

size_t cnfg_flag_parser::get_short_flag_count()
{
    return (this->_short_flag_count);
}

size_t cnfg_flag_parser::get_long_flag_count()
{
    return (this->_long_flag_count);
}

size_t cnfg_flag_parser::get_total_flag_count()
{
    return (this->_short_flag_count + this->_long_flag_count);
}

void    cnfg_flag_parser::set_error(int error_code)
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int     cnfg_flag_parser::get_error() const
{
    return (this->_error_code);
}

const char  *cnfg_flag_parser::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}
