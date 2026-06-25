# Voxel

The `Voxel` module is compiled when `GAME_USE_VOXEL_REGION_BACKEND` is enabled. It generates biome-aware voxel terrain with seeded heightmaps and builds renderable chunk meshes from voxel data.

## Chunk Mesh

- `chunk_mesh_face` - Enum for west, east, down, up, north, and south block faces.
- `chunk_mesh_vertex` - One mesh vertex with voxel coordinates, texture coordinates, block id, and face id.
- `chunk_mesh_bounds` - Integer minimum and maximum bounds for generated mesh content.
- `chunk_mesh` - Mesh container with vertex vector, index vector, full chunk bounds, and cached occupied bounds.
- `chunk_mesh_initialize(chunk_mesh &mesh)` - Initializes the mesh vectors and bounds.
- `chunk_mesh_destroy(chunk_mesh &mesh)` - Destroys mesh-owned vectors.
- `chunk_mesh_clear(chunk_mesh &mesh)` - Clears mesh vertices/indices and resets bounds.
- `chunk_mesh_generate_from_chunk(chunk_mesh &mesh, const game_voxel_chunk &chunk)` - Generates visible faces for a voxel chunk.
- `chunk_mesh_intersects_frustum(frustum, world_origin_x, world_origin_y, world_origin_z)` - Tests whether a chunk-sized world box intersects the view frustum.
- `chunk_mesh_intersects_frustum(frustum, mesh, world_origin_x, world_origin_y, world_origin_z)` - Tests the mesh's occupied bounds against the view frustum.

## Voxel Generation

- `TERRAIN_GENERATOR_SURFACE_HEIGHT` - Default generated terrain surface height.
- `TERRAIN_GENERATOR_GRASS_BLOCK` - Block id used for grass.
- `TERRAIN_GENERATOR_DIRT_BLOCK` - Block id used for dirt.
- `TERRAIN_GENERATOR_STONE_BLOCK` - Block id used for stone.
- `TERRAIN_GENERATOR_SHRUB_BLOCK` - Block id used for shrub decorations.
- `TERRAIN_GENERATOR_OAK_LOG_BLOCK` - Block id used for tree trunks.
- `TERRAIN_GENERATOR_OAK_LEAVES_BLOCK` - Block id used for tree foliage.
- `TERRAIN_GENERATOR_CACTUS_BLOCK` - Block id used for cactus decorations.
- `TERRAIN_BIOME_ZONE_WIDTH` - Width in world blocks used to group biome zones.
- `terrain_biome` - Biome selector enum for plains, hills, desert, snow, and mountains.
- `terrain_biome_profile` - Surface height, variation, and topsoil depth profile for a biome.
- `terrain_block_metadata` - Registry entry that describes whether a block is solid, transparent, liquid, replaceable, light-emitting, whether it occludes mesh faces, and how hard it is.
- `terrain_tree_template_block` - Relative block entry used by tree templates.
- `terrain_tree_template` - Block list wrapper for reusable tree presets.
- `terrain_get_block_metadata(block_id)` - Looks up the metadata entry for a known block id.
- `terrain_block_is_known(block_id)` - Returns whether a block id exists in the registry and should be accepted by chunk storage.
- `terrain_block_is_solid(block_id)` - Returns whether a block is treated as a solid collision block.
- `terrain_block_is_transparent(block_id)` - Returns whether a block should be treated as visually transparent.
- `terrain_block_is_liquid(block_id)` - Returns whether a block behaves like a liquid.
- `terrain_block_is_replaceable(block_id)` - Returns whether a block can be overwritten by tree placement and similar terrain passes.
- `terrain_block_emits_light(block_id)` - Returns whether a block emits light.
- `terrain_block_occludes_faces(block_id)` - Returns whether a block should hide adjacent mesh faces.
- `terrain_block_hardness(block_id)` - Returns the block hardness value from the registry.
- `terrain_get_biome(world_block_x, world_block_z, seed_string)` - Picks a biome for a world block position and optional seed.
- `terrain_get_biome_profile(biome)` - Returns the height profile for a biome.
- `terrain_surface_block_for_biome(biome)` - Returns the surface block used for a biome.
- `terrain_biome_has_shrubs(biome)` - Returns whether a biome can spawn shrub decorations.
- `terrain_biome_has_trees(biome)` - Returns whether a biome can spawn tree decorations.
- `terrain_small_oak_tree_template(variant_index)` - Returns one of the reusable small oak presets.
- `terrain_small_pine_tree_template(variant_index)` - Returns one of the reusable small pine presets.
- `terrain_small_cactus_tree_template(variant_index)` - Returns one of the reusable small cactus presets.
- `terrain_large_oak_tree_template(variant_index)` - Returns one of the reusable large oak presets.
- `terrain_large_pine_tree_template(variant_index)` - Returns one of the reusable large pine presets.
- `terrain_tree_template_for_biome(biome)` - Returns the default tree preset for a biome.
- `terrain_tree_template_for_biome(biome, seed_value)` - Returns a seed-selected tree preset for a biome.
- `terrain_can_place_tree_template(chunk, local_origin_x, local_origin_y, local_origin_z, tree_template)` - Checks that a tree footprint fits in empty space before placement.
- `terrain_place_tree_template(chunk, local_origin_x, local_origin_y, local_origin_z, tree_template)` - Places a tree preset into a chunk.
- `terrain_generate_chunk(game_voxel_chunk &chunk, const char *seed_string)` - Fills a voxel chunk with biome-aware heightmap terrain based on an optional seed.
- `terrain_generate_chunk(game_voxel_chunk &chunk, int32_t world_block_origin_x, int32_t world_block_origin_z, const char *seed_string)` - Fills a voxel chunk with biome-aware heightmap terrain using a world-space chunk origin and optional seed.

## Voxel Behavior

- Biomes are selected in world-space zones so adjacent chunks line up cleanly across region boundaries.
- Height is driven by smooth noise instead of a flat cutoff, which produces hills and terrain variation within each biome.
- Biome profiles control baseline elevation, height variation, and topsoil depth, which makes it easy to tune different terrain regions independently.
- Surface blocks now vary by biome, and post-passes place shrubs plus reusable tree templates on suitable surfaces.
- The block registry records collision, transparency, replaceability, face occlusion, and hardness so terrain code can make local decisions without hardcoding block ids everywhere.
- Unknown block ids are rejected at chunk write and deserialization boundaries so corrupted palette data does not silently degrade into air-like behavior.
- Small oak trees are used for plains and hills, small pine trees are used for snow and mountains, and small cactus templates are used for desert regions.
- Oak and pine now have both small and large reusable presets, while cactus remains small only.
- Tree species and variants are selected from the world seed plus the tree's world position, so the same seed reproduces the same trees in the same places.
- Tree placement is preflight-checked against the target footprint, so generation skips blocked locations instead of overwriting existing blocks.
- Replaceable blocks do not block tree placement, which lets shrubs and similar decorative terrain be overwritten cleanly.
- Mesh generation now uses a block's face-occlusion flag instead of raw transparency, so transparent foliage can still suppress hidden internal faces.
- Chunk rendering can now frustum-cull whole chunk bounds before drawing, which is the next layer above greedy meshing.
- The mesh container now caches occupied bounds separately from the full chunk bounds, which lets render code cull sparse chunks more tightly when it has the generated mesh available.
