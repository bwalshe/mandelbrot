cmake_minimum_required(VERSION 3.1)
project(Mandelbrot)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)

include_directories(${GTK_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)
link_directories(${GTK_LIBRARY_DIRS})


add_executable(mandelbrot ${CMAKE_SOURCE_DIR}/src/viewer.c 
			  ${CMAKE_SOURCE_DIR}/src/mandelbrot.c 
	                  ${CMAKE_SOURCE_DIR}/src/rubberband.c)
target_link_libraries(mandelbrot ${GTK_LIBRARIES} m)
target_compile_options(mandelbrot PRIVATE -Wall -Wextra -Wpedantic)

add_executable(demo_rubberband ${CMAKE_SOURCE_DIR}/test/demo_rubberband.c 
	                       ${CMAKE_SOURCE_DIR}/src/rubberband.c)
target_link_libraries(demo_rubberband ${GTK_LIBRARIES})
target_compile_options(demo_rubberband PRIVATE -Wall -Wextra -Wpedantic)
