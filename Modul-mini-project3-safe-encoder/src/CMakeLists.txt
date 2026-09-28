# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(
    SRCS
        "main.c"
        "../app/sources/safe.c"
        "../bsp/esp32/sources/encoder_esp32.c"
        "../bsp/esp32/sources/button_esp32.c"

    INCLUDE_DIRS
        "."
        "../app/headers"
        "../interfaces/headers"
        "../bsp/esp32/headers"
)
