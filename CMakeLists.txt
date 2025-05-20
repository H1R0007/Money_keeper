cmake_minimum_required(VERSION 3.17)
project(FinanceManager)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/Zeta/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_CXX_STANDARD 17)

# Находим doxygen.exe
find_program(DOXYGEN_EXE 
    NAMES doxygen.exe doxygen
    PATHS "${CMAKE_SOURCE_DIR}/src/doxygen/bin"
    NO_DEFAULT_PATH
)

if(DOXYGEN_EXE)
    message(STATUS "Found Doxygen: ${DOXYGEN_EXE}")
    
    # Указываем правильный путь к Doxyfile
    set(DOXYFILE_PATH "${CMAKE_SOURCE_DIR}/docs/Doxyfile")
    
    # Проверяем существование Doxyfile
    if(EXISTS "${DOXYFILE_PATH}")
        message(STATUS "Found Doxyfile: ${DOXYFILE_PATH}")
        
        add_custom_target(docs
            COMMAND "${DOXYGEN_EXE}" "${DOXYFILE_PATH}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Generating documentation using ${DOXYGEN_EXE}"
            VERBATIM
        )
        
        # Создаем удобную команду для генерации
        add_custom_command(
            TARGET docs
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Documentation generated in ${CMAKE_SOURCE_DIR}/docs/html"
        )
    else()
        message(WARNING "Doxyfile not found at ${DOXYFILE_PATH}")
    endif()
else()
    message(WARNING "Doxygen not found in ${CMAKE_SOURCE_DIR}/src/doxygen/bin")
endif()


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
    core/currency/CurrencyConverter.cpp
    core/currency/CurrencyFetcher.cpp
    core/currency/CurrencyConverter.hpp
    core/currency/CurrencyFetcher.hpp
    core/currency/curl/CurlHttpClient.cpp
    core/currency/curl/CurlHttpClient.hpp
 "docs/mainpage.h")

if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

target_include_directories(FinanceManager PRIVATE core)


find_package(CURL REQUIRED)
target_link_libraries(FinanceManager PRIVATE
    CURL::libcurl
)
