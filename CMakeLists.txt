cmake_minimum_required(VERSION 3.10)

project(PUT-SK2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
endif()

include_directories(include)
set(GLOBAL_SRC src/global.cpp include/global.hpp)
set(MSQCLI_SRC src/msqCLI.cpp include/msqCLI.hpp)

add_library(msqCLI SHARED ${MSQCLI_SRC} ${GLOBAL_SRC})
add_executable(server src/server.cpp ${GLOBAL_SRC})
add_executable(demo src/demo.cpp ${GLOBAL_SRC} ${MSQCLI_SRC})

target_link_libraries(demo PRIVATE msqCLI)



