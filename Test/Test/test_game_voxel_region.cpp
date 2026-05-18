#include "../test_internal.hpp"
#include "../../Modules/Game/game_voxel_region.hpp"
#include "../../Modules/Printf/printf.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

static int32_t test_voxel_make_temp_path(char *buffer,
    ft_size_t buffer_size) noexcept
{
    int32_t process_id;

    process_id = getpid();
    if (pf_snprintf(buffer, buffer_size, "/tmp/libft_voxel_region_%d",
            process_id) < 0)
        return (FT_ERR_INVALID_ARGUMENT);
    return (FT_ERR_SUCCESS);
}

static void test_voxel_cleanup_region_dir(const char *directory_path) noexcept
{
    char file_path[256];

    (void)pf_snprintf(file_path, sizeof(file_path), "%s/region_P0P0.dat",
        directory_path);
    (void)unlink(file_path);
    (void)pf_snprintf(file_path, sizeof(file_path), "%s/region_N512N512.dat",
        directory_path);
    (void)unlink(file_path);
    (void)pf_snprintf(file_path, sizeof(file_path), "%s/region_P512P0.dat",
        directory_path);
    (void)unlink(file_path);
    (void)rmdir(directory_path);
    return ;
}

static int32_t test_voxel_write_unique_blocks(game_voxel_chunk &chunk,
    uint32_t block_count) noexcept
{
    uint32_t block_index;
    int32_t write_error;

    block_index = 1;
    while (block_index <= block_count)
    {
        write_error = chunk.write_block(static_cast<int32_t>(block_index & 15U),
            static_cast<int32_t>((block_index >> 8) & 15U),
            static_cast<int32_t>((block_index >> 4) & 15U),
            block_index);
        if (write_error != FT_ERR_SUCCESS)
            return (write_error);
        block_index += 1;
    }
    return (FT_ERR_SUCCESS);
}

FT_TEST(test_game_voxel_chunk_empty_reads_air)
{
    game_voxel_chunk chunk;
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(0, 0, 0, &block_id));
    FT_ASSERT_EQ(GAME_VOXEL_AIR_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(15, 255, 15, &block_id));
    FT_ASSERT_EQ(GAME_VOXEL_AIR_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, chunk.read_block(-1, 0, 0, &block_id));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, chunk.read_block(16, 0, 0, &block_id));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, chunk.read_block(0, 256, 0, &block_id));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_game_voxel_chunk_palette_sections)
{
    game_voxel_chunk chunk;
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_TRUE, chunk.get_section(0).is_uniform());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(1, 2, 3, 7));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(1, 2, 3, &block_id));
    FT_ASSERT_EQ(7U, block_id);
    FT_ASSERT_EQ(FT_TRUE, chunk.get_section(0).is_materialized());
    FT_ASSERT_EQ(2, chunk.get_section(0).get_palette_size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(2, 2, 3, 8));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(2, 2, 3, &block_id));
    FT_ASSERT_EQ(8U, block_id);
    FT_ASSERT_EQ(3, chunk.get_section(0).get_palette_size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(1, 20, 1, 9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(1, 20, 1, &block_id));
    FT_ASSERT_EQ(9U, block_id);
    FT_ASSERT_EQ(FT_TRUE, chunk.get_section(1).is_materialized());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(1, 2, 3,
        GAME_VOXEL_AIR_BLOCK));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(1, 2, 3, &block_id));
    FT_ASSERT_EQ(GAME_VOXEL_AIR_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_game_voxel_section_collapses_to_uniform_air)
{
    game_voxel_chunk chunk;
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(4, 4, 4, 12));
    FT_ASSERT_EQ(FT_TRUE, chunk.get_section(0).is_materialized());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(4, 4, 4,
        GAME_VOXEL_AIR_BLOCK));
    FT_ASSERT_EQ(FT_TRUE, chunk.get_section(0).is_uniform());
    FT_ASSERT_EQ(GAME_VOXEL_AIR_BLOCK,
        chunk.get_section(0).get_uniform_block());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.read_block(4, 4, 4, &block_id));
    FT_ASSERT_EQ(GAME_VOXEL_AIR_BLOCK, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_game_voxel_chunk_dirty_and_serialization_round_trip)
{
    game_voxel_chunk chunk;
    game_voxel_chunk loaded_chunk;
    ft_byte_buffer buffer;
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.initialize());
    FT_ASSERT_EQ(FT_FALSE, chunk.is_dirty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(0, 0, 0, 44));
    FT_ASSERT_EQ(FT_TRUE, chunk.is_dirty());
    chunk.clear_dirty();
    FT_ASSERT_EQ(FT_FALSE, chunk.is_dirty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(15, 255, 15, 45));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.serialize(buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_chunk.deserialize(buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_chunk.read_block(0, 0, 0, &block_id));
    FT_ASSERT_EQ(44U, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_chunk.read_block(15, 255, 15,
        &block_id));
    FT_ASSERT_EQ(45U, block_id);
    FT_ASSERT_EQ(FT_FALSE, loaded_chunk.is_dirty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_chunk.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_game_voxel_chunk_rejects_bad_serialized_data)
{
    game_voxel_chunk chunk;
    game_voxel_chunk loaded_chunk;
    ft_byte_buffer truncated_buffer;
    ft_byte_buffer bad_section_buffer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, truncated_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, truncated_buffer.append_u32_le(0));
    FT_ASSERT_EQ(FT_ERR_IO, loaded_chunk.deserialize(truncated_buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bad_section_buffer.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.serialize(bad_section_buffer));
    bad_section_buffer._data[8] = 99;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        loaded_chunk.deserialize(bad_section_buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, bad_section_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, truncated_buffer.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_chunk.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_game_voxel_chunk_palette_limit_is_reported)
{
    game_voxel_chunk chunk;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, test_voxel_write_unique_blocks(chunk, 255));
    FT_ASSERT_EQ(256, chunk.get_section(0).get_palette_size());
    FT_ASSERT_EQ(FT_ERR_FULL, chunk.write_block(0, 1, 0, 300));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    return (1);
}

FT_TEST(test_game_voxel_region_file_names)
{
    ft_string file_name;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_name.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        game_voxel_region_file_name_from_start(0, 0, file_name));
    FT_ASSERT_STR_EQ("region_P0P0.dat", file_name.c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        game_voxel_region_file_name_from_start(16, -16, file_name));
    FT_ASSERT_STR_EQ("region_P16N16.dat", file_name.c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        game_voxel_region_file_name_from_start(-16, -16, file_name));
    FT_ASSERT_STR_EQ("region_N16N16.dat", file_name.c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        game_voxel_region_file_name(0, 0, file_name));
    FT_ASSERT_STR_EQ("region_P0P0.dat", file_name.c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        game_voxel_region_file_name(512, 0, file_name));
    FT_ASSERT_STR_EQ("region_P512P0.dat", file_name.c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        game_voxel_region_file_name(-1, -1, file_name));
    FT_ASSERT_STR_EQ("region_N512N512.dat", file_name.c_str());
    FT_ASSERT_EQ(-512, game_voxel_region_start_coordinate(-1));
    FT_ASSERT_EQ(0, game_voxel_region_start_coordinate(511));
    FT_ASSERT_EQ(512, game_voxel_region_start_coordinate(512));
    FT_ASSERT_EQ(-512, game_voxel_region_start_coordinate(-512));
    FT_ASSERT_EQ(-1024, game_voxel_region_start_coordinate(-513));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_name.destroy());
    return (1);
}

FT_TEST(test_game_voxel_region_negative_coordinate_boundaries)
{
    char directory_path[256];
    char expected_file_path[300];
    game_voxel_region region;
    game_voxel_region loaded_region;
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, test_voxel_make_temp_path(directory_path,
        sizeof(directory_path)));
    test_voxel_cleanup_region_dir(directory_path);
    FT_ASSERT_EQ(0, mkdir(directory_path, 0700));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize(-1, -1, directory_path));
    FT_ASSERT_EQ(-512, region.get_region_start_x());
    FT_ASSERT_EQ(-512, region.get_region_start_z());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.write_block(-512, 0, -512, 31));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.write_block(-1, 255, -1, 32));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, region.write_block(-513, 0, -512, 33));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.save_region());
    (void)pf_snprintf(expected_file_path, sizeof(expected_file_path),
        "%s/region_N512N512.dat", directory_path);
    FT_ASSERT_EQ(0, access(expected_file_path, F_OK));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.initialize(-1, -1,
        directory_path));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.load_region(-1, -1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.read_block(-512, 0, -512,
        &block_id));
    FT_ASSERT_EQ(31U, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.read_block(-1, 255, -1,
        &block_id));
    FT_ASSERT_EQ(32U, block_id);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, loaded_region.read_block(0, 0, -1,
        &block_id));
    FT_ASSERT_EQ(FT_TRUE, loaded_region.is_chunk_loaded(-32, -32));
    FT_ASSERT_EQ(FT_TRUE, loaded_region.is_chunk_loaded(-1, -1));
    FT_ASSERT_EQ(FT_FALSE, loaded_region.is_chunk_loaded(0, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.destroy());
    test_voxel_cleanup_region_dir(directory_path);
    return (1);
}

FT_TEST(test_game_voxel_region_save_load_round_trip)
{
    char directory_path[256];
    char expected_file_path[300];
    game_voxel_region region;
    game_voxel_region loaded_region;
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, test_voxel_make_temp_path(directory_path,
        sizeof(directory_path)));
    test_voxel_cleanup_region_dir(directory_path);
    FT_ASSERT_EQ(0, mkdir(directory_path, 0700));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize(0, 0, directory_path));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.write_block(0, 0, 0, 22));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.write_block(15, 255, 15, 23));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.write_block(511, 10, 511, 24));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.save_region());
    (void)pf_snprintf(expected_file_path, sizeof(expected_file_path),
        "%s/region_P0P0.dat", directory_path);
    FT_ASSERT_EQ(0, access(expected_file_path, F_OK));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.initialize(0, 0, directory_path));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.load_region(0, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.read_block(0, 0, 0, &block_id));
    FT_ASSERT_EQ(22U, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.read_block(15, 255, 15,
        &block_id));
    FT_ASSERT_EQ(23U, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.read_block(511, 10, 511,
        &block_id));
    FT_ASSERT_EQ(24U, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.read_block(16, 1, 16,
        &block_id));
    FT_ASSERT_EQ(GAME_VOXEL_AIR_BLOCK, block_id);
    FT_ASSERT_EQ(FT_TRUE, loaded_region.is_chunk_loaded(0, 0));
    FT_ASSERT_EQ(FT_FALSE, loaded_region.is_chunk_loaded(1, 1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.destroy());
    test_voxel_cleanup_region_dir(directory_path);
    return (1);
}

FT_TEST(test_game_voxel_region_save_chunk_updates_slot)
{
    char directory_path[256];
    game_voxel_chunk chunk;
    game_voxel_region region;
    game_voxel_region loaded_region;
    uint32_t block_id;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, test_voxel_make_temp_path(directory_path,
        sizeof(directory_path)));
    test_voxel_cleanup_region_dir(directory_path);
    FT_ASSERT_EQ(0, mkdir(directory_path, 0700));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.write_block(3, 4, 5, 77));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize(0, 0, directory_path));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.save_chunk(2, 3, chunk));
    FT_ASSERT_EQ(FT_TRUE, region.has_chunk(2, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.read_block(35, 4, 53, &block_id));
    FT_ASSERT_EQ(77U, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.save_region());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.initialize(0, 0,
        directory_path));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.load_region(0, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.read_block(35, 4, 53,
        &block_id));
    FT_ASSERT_EQ(77U, block_id);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loaded_region.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chunk.destroy());
    test_voxel_cleanup_region_dir(directory_path);
    return (1);
}

FT_TEST(test_game_voxel_region_missing_storage_path_errors)
{
    char directory_path[256];
    game_voxel_region region;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, test_voxel_make_temp_path(directory_path,
        sizeof(directory_path)));
    test_voxel_cleanup_region_dir(directory_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize(0, 0, directory_path));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        region.set_region_storage_path(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_FILE_OPEN_FAILED, region.save_region());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.destroy());
    return (1);
}

FT_TEST(test_game_voxel_region_missing_and_corrupt_files)
{
    char directory_path[256];
    char file_path[300];
    game_voxel_region region;
    FILE *file;
    uint32_t bad_magic;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, test_voxel_make_temp_path(directory_path,
        sizeof(directory_path)));
    test_voxel_cleanup_region_dir(directory_path);
    FT_ASSERT_EQ(0, mkdir(directory_path, 0700));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.initialize(-1, -1, directory_path));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.load_region(-1, -1));
    (void)pf_snprintf(file_path, sizeof(file_path), "%s/region_N512N512.dat",
        directory_path);
    file = std::fopen(file_path, "wb");
    FT_ASSERT(file != ft_nullptr);
    bad_magic = 0;
    FT_ASSERT_EQ(1, std::fwrite(&bad_magic, sizeof(bad_magic), 1, file));
    (void)std::fclose(file);
    FT_ASSERT_EQ(FT_ERR_IO, region.load_region(-1, -1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region.destroy());
    test_voxel_cleanup_region_dir(directory_path);
    return (1);
}
