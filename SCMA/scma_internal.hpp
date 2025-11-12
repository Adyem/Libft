#ifndef SCMA_INTERNAL_HPP
# define SCMA_INTERNAL_HPP

#include <cstddef>
#include "SCMA.hpp"

struct scma_block
{
    ft_size_t    offset;
    ft_size_t    size;
    int          in_use;
    ft_size_t    generation;
};

struct scma_block_span
{
    scma_block    *data;
    ft_size_t     count;
};

struct scma_live_snapshot
{
    unsigned char    *data;
    ft_size_t        size;
    scma_handle      handle;
    int              active;
};

unsigned char    *&scma_heap_data_ref(void);
ft_size_t        &scma_heap_capacity_ref(void);
scma_block       *&scma_blocks_data_ref(void);
ft_size_t        &scma_block_capacity_ref(void);
ft_size_t        &scma_block_count_ref(void);
ft_size_t        &scma_used_size_ref(void);
int              &scma_initialized_ref(void);
scma_live_snapshot    &scma_live_snapshot_ref(void);

scma_block_span    scma_get_block_span(void);
unsigned char    *scma_get_heap_data(void);

scma_handle    scma_invalid_handle(void);
int     scma_handle_is_invalid(scma_handle handle);
void    scma_reset_live_snapshot(void);
void    scma_track_live_snapshot(scma_handle handle, unsigned char *data, ft_size_t size, int active);
void    scma_update_tracked_snapshot(scma_handle handle, ft_size_t offset, const void *source, ft_size_t size);
void    scma_compact(void);
int     scma_validate_handle(scma_handle handle, scma_block **out_block);
int     scma_ensure_block_capacity(ft_size_t required_count);
int     scma_ensure_capacity(ft_size_t required_size);
ft_size_t    scma_next_generation(ft_size_t generation);
int     scma_unlock_and_return_int(int value);
ft_size_t    scma_unlock_and_return_size(ft_size_t value);
scma_handle    scma_unlock_and_return_handle(scma_handle value);
void    *scma_unlock_and_return_pointer(void *value);
void    scma_unlock_and_return_void(void);

#endif
