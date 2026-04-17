cmake_minimum_required(VERSION 3.20)
project(Lorenz3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set your SDL3 paths here
set(SDL3_INCLUDE_DIR "C:/SDL3/include")
set(SDL3_LIB_DIR "C:/SDL3/lib/x64")

include_directories(${SDL3_INCLUDE_DIR})
link_directories(${SDL3_LIB_DIR})

add_executable(Lorenz3D main.cpp)
target_link_libraries(Lorenz3D SDL3)

# This part copies the DLL to your folder so the app can run
add_custom_command(TARGET Lorenz3D POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/SDL3/lib/x64/SDL3.dll"
        "$<TARGET_FILE_DIR:Lorenz3D>"
)