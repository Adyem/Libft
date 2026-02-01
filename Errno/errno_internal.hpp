#ifndef ERRNO_INTERNAL_HPP
# define ERRNO_INTERNAL_HPP

#include <cstdint>
#include "errno.hpp"

struct ft_error_frame
{
    int32_t             code;
    unsigned long long  op_id;
};

struct ft_error_stack
{
    ft_error_frame  frames[32];
    uint32_t        depth;
    unsigned long long        next_op_id;
    int32_t         last_error;
    unsigned long long        last_op_id;
};

struct ft_operation_error_stack
{
    int32_t     errors[20];
    unsigned long long    op_ids[20];
    uint32_t    count;
};

#endif
