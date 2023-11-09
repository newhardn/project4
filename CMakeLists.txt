cmake_minimum_required(VERSION 3.23)
project(project4 C)

set(CMAKE_C_STANDARD 99)

add_executable(stego stego.c)
add_executable(stegoRead stegoRead.c)
add_executable(stegoDestroy stegoDestroy.c)

set(project_bitmaps
    alice.bmp
    aliceStego.bmp
)

# Copy non-compileable bitmaps to build directory
foreach(file ${project_bitmaps})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()
