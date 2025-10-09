cmake_minimum_required(VERSION 3.20)
project(libft_quick_start LANGUAGES CXX)

set(LIBFT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/libft" CACHE PATH "Path to the Libft checkout")

if(NOT EXISTS "${LIBFT_ROOT}/FullLibft.hpp")
    message(FATAL_ERROR "LIBFT_ROOT must point to the Libft repository root")
endif()

file(GLOB_RECURSE LIBFT_SOURCES CONFIGURE_DEPENDS "${LIBFT_ROOT}/*.cpp")
list(FILTER LIBFT_SOURCES EXCLUDE REGEX "/Test/")
list(FILTER LIBFT_SOURCES EXCLUDE REGEX "/tools/")

add_library(libft_core STATIC ${LIBFT_SOURCES})
target_include_directories(libft_core
    PUBLIC
        "${LIBFT_ROOT}"
)
target_compile_features(libft_core PUBLIC cxx_std_17)

add_executable(quick_start src/main.cpp)
target_link_libraries(quick_start PRIVATE libft_core)
set_target_properties(quick_start PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
