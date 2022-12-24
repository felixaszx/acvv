cmake_minimum_required(VERSION 3.0.0)
project(acvv VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS off)

file(GLOB_RECURSE SRC_CPP src/*.cpp)
add_executable(acvv main.cpp ${SRC_CPP})
target_compile_definitions(acvv PUBLIC GLFW_INCLUDE_NONE)

# includes-cpp
target_include_directories(acvv PUBLIC include)

# libraries-cpp
target_link_libraries(acvv vulkan-1)
target_link_libraries(acvv glfw3)
target_link_libraries(acvv fmt)
target_link_libraries(acvv fltk)
