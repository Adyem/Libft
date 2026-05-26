# Terrain

The `Terrain` module is compiled when `GAME_USE_VOXEL_REGION_BACKEND` is enabled. It generates voxel chunks and builds renderable chunk meshes from voxel data.

## Chunk Mesh

- `chunk_mesh_face` - Enum for west, east, down, up, north, and south block faces.
- `chunk_mesh_vertex` - One mesh vertex with voxel coordinates, texture coordinates, block id, and face id.
- `chunk_mesh_bounds` - Integer minimum and maximum bounds for generated mesh content.
- `chunk_mesh` - Mesh container with vertex vector, index vector, and bounds.
- `chunk_mesh_initialize(chunk_mesh &mesh)` - Initializes the mesh vectors and bounds.
- `chunk_mesh_destroy(chunk_mesh &mesh)` - Destroys mesh-owned vectors.
- `chunk_mesh_clear(chunk_mesh &mesh)` - Clears mesh vertices/indices and resets bounds.
- `chunk_mesh_generate_from_chunk(chunk_mesh &mesh, const game_voxel_chunk &chunk)` - Generates visible faces for a voxel chunk.

## Terrain Generation

- `TERRAIN_GENERATOR_SURFACE_HEIGHT` - Default generated terrain surface height.
- `TERRAIN_GENERATOR_GRASS_BLOCK` - Block id used for grass.
- `TERRAIN_GENERATOR_DIRT_BLOCK` - Block id used for dirt.
- `TERRAIN_GENERATOR_STONE_BLOCK` - Block id used for stone.
- `terrain_generate_chunk(game_voxel_chunk &chunk, const char *seed_string)` - Fills a voxel chunk with deterministic terrain based on an optional seed.
