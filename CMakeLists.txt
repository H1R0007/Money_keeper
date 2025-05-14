cmake_minimum_required(VERSION 3.10)
project(FinanceManager)

set(CMAKE_CXX_STANDARD 17)

add_executable(FinanceManager
    main.cpp
    core/FinanceCore.cpp
    core/File_Manager.cpp
    core/Transactions.cpp
    core/Statistics.cpp
    core/Menu_Handlers.cpp
    core/Account.cpp
    core/Date.hpp
    core/Time_Manager.hpp
    core/Account.hpp
    core/FinanceCore.hpp
)

# Äëÿ Windows (MinGW)
if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
endif()

if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

target_include_directories(FinanceManager PRIVATE core)
