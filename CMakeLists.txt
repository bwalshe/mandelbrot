cmake_minimum_required(VERSION 3.1)
project(Mandelbrot)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)

include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})


add_executable(mandelbrot ${CMAKE_SOURCE_DIR}/src/mandelbrot.c)
target_link_libraries(mandelbrot ${GTK_LIBRARIES} m)
target_compile_options(mandelbrot PRIVATE -Wall -Wextra -Wpedantic -Werror)
