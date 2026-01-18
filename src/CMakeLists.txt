add_library(sensor_pcf8523 STATIC
    ${CMAKE_CURRENT_LIST_DIR}/pcf8523.c
)

target_include_directories(sensor_pcf8523
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(sensor_pcf8523 PUBLIC
    pico_stdlib
    hardware_i2c
)

target_compile_options(sensor_pcf8523 PRIVATE
    -Wall
    -Wextra
    -Wshadow
    -Wconversion
)
