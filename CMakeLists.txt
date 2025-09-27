cmake_minimum_required(VERSION 3.10)
project(MiniRedis)

# Add global include directory (all targets see include/)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add subdirectories
add_subdirectory(src/protocols)
add_subdirectory(src/server)
#add_subdirectory(src/client)
