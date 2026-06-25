#ifndef GAME_REGION_BACKEND_HPP
# define GAME_REGION_BACKEND_HPP

#include "game_world_region.hpp"
#include "game_voxel_region.hpp"

#ifdef GAME_USE_VOXEL_REGION_BACKEND
typedef game_voxel_region game_active_region_backend;
# define GAME_ACTIVE_REGION_BACKEND_VOXEL 1
#else
typedef game_world_region game_active_region_backend;
# define GAME_ACTIVE_REGION_BACKEND_VOXEL 0
#endif

#ifdef LIBFT_TEST_BUILD
typedef game_world_region game_world_region_backend;
typedef game_voxel_region game_voxel_region_backend;
#endif

#endif
