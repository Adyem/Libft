#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifdef GAME_USE_VOXEL_REGION_BACKEND

#include "../../Modules/Terrain/chunk_mesh.hpp"
#include "../../Modules/Terrain/terrain_generator.hpp"

FT_TEST(test_chunk_mesh_generate_single_block_visible_faces)
{
    game_voxel_chunk chunk;
    chunk_mesh mesh;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(1, 1, 1,
        TERRAIN_GENERATOR_STONE_BLOCK));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_initialize(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_generate_from_chunk(mesh, chunk));
    FT_ASSERT_EQ(24, mesh.vertices.size());
    FT_ASSERT_EQ(36, mesh.indices.size());
    FT_ASSERT_EQ(TERRAIN_GENERATOR_STONE_BLOCK, mesh.vertices[0].block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_destroy(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_chunk_mesh_generate_hides_shared_faces)
{
    game_voxel_chunk chunk;
    chunk_mesh mesh;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(1, 1, 1,
        TERRAIN_GENERATOR_STONE_BLOCK));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(2, 1, 1,
        TERRAIN_GENERATOR_STONE_BLOCK));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_initialize(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_generate_from_chunk(mesh, chunk));
    FT_ASSERT_EQ(40, mesh.vertices.size());
    FT_ASSERT_EQ(60, mesh.indices.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_destroy(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_chunk_mesh_generate_empty_chunk_has_no_geometry)
{
    game_voxel_chunk chunk;
    chunk_mesh mesh;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_initialize(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_generate_from_chunk(mesh, chunk));
    FT_ASSERT_EQ(0, mesh.vertices.size());
    FT_ASSERT_EQ(0, mesh.indices.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_destroy(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_chunk_mesh_generate_boundary_block_visible_faces)
{
    game_voxel_chunk chunk;
    chunk_mesh mesh;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(0, 0, 0,
        TERRAIN_GENERATOR_STONE_BLOCK));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_initialize(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_generate_from_chunk(mesh, chunk));
    FT_ASSERT_EQ(24, mesh.vertices.size());
    FT_ASSERT_EQ(36, mesh.indices.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_destroy(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_chunk_mesh_generate_hides_vertical_shared_faces)
{
    game_voxel_chunk chunk;
    chunk_mesh mesh;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(1, 1, 1,
        TERRAIN_GENERATOR_STONE_BLOCK));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(1, 2, 1,
        TERRAIN_GENERATOR_STONE_BLOCK));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_initialize(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_generate_from_chunk(mesh, chunk));
    FT_ASSERT_EQ(40, mesh.vertices.size());
    FT_ASSERT_EQ(60, mesh.indices.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_destroy(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_chunk_mesh_clear_removes_generated_geometry)
{
    game_voxel_chunk chunk;
    chunk_mesh mesh;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(1, 1, 1,
        TERRAIN_GENERATOR_STONE_BLOCK));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_initialize(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_generate_from_chunk(mesh, chunk));
    FT_ASSERT_EQ(24, mesh.vertices.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_clear(mesh));
    FT_ASSERT_EQ(0, mesh.vertices.size());
    FT_ASSERT_EQ(0, mesh.indices.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_destroy(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_chunk_mesh_generate_replaces_previous_mesh)
{
    game_voxel_chunk chunk;
    game_voxel_chunk empty_chunk;
    chunk_mesh mesh;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(1, 1, 1,
        TERRAIN_GENERATOR_STONE_BLOCK));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_initialize(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_generate_from_chunk(mesh, chunk));
    FT_ASSERT_EQ(24, mesh.vertices.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_generate_from_chunk(mesh,
        empty_chunk));
    FT_ASSERT_EQ(0, mesh.vertices.size());
    FT_ASSERT_EQ(0, mesh.indices.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_destroy(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty_chunk.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_chunk_mesh_generate_sets_chunk_bounds)
{
    game_voxel_chunk chunk;
    chunk_mesh mesh;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_initialize(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_generate_from_chunk(mesh, chunk));
    FT_ASSERT_EQ(0, mesh.bounds.minimum_x);
    FT_ASSERT_EQ(0, mesh.bounds.minimum_y);
    FT_ASSERT_EQ(0, mesh.bounds.minimum_z);
    FT_ASSERT_EQ(GAME_VOXEL_CHUNK_WIDTH, mesh.bounds.maximum_x);
    FT_ASSERT_EQ(GAME_VOXEL_CHUNK_HEIGHT, mesh.bounds.maximum_y);
    FT_ASSERT_EQ(GAME_VOXEL_CHUNK_DEPTH, mesh.bounds.maximum_z);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_destroy(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_chunk_mesh_generate_flat_chunk_expected_surface_counts)
{
    game_voxel_chunk chunk;
    chunk_mesh mesh;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, terrain_generate_chunk(chunk,
        "terrain-test-seed"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_initialize(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_generate_from_chunk(mesh, chunk));
    FT_ASSERT_EQ(21760, mesh.vertices.size());
    FT_ASSERT_EQ(32640, mesh.indices.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk_mesh_destroy(mesh));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

#endif
