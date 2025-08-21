add_executable(example
    main.c
)

target_link_libraries(example
    pico_stdlib
    hardware_i2c
    sensor_pcf8523
)

pico_enable_stdio_usb(example 0)
pico_enable_stdio_uart(example 1)

pico_add_extra_outputs(example)

add_executable(example2
    all.c
)

target_link_libraries(example2
    pico_stdlib
    hardware_i2c
    sensor_pcf8523
)

pico_enable_stdio_usb(example2 0)
pico_enable_stdio_uart(example2 1)

pico_add_extra_outputs(example2)
