#include "chunk_mesh.hpp"

#ifdef GAME_USE_VOXEL_REGION_BACKEND

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
    if (neighbor_block_id == GAME_VOXEL_AIR_BLOCK)
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

static void chunk_mesh_make_west_face(chunk_mesh_vertex vertices[4],
    int32_t local_x, int32_t local_y, int32_t local_z,
    uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], local_x, local_y, local_z, 0U, 0U,
        block_id, CHUNK_MESH_FACE_WEST);
    chunk_mesh_fill_vertex(&vertices[1], local_x, local_y + 1, local_z, 0U,
        1U, block_id, CHUNK_MESH_FACE_WEST);
    chunk_mesh_fill_vertex(&vertices[2], local_x, local_y + 1, local_z + 1,
        1U, 1U, block_id, CHUNK_MESH_FACE_WEST);
    chunk_mesh_fill_vertex(&vertices[3], local_x, local_y, local_z + 1, 1U,
        0U, block_id, CHUNK_MESH_FACE_WEST);
    return ;
}

static void chunk_mesh_make_east_face(chunk_mesh_vertex vertices[4],
    int32_t local_x, int32_t local_y, int32_t local_z,
    uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], local_x + 1, local_y, local_z + 1,
        0U, 0U, block_id, CHUNK_MESH_FACE_EAST);
    chunk_mesh_fill_vertex(&vertices[1], local_x + 1, local_y + 1,
        local_z + 1, 0U, 1U, block_id, CHUNK_MESH_FACE_EAST);
    chunk_mesh_fill_vertex(&vertices[2], local_x + 1, local_y + 1, local_z,
        1U, 1U, block_id, CHUNK_MESH_FACE_EAST);
    chunk_mesh_fill_vertex(&vertices[3], local_x + 1, local_y, local_z, 1U,
        0U, block_id, CHUNK_MESH_FACE_EAST);
    return ;
}

static void chunk_mesh_make_down_face(chunk_mesh_vertex vertices[4],
    int32_t local_x, int32_t local_y, int32_t local_z,
    uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], local_x, local_y, local_z + 1, 0U,
        0U, block_id, CHUNK_MESH_FACE_DOWN);
    chunk_mesh_fill_vertex(&vertices[1], local_x + 1, local_y, local_z + 1,
        0U, 1U, block_id, CHUNK_MESH_FACE_DOWN);
    chunk_mesh_fill_vertex(&vertices[2], local_x + 1, local_y, local_z, 1U,
        1U, block_id, CHUNK_MESH_FACE_DOWN);
    chunk_mesh_fill_vertex(&vertices[3], local_x, local_y, local_z, 1U, 0U,
        block_id, CHUNK_MESH_FACE_DOWN);
    return ;
}

static void chunk_mesh_make_up_face(chunk_mesh_vertex vertices[4],
    int32_t local_x, int32_t local_y, int32_t local_z,
    uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], local_x, local_y + 1, local_z, 0U,
        0U, block_id, CHUNK_MESH_FACE_UP);
    chunk_mesh_fill_vertex(&vertices[1], local_x + 1, local_y + 1, local_z,
        0U, 1U, block_id, CHUNK_MESH_FACE_UP);
    chunk_mesh_fill_vertex(&vertices[2], local_x + 1, local_y + 1,
        local_z + 1, 1U, 1U, block_id, CHUNK_MESH_FACE_UP);
    chunk_mesh_fill_vertex(&vertices[3], local_x, local_y + 1, local_z + 1,
        1U, 0U, block_id, CHUNK_MESH_FACE_UP);
    return ;
}

static void chunk_mesh_make_north_face(chunk_mesh_vertex vertices[4],
    int32_t local_x, int32_t local_y, int32_t local_z,
    uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], local_x + 1, local_y, local_z, 0U,
        0U, block_id, CHUNK_MESH_FACE_NORTH);
    chunk_mesh_fill_vertex(&vertices[1], local_x + 1, local_y + 1, local_z,
        0U, 1U, block_id, CHUNK_MESH_FACE_NORTH);
    chunk_mesh_fill_vertex(&vertices[2], local_x, local_y + 1, local_z, 1U,
        1U, block_id, CHUNK_MESH_FACE_NORTH);
    chunk_mesh_fill_vertex(&vertices[3], local_x, local_y, local_z, 1U, 0U,
        block_id, CHUNK_MESH_FACE_NORTH);
    return ;
}

static void chunk_mesh_make_south_face(chunk_mesh_vertex vertices[4],
    int32_t local_x, int32_t local_y, int32_t local_z,
    uint32_t block_id) noexcept
{
    chunk_mesh_fill_vertex(&vertices[0], local_x, local_y, local_z + 1, 0U,
        0U, block_id, CHUNK_MESH_FACE_SOUTH);
    chunk_mesh_fill_vertex(&vertices[1], local_x, local_y + 1, local_z + 1,
        0U, 1U, block_id, CHUNK_MESH_FACE_SOUTH);
    chunk_mesh_fill_vertex(&vertices[2], local_x + 1, local_y + 1,
        local_z + 1, 1U, 1U, block_id, CHUNK_MESH_FACE_SOUTH);
    chunk_mesh_fill_vertex(&vertices[3], local_x + 1, local_y, local_z + 1,
        1U, 0U, block_id, CHUNK_MESH_FACE_SOUTH);
    return ;
}

static void chunk_mesh_make_face_vertices(chunk_mesh_vertex vertices[4],
    int32_t local_x, int32_t local_y, int32_t local_z, uint32_t block_id,
    chunk_mesh_face face) noexcept
{
    if (face == CHUNK_MESH_FACE_WEST)
        chunk_mesh_make_west_face(vertices, local_x, local_y, local_z,
            block_id);
    if (face == CHUNK_MESH_FACE_EAST)
        chunk_mesh_make_east_face(vertices, local_x, local_y, local_z,
            block_id);
    if (face == CHUNK_MESH_FACE_DOWN)
        chunk_mesh_make_down_face(vertices, local_x, local_y, local_z,
            block_id);
    if (face == CHUNK_MESH_FACE_UP)
        chunk_mesh_make_up_face(vertices, local_x, local_y, local_z,
            block_id);
    if (face == CHUNK_MESH_FACE_NORTH)
        chunk_mesh_make_north_face(vertices, local_x, local_y, local_z,
            block_id);
    if (face == CHUNK_MESH_FACE_SOUTH)
        chunk_mesh_make_south_face(vertices, local_x, local_y, local_z,
            block_id);
    return ;
}

static int32_t chunk_mesh_push_index(chunk_mesh &mesh,
    uint32_t index_value) noexcept
{
    return (mesh.indices.push_back(index_value));
}

static int32_t chunk_mesh_emit_face(chunk_mesh &mesh, int32_t local_x,
    int32_t local_y, int32_t local_z, uint32_t block_id,
    chunk_mesh_face face) noexcept
{
    chunk_mesh_vertex vertices[4];
    uint32_t base_vertex;
    int32_t error_code;

    base_vertex = static_cast<uint32_t>(mesh.vertices.size());
    chunk_mesh_make_face_vertices(vertices, local_x, local_y, local_z,
        block_id, face);
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

static int32_t chunk_mesh_emit_visible_block_faces(chunk_mesh &mesh,
    const game_voxel_chunk &chunk, int32_t local_x, int32_t local_y,
    int32_t local_z, uint32_t block_id) noexcept
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
        {
            error_code = chunk_mesh_emit_face(mesh, local_x, local_y,
                local_z, block_id, face);
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        face = static_cast<chunk_mesh_face>(static_cast<int32_t>(face) + 1);
    }
    return (FT_ERR_SUCCESS);
}

static int32_t chunk_mesh_emit_visible_faces(chunk_mesh &mesh,
    const game_voxel_chunk &chunk) noexcept
{
    int32_t local_x;
    int32_t local_y;
    int32_t local_z;
    uint32_t block_id;
    int32_t error_code;

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
                    error_code = chunk_mesh_emit_visible_block_faces(mesh,
                        chunk, local_x, local_y, local_z, block_id);
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
