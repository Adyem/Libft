#include "voxel_mesh.hpp"

#ifdef GAME_USE_VOXEL_REGION_BACKEND

#include "voxel.hpp"
#include "../Errno/errno.hpp"

static void chunk_mesh_reset_bounds(chunk_mesh &mesh) noexcept
{
    mesh.bounds.minimum_x = 0;
    mesh.bounds.minimum_y = 0;
    mesh.bounds.minimum_z = 0;
    mesh.bounds.maximum_x = GAME_VOXEL_CHUNK_WIDTH;
    mesh.bounds.maximum_y = GAME_VOXEL_CHUNK_HEIGHT;
    mesh.bounds.maximum_z = GAME_VOXEL_CHUNK_DEPTH;
    return ;
}

int32_t chunk_mesh_initialize(chunk_mesh &mesh) noexcept
{
    int32_t error_code;

    error_code = mesh.vertices.initialize();
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = mesh.indices.initialize();
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)mesh.vertices.destroy();
        return (error_code);
    }
    chunk_mesh_reset_bounds(mesh);
    return (FT_ERR_SUCCESS);
}

int32_t chunk_mesh_destroy(chunk_mesh &mesh) noexcept
{
    int32_t first_error;
    int32_t error_code;

    first_error = mesh.vertices.destroy();
    error_code = mesh.indices.destroy();
    if (first_error != FT_ERR_SUCCESS)
        return (first_error);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    chunk_mesh_reset_bounds(mesh);
    return (FT_ERR_SUCCESS);
}

int32_t chunk_mesh_clear(chunk_mesh &mesh) noexcept
{
    mesh.vertices.clear();
    if (mesh.vertices.get_error() != FT_ERR_SUCCESS)
        return (mesh.vertices.get_error());
    mesh.indices.clear();
    if (mesh.indices.get_error() != FT_ERR_SUCCESS)
        return (mesh.indices.get_error());
    chunk_mesh_reset_bounds(mesh);
    return (FT_ERR_SUCCESS);
}

static int32_t chunk_mesh_read_or_air(const game_voxel_chunk &chunk,
    int32_t local_x, int32_t local_y, int32_t local_z,
    uint32_t *block_id) noexcept
{
    if (local_x < 0 || local_x >= GAME_VOXEL_CHUNK_WIDTH || local_y < 0
        || local_y >= GAME_VOXEL_CHUNK_HEIGHT || local_z < 0
        || local_z >= GAME_VOXEL_CHUNK_DEPTH)
    {
        *block_id = GAME_VOXEL_AIR_BLOCK;
        return (FT_ERR_SUCCESS);
    }
    return (chunk.read_block(local_x, local_y, local_z, block_id));
}

static int32_t chunk_mesh_face_is_visible(const game_voxel_chunk &chunk,
    int32_t local_x, int32_t local_y, int32_t local_z,
    chunk_mesh_face face, ft_bool *visible) noexcept
{
    uint32_t neighbor_block_id;
    int32_t neighbor_x;
    int32_t neighbor_y;
    int32_t neighbor_z;
    int32_t error_code;

    neighbor_x = local_x;
    neighbor_y = local_y;
    neighbor_z = local_z;
    if (face == CHUNK_MESH_FACE_WEST)
        neighbor_x -= 1;
    if (face == CHUNK_MESH_FACE_EAST)
        neighbor_x += 1;
    if (face == CHUNK_MESH_FACE_DOWN)
        neighbor_y -= 1;
    if (face == CHUNK_MESH_FACE_UP)
        neighbor_y += 1;
    if (face == CHUNK_MESH_FACE_NORTH)
        neighbor_z -= 1;
    if (face == CHUNK_MESH_FACE_SOUTH)
        neighbor_z += 1;
    error_code = chunk_mesh_read_or_air(chunk, neighbor_x, neighbor_y,
        neighbor_z, &neighbor_block_id);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (terrain_block_is_transparent(neighbor_block_id) == FT_TRUE)
        *visible = FT_TRUE;
    else
        *visible = FT_FALSE;
    return (FT_ERR_SUCCESS);
}

static int32_t chunk_mesh_count_block_faces(const game_voxel_chunk &chunk,
    int32_t local_x, int32_t local_y, int32_t local_z,
    ft_size_t *face_count) noexcept
{
    chunk_mesh_face face;
    ft_bool visible;
    int32_t error_code;

    face = CHUNK_MESH_FACE_WEST;
    while (face <= CHUNK_MESH_FACE_SOUTH)
    {
        error_code = chunk_mesh_face_is_visible(chunk, local_x, local_y,
            local_z, face, &visible);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        if (visible == FT_TRUE)
            *face_count += 1;
        face = static_cast<chunk_mesh_face>(static_cast<int32_t>(face) + 1);
    }
    return (FT_ERR_SUCCESS);
}

static int32_t chunk_mesh_count_visible_faces(const game_voxel_chunk &chunk,
    ft_size_t *face_count) noexcept
{
    int32_t local_x;
    int32_t local_y;
    int32_t local_z;
    uint32_t block_id;
    int32_t error_code;

    *face_count = 0;
    local_y = 0;
    while (local_y < GAME_VOXEL_CHUNK_HEIGHT)
    {
        local_z = 0;
        while (local_z < GAME_VOXEL_CHUNK_DEPTH)
        {
            local_x = 0;
            while (local_x < GAME_VOXEL_CHUNK_WIDTH)
            {
                error_code = chunk.read_block(local_x, local_y, local_z,
                    &block_id);
                if (error_code != FT_ERR_SUCCESS)
                    return (error_code);
                if (block_id != GAME_VOXEL_AIR_BLOCK)
                {
                    error_code = chunk_mesh_count_block_faces(chunk,
                        local_x, local_y, local_z, face_count);
                    if (error_code != FT_ERR_SUCCESS)
                        return (error_code);
                }
                local_x += 1;
            }
            local_z += 1;
        }
        local_y += 1;
    }
    return (FT_ERR_SUCCESS);
}

static void chunk_mesh_fill_vertex(chunk_mesh_vertex *vertex,
    int32_t coordinate_x, int32_t coordinate_y, int32_t coordinate_z,
    uint16_t texture_u, uint16_t texture_v, uint32_t block_id,
    chunk_mesh_face face) noexcept
{
    vertex->coordinate_x = static_cast<uint16_t>(coordinate_x);
    vertex->coordinate_y = static_cast<uint16_t>(coordinate_y);
    vertex->coordinate_z = static_cast<uint16_t>(coordinate_z);
    vertex->texture_u = texture_u;
    vertex->texture_v = texture_v;
    vertex->block_id = block_id;
    vertex->face = static_cast<uint8_t>(face);
    return ;
}

static void chunk_mesh_make_west_rectangle(chunk_mesh_vertex vertices[4],
    int32_t coordinate_x, int32_t minimum_y, int32_t minimum_z,
    int32_t maximum_y, int32_t maximum_z, uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], coordinate_x, minimum_y, minimum_z,
        0U, 0U, block_id, CHUNK_MESH_FACE_WEST);
    chunk_mesh_fill_vertex(&vertices[1], coordinate_x, maximum_y, minimum_z,
        0U, static_cast<uint16_t>(maximum_y - minimum_y), block_id,
        CHUNK_MESH_FACE_WEST);
    chunk_mesh_fill_vertex(&vertices[2], coordinate_x, maximum_y, maximum_z,
        static_cast<uint16_t>(maximum_z - minimum_z),
        static_cast<uint16_t>(maximum_y - minimum_y), block_id,
        CHUNK_MESH_FACE_WEST);
    chunk_mesh_fill_vertex(&vertices[3], coordinate_x, minimum_y, maximum_z,
        static_cast<uint16_t>(maximum_z - minimum_z), 0U, block_id,
        CHUNK_MESH_FACE_WEST);
    return ;
}

static void chunk_mesh_make_east_rectangle(chunk_mesh_vertex vertices[4],
    int32_t coordinate_x, int32_t minimum_y, int32_t minimum_z,
    int32_t maximum_y, int32_t maximum_z, uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], coordinate_x, minimum_y, maximum_z,
        0U, 0U, block_id, CHUNK_MESH_FACE_EAST);
    chunk_mesh_fill_vertex(&vertices[1], coordinate_x, maximum_y, maximum_z,
        0U, static_cast<uint16_t>(maximum_y - minimum_y), block_id,
        CHUNK_MESH_FACE_EAST);
    chunk_mesh_fill_vertex(&vertices[2], coordinate_x, maximum_y, minimum_z,
        static_cast<uint16_t>(maximum_z - minimum_z),
        static_cast<uint16_t>(maximum_y - minimum_y), block_id,
        CHUNK_MESH_FACE_EAST);
    chunk_mesh_fill_vertex(&vertices[3], coordinate_x, minimum_y, minimum_z,
        static_cast<uint16_t>(maximum_z - minimum_z), 0U, block_id,
        CHUNK_MESH_FACE_EAST);
    return ;
}

static void chunk_mesh_make_down_rectangle(chunk_mesh_vertex vertices[4],
    int32_t minimum_x, int32_t coordinate_y, int32_t minimum_z,
    int32_t maximum_x, int32_t maximum_z, uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], minimum_x, coordinate_y, maximum_z,
        0U, 0U, block_id, CHUNK_MESH_FACE_DOWN);
    chunk_mesh_fill_vertex(&vertices[1], maximum_x, coordinate_y, maximum_z,
        0U, static_cast<uint16_t>(maximum_x - minimum_x), block_id,
        CHUNK_MESH_FACE_DOWN);
    chunk_mesh_fill_vertex(&vertices[2], maximum_x, coordinate_y, minimum_z,
        static_cast<uint16_t>(maximum_z - minimum_z),
        static_cast<uint16_t>(maximum_x - minimum_x), block_id,
        CHUNK_MESH_FACE_DOWN);
    chunk_mesh_fill_vertex(&vertices[3], minimum_x, coordinate_y, minimum_z,
        static_cast<uint16_t>(maximum_z - minimum_z), 0U, block_id,
        CHUNK_MESH_FACE_DOWN);
    return ;
}

static void chunk_mesh_make_up_rectangle(chunk_mesh_vertex vertices[4],
    int32_t minimum_x, int32_t coordinate_y, int32_t minimum_z,
    int32_t maximum_x, int32_t maximum_z, uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], minimum_x, coordinate_y, minimum_z,
        0U, 0U, block_id, CHUNK_MESH_FACE_UP);
    chunk_mesh_fill_vertex(&vertices[1], maximum_x, coordinate_y, minimum_z,
        0U, static_cast<uint16_t>(maximum_x - minimum_x), block_id,
        CHUNK_MESH_FACE_UP);
    chunk_mesh_fill_vertex(&vertices[2], maximum_x, coordinate_y, maximum_z,
        static_cast<uint16_t>(maximum_z - minimum_z),
        static_cast<uint16_t>(maximum_x - minimum_x), block_id,
        CHUNK_MESH_FACE_UP);
    chunk_mesh_fill_vertex(&vertices[3], minimum_x, coordinate_y, maximum_z,
        static_cast<uint16_t>(maximum_z - minimum_z), 0U, block_id,
        CHUNK_MESH_FACE_UP);
    return ;
}

static void chunk_mesh_make_north_rectangle(chunk_mesh_vertex vertices[4],
    int32_t minimum_x, int32_t minimum_y, int32_t coordinate_z,
    int32_t maximum_x, int32_t maximum_y, uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], maximum_x, minimum_y, coordinate_z,
        0U, 0U, block_id, CHUNK_MESH_FACE_NORTH);
    chunk_mesh_fill_vertex(&vertices[1], maximum_x, maximum_y, coordinate_z,
        0U, static_cast<uint16_t>(maximum_y - minimum_y), block_id,
        CHUNK_MESH_FACE_NORTH);
    chunk_mesh_fill_vertex(&vertices[2], minimum_x, maximum_y, coordinate_z,
        static_cast<uint16_t>(maximum_x - minimum_x),
        static_cast<uint16_t>(maximum_y - minimum_y), block_id,
        CHUNK_MESH_FACE_NORTH);
    chunk_mesh_fill_vertex(&vertices[3], minimum_x, minimum_y, coordinate_z,
        static_cast<uint16_t>(maximum_x - minimum_x), 0U, block_id,
        CHUNK_MESH_FACE_NORTH);
    return ;
}

static void chunk_mesh_make_south_rectangle(chunk_mesh_vertex vertices[4],
    int32_t minimum_x, int32_t minimum_y, int32_t coordinate_z,
    int32_t maximum_x, int32_t maximum_y, uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], minimum_x, minimum_y, coordinate_z,
        0U, 0U, block_id, CHUNK_MESH_FACE_SOUTH);
    chunk_mesh_fill_vertex(&vertices[1], minimum_x, maximum_y, coordinate_z,
        0U, static_cast<uint16_t>(maximum_y - minimum_y), block_id,
        CHUNK_MESH_FACE_SOUTH);
    chunk_mesh_fill_vertex(&vertices[2], maximum_x, maximum_y, coordinate_z,
        static_cast<uint16_t>(maximum_x - minimum_x),
        static_cast<uint16_t>(maximum_y - minimum_y), block_id,
        CHUNK_MESH_FACE_SOUTH);
    chunk_mesh_fill_vertex(&vertices[3], maximum_x, minimum_y, coordinate_z,
        static_cast<uint16_t>(maximum_x - minimum_x), 0U, block_id,
        CHUNK_MESH_FACE_SOUTH);
    return ;
}

static void chunk_mesh_make_rectangle_vertices(chunk_mesh_vertex vertices[4],
    int32_t axis_value, int32_t minimum_column, int32_t minimum_row,
    int32_t maximum_column, int32_t maximum_row, uint32_t block_id,
    chunk_mesh_face face) noexcept
{
    if (face == CHUNK_MESH_FACE_WEST)
        chunk_mesh_make_west_rectangle(vertices, axis_value, minimum_row,
            minimum_column, maximum_row, maximum_column, block_id);
    if (face == CHUNK_MESH_FACE_EAST)
        chunk_mesh_make_east_rectangle(vertices, axis_value + 1, minimum_row,
            minimum_column, maximum_row, maximum_column, block_id);
    if (face == CHUNK_MESH_FACE_DOWN)
        chunk_mesh_make_down_rectangle(vertices, minimum_column, axis_value,
            minimum_row, maximum_column, maximum_row, block_id);
    if (face == CHUNK_MESH_FACE_UP)
        chunk_mesh_make_up_rectangle(vertices, minimum_column, axis_value + 1,
            minimum_row, maximum_column, maximum_row, block_id);
    if (face == CHUNK_MESH_FACE_NORTH)
        chunk_mesh_make_north_rectangle(vertices, minimum_column, minimum_row,
            axis_value, maximum_column, maximum_row, block_id);
    if (face == CHUNK_MESH_FACE_SOUTH)
        chunk_mesh_make_south_rectangle(vertices, minimum_column, minimum_row,
            axis_value + 1, maximum_column, maximum_row, block_id);
    return ;
}

static int32_t chunk_mesh_push_index(chunk_mesh &mesh,
    uint32_t index_value) noexcept
{
    return (mesh.indices.push_back(index_value));
}

static int32_t chunk_mesh_emit_rectangle(chunk_mesh &mesh, int32_t axis_value,
    int32_t minimum_column, int32_t minimum_row, int32_t maximum_column,
    int32_t maximum_row, uint32_t block_id, chunk_mesh_face face) noexcept
{
    chunk_mesh_vertex vertices[4];
    uint32_t base_vertex;
    int32_t error_code;

    base_vertex = static_cast<uint32_t>(mesh.vertices.size());
    chunk_mesh_make_rectangle_vertices(vertices, axis_value, minimum_column,
        minimum_row, maximum_column, maximum_row, block_id, face);
    error_code = mesh.vertices.push_back(vertices[0]);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = mesh.vertices.push_back(vertices[1]);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = mesh.vertices.push_back(vertices[2]);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = mesh.vertices.push_back(vertices[3]);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = chunk_mesh_push_index(mesh, base_vertex);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = chunk_mesh_push_index(mesh, base_vertex + 1U);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = chunk_mesh_push_index(mesh, base_vertex + 2U);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = chunk_mesh_push_index(mesh, base_vertex);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = chunk_mesh_push_index(mesh, base_vertex + 2U);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (chunk_mesh_push_index(mesh, base_vertex + 3U));
}

static void chunk_mesh_plane_dimensions(chunk_mesh_face face,
    int32_t *column_count, int32_t *row_count) noexcept
{
    if (face == CHUNK_MESH_FACE_WEST || face == CHUNK_MESH_FACE_EAST)
    {
        *column_count = GAME_VOXEL_CHUNK_DEPTH;
        *row_count = GAME_VOXEL_CHUNK_HEIGHT;
        return ;
    }
    if (face == CHUNK_MESH_FACE_DOWN || face == CHUNK_MESH_FACE_UP)
    {
        *column_count = GAME_VOXEL_CHUNK_WIDTH;
        *row_count = GAME_VOXEL_CHUNK_DEPTH;
        return ;
    }
    *column_count = GAME_VOXEL_CHUNK_WIDTH;
    *row_count = GAME_VOXEL_CHUNK_HEIGHT;
    return ;
}

static void chunk_mesh_block_coordinates_for_plane(chunk_mesh_face face,
    int32_t axis_value, int32_t column_value, int32_t row_value,
    int32_t *local_x, int32_t *local_y, int32_t *local_z) noexcept
{
    if (face == CHUNK_MESH_FACE_WEST || face == CHUNK_MESH_FACE_EAST)
    {
        *local_x = axis_value;
        *local_y = row_value;
        *local_z = column_value;
        return ;
    }
    if (face == CHUNK_MESH_FACE_DOWN || face == CHUNK_MESH_FACE_UP)
    {
        *local_x = column_value;
        *local_y = axis_value;
        *local_z = row_value;
        return ;
    }
    *local_x = column_value;
    *local_y = row_value;
    *local_z = axis_value;
    return ;
}

static int32_t chunk_mesh_fill_visible_face_mask(const game_voxel_chunk &chunk,
    chunk_mesh_face face, int32_t axis_value, uint32_t mask[4096]) noexcept
{
    int32_t column_count;
    int32_t row_count;
    int32_t column_value;
    int32_t row_value;
    int32_t local_x;
    int32_t local_y;
    int32_t local_z;
    uint32_t block_id;
    ft_bool visible;
    int32_t error_code;

    chunk_mesh_plane_dimensions(face, &column_count, &row_count);
    row_value = 0;
    while (row_value < row_count)
    {
        column_value = 0;
        while (column_value < column_count)
        {
            chunk_mesh_block_coordinates_for_plane(face, axis_value,
                column_value, row_value, &local_x, &local_y, &local_z);
            error_code = chunk.read_block(local_x, local_y, local_z,
                &block_id);
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
            mask[(row_value * column_count) + column_value] = 0U;
            if (block_id != GAME_VOXEL_AIR_BLOCK)
            {
                error_code = chunk_mesh_face_is_visible(chunk, local_x,
                    local_y, local_z, face, &visible);
                if (error_code != FT_ERR_SUCCESS)
                    return (error_code);
                if (visible == FT_TRUE)
                    mask[(row_value * column_count) + column_value] = block_id;
            }
            column_value += 1;
        }
        row_value += 1;
    }
    return (FT_ERR_SUCCESS);
}

static ft_bool chunk_mesh_mask_cell_matches(uint32_t mask[4096],
    ft_bool consumed[4096], int32_t column_count, int32_t column_value,
    int32_t row_value, uint32_t block_id) noexcept
{
    int32_t mask_index;

    mask_index = (row_value * column_count) + column_value;
    if (consumed[mask_index] == FT_TRUE)
        return (FT_FALSE);
    if (mask[mask_index] != block_id)
        return (FT_FALSE);
    return (FT_TRUE);
}

static int32_t chunk_mesh_greedy_width(uint32_t mask[4096],
    ft_bool consumed[4096], int32_t column_count, int32_t row_value,
    int32_t start_column, uint32_t block_id) noexcept
{
    int32_t width_count;

    width_count = 0;
    while (start_column + width_count < column_count
        && chunk_mesh_mask_cell_matches(mask, consumed, column_count,
            start_column + width_count, row_value, block_id) == FT_TRUE)
        width_count += 1;
    return (width_count);
}

static ft_bool chunk_mesh_greedy_row_matches(uint32_t mask[4096],
    ft_bool consumed[4096], int32_t column_count, int32_t row_value,
    int32_t start_column, int32_t width_count, uint32_t block_id) noexcept
{
    int32_t width_index;

    width_index = 0;
    while (width_index < width_count)
    {
        if (chunk_mesh_mask_cell_matches(mask, consumed, column_count,
                start_column + width_index, row_value, block_id) == FT_FALSE)
            return (FT_FALSE);
        width_index += 1;
    }
    return (FT_TRUE);
}

static int32_t chunk_mesh_greedy_height(uint32_t mask[4096],
    ft_bool consumed[4096], int32_t column_count, int32_t row_count,
    int32_t start_column, int32_t start_row, int32_t width_count,
    uint32_t block_id) noexcept
{
    int32_t height_count;

    height_count = 0;
    while (start_row + height_count < row_count
        && chunk_mesh_greedy_row_matches(mask, consumed, column_count,
            start_row + height_count, start_column, width_count,
            block_id) == FT_TRUE)
        height_count += 1;
    return (height_count);
}

static void chunk_mesh_mark_rectangle_consumed(ft_bool consumed[4096],
    int32_t column_count, int32_t start_column, int32_t start_row,
    int32_t width_count, int32_t height_count) noexcept
{
    int32_t width_index;
    int32_t height_index;

    height_index = 0;
    while (height_index < height_count)
    {
        width_index = 0;
        while (width_index < width_count)
        {
            consumed[((start_row + height_index) * column_count)
                + start_column + width_index] = FT_TRUE;
            width_index += 1;
        }
        height_index += 1;
    }
    return ;
}

static void chunk_mesh_clear_consumed_mask(ft_bool consumed[4096],
    int32_t cell_count) noexcept
{
    int32_t cell_index;

    cell_index = 0;
    while (cell_index < cell_count)
    {
        consumed[cell_index] = FT_FALSE;
        cell_index += 1;
    }
    return ;
}

static int32_t chunk_mesh_emit_greedy_mask(chunk_mesh &mesh,
    uint32_t mask[4096], ft_bool consumed[4096], chunk_mesh_face face,
    int32_t axis_value) noexcept
{
    int32_t column_count;
    int32_t row_count;
    int32_t column_value;
    int32_t row_value;
    int32_t width_count;
    int32_t height_count;
    uint32_t block_id;
    int32_t error_code;

    chunk_mesh_plane_dimensions(face, &column_count, &row_count);
    chunk_mesh_clear_consumed_mask(consumed, column_count * row_count);
    row_value = 0;
    while (row_value < row_count)
    {
        column_value = 0;
        while (column_value < column_count)
        {
            block_id = mask[(row_value * column_count) + column_value];
            if (block_id != GAME_VOXEL_AIR_BLOCK
                && consumed[(row_value * column_count)
                    + column_value] == FT_FALSE)
            {
                width_count = chunk_mesh_greedy_width(mask, consumed,
                    column_count, row_value, column_value, block_id);
                height_count = chunk_mesh_greedy_height(mask, consumed,
                    column_count, row_count, column_value, row_value,
                    width_count, block_id);
                error_code = chunk_mesh_emit_rectangle(mesh, axis_value,
                    column_value, row_value, column_value + width_count,
                    row_value + height_count, block_id, face);
                if (error_code != FT_ERR_SUCCESS)
                    return (error_code);
                chunk_mesh_mark_rectangle_consumed(consumed, column_count,
                    column_value, row_value, width_count, height_count);
            }
            column_value += 1;
        }
        row_value += 1;
    }
    return (FT_ERR_SUCCESS);
}

static int32_t chunk_mesh_axis_count(chunk_mesh_face face) noexcept
{
    if (face == CHUNK_MESH_FACE_WEST || face == CHUNK_MESH_FACE_EAST)
        return (GAME_VOXEL_CHUNK_WIDTH);
    if (face == CHUNK_MESH_FACE_DOWN || face == CHUNK_MESH_FACE_UP)
        return (GAME_VOXEL_CHUNK_HEIGHT);
    return (GAME_VOXEL_CHUNK_DEPTH);
}

static int32_t chunk_mesh_emit_greedy_faces_for_direction(chunk_mesh &mesh,
    const game_voxel_chunk &chunk, chunk_mesh_face face) noexcept
{
    uint32_t mask[4096];
    ft_bool consumed[4096];
    int32_t axis_count;
    int32_t axis_value;
    int32_t error_code;

    axis_count = chunk_mesh_axis_count(face);
    axis_value = 0;
    while (axis_value < axis_count)
    {
        error_code = chunk_mesh_fill_visible_face_mask(chunk, face,
            axis_value, mask);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        error_code = chunk_mesh_emit_greedy_mask(mesh, mask, consumed, face,
            axis_value);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        axis_value += 1;
    }
    return (FT_ERR_SUCCESS);
}

static int32_t chunk_mesh_emit_visible_faces(chunk_mesh &mesh,
    const game_voxel_chunk &chunk) noexcept
{
    chunk_mesh_face face;
    int32_t error_code;

    face = CHUNK_MESH_FACE_WEST;
    while (face <= CHUNK_MESH_FACE_SOUTH)
    {
        error_code = chunk_mesh_emit_greedy_faces_for_direction(mesh, chunk,
            face);
        if (error_code != FT_ERR_SUCCESS)
            return (error_code);
        face = static_cast<chunk_mesh_face>(static_cast<int32_t>(face) + 1);
    }
    return (FT_ERR_SUCCESS);
}

int32_t chunk_mesh_generate_from_chunk(chunk_mesh &mesh,
    const game_voxel_chunk &chunk) noexcept
{
    ft_size_t face_count;
    int32_t error_code;

    error_code = chunk_mesh_count_visible_faces(chunk, &face_count);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = chunk_mesh_clear(mesh);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    mesh.vertices.reserve(face_count * 4U);
    if (mesh.vertices.get_error() != FT_ERR_SUCCESS)
        return (mesh.vertices.get_error());
    mesh.indices.reserve(face_count * 6U);
    if (mesh.indices.get_error() != FT_ERR_SUCCESS)
        return (mesh.indices.get_error());
    return (chunk_mesh_emit_visible_faces(mesh, chunk));
}

#endif
