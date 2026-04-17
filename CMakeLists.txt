cmake_minimum_required(VERSION 3.20)
project(Lorenz3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
# Fetch GLFW and GLAD
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.36
)
FetchContent_MakeAvailable(glfw glad)

add_executable(Lorenz3D src/main_gl.cpp src/tesseract.cpp src/hypercube.cpp)

target_include_directories(Lorenz3D PRIVATE ${glad_SOURCE_DIR}/include)

target_link_libraries(Lorenz3D PRIVATE glfw glad)

if (WIN32)
  target_link_libraries(Lorenz3D PRIVATE opengl32)
endif()
