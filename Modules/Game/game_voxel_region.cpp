#include "game_voxel_region.hpp"
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno_internal.hpp"
#include "../File/file_utils.hpp"
#include "../Printf/printf.hpp"
#include <cstdio>
#include <new>

#define GAME_VOXEL_REGION_MAGIC 0x474D5247U
#define GAME_VOXEL_REGION_VERSION 1U

thread_local int32_t game_voxel_region::_last_error = FT_ERR_SUCCESS;

typedef struct s_game_voxel_region_table_entry
{
    uint64_t offset;
    uint32_t size;
}   t_game_voxel_region_table_entry;

static int32_t game_voxel_floor_mod(int32_t value, int32_t divisor) noexcept
{
    int32_t result;

    result = value % divisor;
    if (result < 0)
        result += divisor;
    return (result);
}

int32_t game_voxel_region_start_coordinate(int32_t coordinate) noexcept
{
    int32_t remainder;

    remainder = game_voxel_floor_mod(coordinate,
        GAME_VOXEL_REGION_BLOCK_WIDTH);
    return (coordinate - remainder);
}

static int32_t game_voxel_append_signed_coordinate(ft_string &target,
    int32_t coordinate) noexcept
{
    char number_buffer[32];
    int32_t absolute_value;

    if (coordinate < 0)
    {
        if (target.append('N') != FT_ERR_SUCCESS)
            return (target.get_error());
        absolute_value = -coordinate;
    }
    else
    {
        if (target.append('P') != FT_ERR_SUCCESS)
            return (target.get_error());
        absolute_value = coordinate;
    }
    if (pf_snprintf(number_buffer, sizeof(number_buffer), "%d",
            absolute_value) < 0)
        return (FT_ERR_INVALID_ARGUMENT);
    if (target.append(number_buffer) != FT_ERR_SUCCESS)
        return (target.get_error());
    return (FT_ERR_SUCCESS);
}

int32_t game_voxel_region_file_name_from_start(int32_t region_start_x,
    int32_t region_start_z, ft_string &file_name) noexcept
{
    int32_t error_code;

    if (file_name.is_initialised() == FT_FALSE)
    {
        error_code = file_name.initialize();
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
    }
    file_name.clear();
    if (file_name.append("region_") != FT_ERR_SUCCESS)
        return (file_name.get_error());
    error_code = game_voxel_append_signed_coordinate(file_name,
        region_start_x);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = game_voxel_append_signed_coordinate(file_name,
        region_start_z);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (file_name.append(".dat") != FT_ERR_SUCCESS)
        return (file_name.get_error());
    return (FT_ERR_SUCCESS);
}

int32_t game_voxel_region_file_name(int32_t world_x, int32_t world_z,
    ft_string &file_name) noexcept
{
    return (game_voxel_region_file_name_from_start(
        game_voxel_region_start_coordinate(world_x),
        game_voxel_region_start_coordinate(world_z), file_name));
}

static int32_t game_voxel_write_file(FILE *file, const void *data,
    ft_size_t size) noexcept
{
    if (size == 0)
        return (FT_ERR_SUCCESS);
    if (std::fwrite(data, 1, static_cast<size_t>(size), file)
        != static_cast<size_t>(size))
        return (FT_ERR_IO);
    return (FT_ERR_SUCCESS);
}

static int32_t game_voxel_read_file(FILE *file, void *data,
    ft_size_t size) noexcept
{
    if (size == 0)
        return (FT_ERR_SUCCESS);
    if (std::fread(data, 1, static_cast<size_t>(size), file)
        != static_cast<size_t>(size))
        return (FT_ERR_IO);
    return (FT_ERR_SUCCESS);
}

int32_t game_voxel_region::set_error(int32_t error_code) noexcept
{
    game_voxel_region::_last_error = error_code;
    return (error_code);
}

game_voxel_region::game_voxel_region() noexcept
    : _chunks(), _storage_path(), _region_start_x(0), _region_start_z(0),
    _dirty(FT_FALSE), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    uint16_t slot;

    slot = 0;
    while (slot < GAME_VOXEL_REGION_CHUNK_COUNT)
    {
        this->_chunks[slot] = ft_nullptr;
        slot += 1;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_voxel_region::~game_voxel_region() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t game_voxel_region::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state,
            "game_voxel_region::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_storage_path.is_initialised() == FT_FALSE
        && this->_storage_path.initialize() != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (this->set_error(FT_ERR_NO_MEMORY));
    }
    this->_region_start_x = 0;
    this->_region_start_z = 0;
    this->_dirty = FT_FALSE;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (this->set_error(FT_ERR_SUCCESS));
}

int32_t game_voxel_region::initialize(int32_t world_x, int32_t world_z,
    const char *storage_path) noexcept
{
    int32_t error_code;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        error_code = this->destroy();
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
    }
    error_code = this->initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = this->set_region_storage_path(storage_path);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    this->_region_start_x = game_voxel_region_start_coordinate(world_x);
    this->_region_start_z = game_voxel_region_start_coordinate(world_z);
    return (this->set_error(FT_ERR_SUCCESS));
}

void game_voxel_region::clear_chunks() noexcept
{
    uint16_t slot;

    slot = 0;
    while (slot < GAME_VOXEL_REGION_CHUNK_COUNT)
    {
        if (this->_chunks[slot] != ft_nullptr)
        {
            (void)this->_chunks[slot]->destroy();
            delete this->_chunks[slot];
            this->_chunks[slot] = ft_nullptr;
        }
        slot += 1;
    }
    return ;
}

int32_t game_voxel_region::destroy() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->clear_chunks();
    if (this->_storage_path.is_initialised() == FT_TRUE)
        (void)this->_storage_path.clear();
    this->_region_start_x = 0;
    this->_region_start_z = 0;
    this->_dirty = FT_FALSE;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (this->set_error(FT_ERR_SUCCESS));
}

int32_t game_voxel_region::move(game_voxel_region &other) noexcept
{
    uint16_t slot;
    int32_t error_code;

    if (&other == this)
        return (this->set_error(FT_ERR_SUCCESS));
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "game_voxel_region::move", "source object is uninitialised");
        return (this->set_error(FT_ERR_INVALID_STATE));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    error_code = this->initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    this->_region_start_x = other._region_start_x;
    this->_region_start_z = other._region_start_z;
    this->_dirty = other._dirty;
    this->_storage_path = other._storage_path;
    slot = 0;
    while (slot < GAME_VOXEL_REGION_CHUNK_COUNT)
    {
        this->_chunks[slot] = other._chunks[slot];
        other._chunks[slot] = ft_nullptr;
        slot += 1;
    }
    other._dirty = FT_FALSE;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (this->set_error(FT_ERR_SUCCESS));
}

int32_t game_voxel_region::set_region_storage_path(
    const char *storage_path) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "game_voxel_region::set_region_storage_path");
    if (storage_path == ft_nullptr)
        return (this->set_error(FT_ERR_INVALID_ARGUMENT));
    if (this->_storage_path.is_initialised() == FT_FALSE
        && this->_storage_path.initialize() != FT_ERR_SUCCESS)
        return (this->set_error(FT_ERR_NO_MEMORY));
    this->_storage_path = storage_path;
    if (this->_storage_path.get_error() != FT_ERR_SUCCESS)
        return (this->set_error(this->_storage_path.get_error()));
    return (this->set_error(FT_ERR_SUCCESS));
}

uint16_t game_voxel_region::chunk_slot_from_local(int32_t local_chunk_x,
    int32_t local_chunk_z) noexcept
{
    return (static_cast<uint16_t>(local_chunk_x
        + (local_chunk_z * GAME_VOXEL_REGION_CHUNKS_X)));
}

int32_t game_voxel_region::build_file_path(ft_string &file_path) const noexcept
{
    ft_string file_name;
    int32_t error_code;

    error_code = file_name.initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = game_voxel_region_file_name_from_start(
        this->_region_start_x, this->_region_start_z, file_name);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)file_name.destroy();
        return (error_code);
    }
    if (file_path.is_initialised() == FT_FALSE)
    {
        error_code = file_path.initialize();
        if (error_code != FT_ERR_SUCCESS)
        {
            (void)file_name.destroy();
            return (error_code);
        }
    }
    file_path = this->_storage_path;
    if (file_path.get_error() != FT_ERR_SUCCESS)
    {
        (void)file_name.destroy();
        return (file_path.get_error());
    }
    if (file_path.size() > 0 && file_path[file_path.size() - 1] != '/')
    {
        error_code = file_path.append('/');
        if (error_code != FT_ERR_SUCCESS)
        {
            (void)file_name.destroy();
            return (error_code);
        }
    }
    error_code = file_path.append(file_name);
    (void)file_name.destroy();
    return (error_code);
}

int32_t game_voxel_region::ensure_chunk(uint16_t slot,
    game_voxel_chunk **chunk_out) noexcept
{
    int32_t error_code;

    if (slot >= GAME_VOXEL_REGION_CHUNK_COUNT || chunk_out == ft_nullptr)
        return (this->set_error(FT_ERR_INVALID_ARGUMENT));
    if (this->_chunks[slot] == ft_nullptr)
    {
        this->_chunks[slot] = new (std::nothrow) game_voxel_chunk();
        if (this->_chunks[slot] == ft_nullptr)
            return (this->set_error(FT_ERR_NO_MEMORY));
        error_code = this->_chunks[slot]->initialize();
        if (error_code != FT_ERR_SUCCESS)
        {
            delete this->_chunks[slot];
            this->_chunks[slot] = ft_nullptr;
            return (this->set_error(error_code));
        }
    }
    *chunk_out = this->_chunks[slot];
    return (this->set_error(FT_ERR_SUCCESS));
}

int32_t game_voxel_region::load_region(int32_t world_x,
    int32_t world_z) noexcept
{
    ft_string file_path;
    FILE *file;
    uint32_t magic;
    uint32_t version;
    int32_t file_start_x;
    int32_t file_start_z;
    t_game_voxel_region_table_entry table[GAME_VOXEL_REGION_CHUNK_COUNT];
    uint16_t slot;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "game_voxel_region::load_region");
    this->clear_chunks();
    this->_region_start_x = game_voxel_region_start_coordinate(world_x);
    this->_region_start_z = game_voxel_region_start_coordinate(world_z);
    error_code = file_path.initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (this->set_error(error_code));
    error_code = this->build_file_path(file_path);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)file_path.destroy();
        return (this->set_error(error_code));
    }
    file = std::fopen(file_path.c_str(), "rb");
    (void)file_path.destroy();
    if (file == ft_nullptr)
    {
        this->_dirty = FT_FALSE;
        return (this->set_error(FT_ERR_SUCCESS));
    }
    error_code = game_voxel_read_file(file, &magic, sizeof(magic));
    if (error_code == FT_ERR_SUCCESS)
        error_code = game_voxel_read_file(file, &version, sizeof(version));
    if (error_code == FT_ERR_SUCCESS)
        error_code = game_voxel_read_file(file, &file_start_x,
            sizeof(file_start_x));
    if (error_code == FT_ERR_SUCCESS)
        error_code = game_voxel_read_file(file, &file_start_z,
            sizeof(file_start_z));
    if (error_code == FT_ERR_SUCCESS)
        error_code = game_voxel_read_file(file, table, sizeof(table));
    if (error_code != FT_ERR_SUCCESS || magic != GAME_VOXEL_REGION_MAGIC
        || version != GAME_VOXEL_REGION_VERSION
        || file_start_x != this->_region_start_x
        || file_start_z != this->_region_start_z)
    {
        (void)std::fclose(file);
        return (this->set_error(FT_ERR_IO));
    }
    slot = 0;
    while (slot < GAME_VOXEL_REGION_CHUNK_COUNT)
    {
        if (table[slot].size > 0)
        {
            ft_byte_buffer chunk_buffer;
            game_voxel_chunk *chunk;
            uint8_t *raw_chunk_data;

            raw_chunk_data = ft_nullptr;
            if (std::fseek(file, static_cast<long>(table[slot].offset),
                    SEEK_SET) != 0)
                error_code = FT_ERR_IO;
            else
                error_code = chunk_buffer.initialize(table[slot].size,
                    FT_FALSE);
            if (error_code == FT_ERR_SUCCESS)
            {
                raw_chunk_data = static_cast<uint8_t *>(cma_malloc(
                    table[slot].size));
                if (raw_chunk_data == ft_nullptr)
                    error_code = FT_ERR_NO_MEMORY;
            }
            if (error_code == FT_ERR_SUCCESS)
                error_code = game_voxel_read_file(file, raw_chunk_data,
                    table[slot].size);
            if (error_code == FT_ERR_SUCCESS)
                error_code = chunk_buffer.append(raw_chunk_data,
                    table[slot].size);
            if (error_code == FT_ERR_SUCCESS)
                error_code = this->ensure_chunk(slot, &chunk);
            if (error_code == FT_ERR_SUCCESS)
                error_code = chunk->deserialize(chunk_buffer);
            if (raw_chunk_data != ft_nullptr)
                cma_free(raw_chunk_data);
            (void)chunk_buffer.destroy();
            if (error_code != FT_ERR_SUCCESS)
            {
                (void)std::fclose(file);
                return (this->set_error(error_code));
            }
        }
        slot += 1;
    }
    (void)std::fclose(file);
    this->_dirty = FT_FALSE;
    return (this->set_error(FT_ERR_SUCCESS));
}

int32_t game_voxel_region::save_region() noexcept
{
    ft_string file_path;
    FILE *file;
    uint32_t magic;
    uint32_t version;
    t_game_voxel_region_table_entry table[GAME_VOXEL_REGION_CHUNK_COUNT];
    ft_byte_buffer chunk_buffers[GAME_VOXEL_REGION_CHUNK_COUNT];
    uint16_t slot;
    uint64_t offset;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "game_voxel_region::save_region");
    slot = 0;
    while (slot < GAME_VOXEL_REGION_CHUNK_COUNT)
    {
        table[slot].offset = 0;
        table[slot].size = 0;
        slot += 1;
    }
    offset = sizeof(magic) + sizeof(version) + sizeof(this->_region_start_x)
        + sizeof(this->_region_start_z) + sizeof(table);
    slot = 0;
    while (slot < GAME_VOXEL_REGION_CHUNK_COUNT)
    {
        if (this->_chunks[slot] != ft_nullptr)
        {
            error_code = chunk_buffers[slot].initialize();
            if (error_code != FT_ERR_SUCCESS)
                return (this->set_error(error_code));
            error_code = this->_chunks[slot]->serialize(chunk_buffers[slot]);
            if (error_code != FT_ERR_SUCCESS)
                return (this->set_error(error_code));
            table[slot].offset = offset;
            table[slot].size = static_cast<uint32_t>(chunk_buffers[slot].size());
            offset += table[slot].size;
        }
        slot += 1;
    }
    error_code = file_path.initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (this->set_error(error_code));
    error_code = this->build_file_path(file_path);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)file_path.destroy();
        return (this->set_error(error_code));
    }
    file = std::fopen(file_path.c_str(), "wb");
    (void)file_path.destroy();
    if (file == ft_nullptr)
        return (this->set_error(FT_ERR_FILE_OPEN_FAILED));
    magic = GAME_VOXEL_REGION_MAGIC;
    version = GAME_VOXEL_REGION_VERSION;
    error_code = game_voxel_write_file(file, &magic, sizeof(magic));
    if (error_code == FT_ERR_SUCCESS)
        error_code = game_voxel_write_file(file, &version, sizeof(version));
    if (error_code == FT_ERR_SUCCESS)
        error_code = game_voxel_write_file(file, &this->_region_start_x,
            sizeof(this->_region_start_x));
    if (error_code == FT_ERR_SUCCESS)
        error_code = game_voxel_write_file(file, &this->_region_start_z,
            sizeof(this->_region_start_z));
    if (error_code == FT_ERR_SUCCESS)
        error_code = game_voxel_write_file(file, table, sizeof(table));
    slot = 0;
    while (slot < GAME_VOXEL_REGION_CHUNK_COUNT
        && error_code == FT_ERR_SUCCESS)
    {
        if (table[slot].size > 0)
            error_code = game_voxel_write_file(file,
                chunk_buffers[slot].data(), chunk_buffers[slot].size());
        slot += 1;
    }
    (void)std::fclose(file);
    slot = 0;
    while (slot < GAME_VOXEL_REGION_CHUNK_COUNT)
    {
        if (chunk_buffers[slot].is_initialised() == FT_TRUE)
            (void)chunk_buffers[slot].destroy();
        if (this->_chunks[slot] != ft_nullptr)
            this->_chunks[slot]->clear_dirty();
        slot += 1;
    }
    if (error_code != FT_ERR_SUCCESS)
        return (this->set_error(error_code));
    this->_dirty = FT_FALSE;
    return (this->set_error(FT_ERR_SUCCESS));
}

int32_t game_voxel_region::load_chunk(int32_t world_chunk_x,
    int32_t world_chunk_z, game_voxel_chunk **chunk_out) noexcept
{
    int32_t local_chunk_x;
    int32_t local_chunk_z;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "game_voxel_region::load_chunk");
    if (chunk_out == ft_nullptr)
        return (this->set_error(FT_ERR_INVALID_ARGUMENT));
    local_chunk_x = world_chunk_x
        - (this->_region_start_x / GAME_VOXEL_CHUNK_WIDTH);
    local_chunk_z = world_chunk_z
        - (this->_region_start_z / GAME_VOXEL_CHUNK_DEPTH);
    if (local_chunk_x < 0 || local_chunk_x >= GAME_VOXEL_REGION_CHUNKS_X
        || local_chunk_z < 0 || local_chunk_z >= GAME_VOXEL_REGION_CHUNKS_Z)
        return (this->set_error(FT_ERR_OUT_OF_RANGE));
    return (this->ensure_chunk(this->chunk_slot_from_local(local_chunk_x,
            local_chunk_z), chunk_out));
}

int32_t game_voxel_region::save_chunk(int32_t world_chunk_x,
    int32_t world_chunk_z, const game_voxel_chunk &chunk) noexcept
{
    game_voxel_chunk *destination;
    ft_byte_buffer buffer;
    int32_t error_code;

    error_code = this->load_chunk(world_chunk_x, world_chunk_z, &destination);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = buffer.initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (this->set_error(error_code));
    error_code = chunk.serialize(buffer);
    if (error_code == FT_ERR_SUCCESS)
        error_code = destination->deserialize(buffer);
    (void)buffer.destroy();
    if (error_code != FT_ERR_SUCCESS)
        return (this->set_error(error_code));
    this->_dirty = FT_TRUE;
    return (this->set_error(FT_ERR_SUCCESS));
}

int32_t game_voxel_region::read_block(int32_t world_x, int32_t world_y,
    int32_t world_z, uint32_t *block_id) const noexcept
{
    int32_t local_x;
    int32_t local_z;
    int32_t chunk_x;
    int32_t chunk_z;
    uint16_t slot;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "game_voxel_region::read_block");
    if (block_id == ft_nullptr)
        return (game_voxel_region::set_error(FT_ERR_INVALID_ARGUMENT));
    if (world_y < 0 || world_y >= GAME_VOXEL_CHUNK_HEIGHT)
        return (game_voxel_region::set_error(FT_ERR_OUT_OF_RANGE));
    local_x = world_x - this->_region_start_x;
    local_z = world_z - this->_region_start_z;
    if (local_x < 0 || local_x >= GAME_VOXEL_REGION_BLOCK_WIDTH
        || local_z < 0 || local_z >= GAME_VOXEL_REGION_BLOCK_WIDTH)
        return (game_voxel_region::set_error(FT_ERR_OUT_OF_RANGE));
    chunk_x = local_x >> 4;
    chunk_z = local_z >> 4;
    slot = game_voxel_region::chunk_slot_from_local(chunk_x, chunk_z);
    if (this->_chunks[slot] == ft_nullptr)
    {
        *block_id = GAME_VOXEL_AIR_BLOCK;
        return (game_voxel_region::set_error(FT_ERR_SUCCESS));
    }
    return (this->_chunks[slot]->read_block(local_x & 15, world_y,
        local_z & 15, block_id));
}

int32_t game_voxel_region::write_block(int32_t world_x, int32_t world_y,
    int32_t world_z, uint32_t block_id) noexcept
{
    int32_t local_x;
    int32_t local_z;
    int32_t chunk_x;
    int32_t chunk_z;
    uint16_t slot;
    game_voxel_chunk *chunk;
    int32_t error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "game_voxel_region::write_block");
    if (world_y < 0 || world_y >= GAME_VOXEL_CHUNK_HEIGHT)
        return (this->set_error(FT_ERR_OUT_OF_RANGE));
    local_x = world_x - this->_region_start_x;
    local_z = world_z - this->_region_start_z;
    if (local_x < 0 || local_x >= GAME_VOXEL_REGION_BLOCK_WIDTH
        || local_z < 0 || local_z >= GAME_VOXEL_REGION_BLOCK_WIDTH)
        return (this->set_error(FT_ERR_OUT_OF_RANGE));
    chunk_x = local_x >> 4;
    chunk_z = local_z >> 4;
    slot = game_voxel_region::chunk_slot_from_local(chunk_x, chunk_z);
    if (block_id == GAME_VOXEL_AIR_BLOCK && this->_chunks[slot] == ft_nullptr)
        return (this->set_error(FT_ERR_SUCCESS));
    error_code = this->ensure_chunk(slot, &chunk);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = chunk->write_block(local_x & 15, world_y, local_z & 15,
        block_id);
    if (error_code != FT_ERR_SUCCESS)
        return (this->set_error(error_code));
    this->_dirty = FT_TRUE;
    return (this->set_error(FT_ERR_SUCCESS));
}

ft_bool game_voxel_region::has_chunk(int32_t world_chunk_x,
    int32_t world_chunk_z) const noexcept
{
    return (this->is_chunk_loaded(world_chunk_x, world_chunk_z));
}

ft_bool game_voxel_region::is_chunk_loaded(int32_t world_chunk_x,
    int32_t world_chunk_z) const noexcept
{
    int32_t local_chunk_x;
    int32_t local_chunk_z;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_FALSE);
    local_chunk_x = world_chunk_x
        - (this->_region_start_x / GAME_VOXEL_CHUNK_WIDTH);
    local_chunk_z = world_chunk_z
        - (this->_region_start_z / GAME_VOXEL_CHUNK_DEPTH);
    if (local_chunk_x < 0 || local_chunk_x >= GAME_VOXEL_REGION_CHUNKS_X
        || local_chunk_z < 0 || local_chunk_z >= GAME_VOXEL_REGION_CHUNKS_Z)
        return (FT_FALSE);
    if (this->_chunks[this->chunk_slot_from_local(local_chunk_x,
            local_chunk_z)] == ft_nullptr)
        return (FT_FALSE);
    return (FT_TRUE);
}

int32_t game_voxel_region::get_region_start_x() const noexcept
{
    return (this->_region_start_x);
}

int32_t game_voxel_region::get_region_start_z() const noexcept
{
    return (this->_region_start_z);
}

int32_t game_voxel_region::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_voxel_region::get_error");
    return (game_voxel_region::_last_error);
}

const char *game_voxel_region::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_voxel_region::get_error_str");
    return (ft_strerror(game_voxel_region::_last_error));
}
