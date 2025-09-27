# --- SERVER EXECUTABLE ---
add_executable(server
    server.c
    linlist.c
)

target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(server PRIVATE protocols ws2_32)


# --- CLIENT EXECUTABLE ---
add_executable(client
    client.c
    # If other utility files are only used by the client, add them here.
)

target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(client PRIVATE protocols ws2_32)
