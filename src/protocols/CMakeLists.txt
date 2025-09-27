add_library(protocols
    resp.c
)

target_include_directories(protocols PUBLIC ${CMAKE_SOURCE_DIR}/include)
