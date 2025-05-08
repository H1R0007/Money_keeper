cmake_minimum_required(VERSION 3.10)
project(FinanceManager)

set(CMAKE_CXX_STANDARD 17)

add_executable(FinanceManager
    main.cpp
    core/FinanceCore.cpp
    core/Date.hpp
    core/Time_manager.hpp
    core/Account.hpp
)
target_include_directories(FinanceManager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
