cmake_minimum_required(VERSION 3.17)
project(FinanceManager)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/Zeta/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_CXX_STANDARD 17)

# Настройка документации
set(DOXYGEN_PATH "C:/Users/Zeta/source/external/doxygen/bin/doxygen.exe")
set(GRAPHVIZ_PATH "C:/Users/Zeta/source/external/Graphviz/bin/dot.exe")
set(DOXYFILE_PATH "C:/Users/Zeta/source/external/docs/Doxyfile")

if(EXISTS ${DOXYGEN_PATH})
    message(STATUS "Found Doxygen: ${DOXYGEN_PATH}")
    
    if(EXISTS ${GRAPHVIZ_PATH})
        message(STATUS "Found Graphviz: ${GRAPHVIZ_PATH}")
    else()
        message(WARNING "Graphviz not found - diagrams will be disabled")
    endif()

    add_custom_target(docs
        COMMAND ${DOXYGEN_PATH} ${DOXYFILE_PATH}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating documentation"
        VERBATIM
    )
    
    add_custom_command(
        TARGET docs
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Documentation generated in ${CMAKE_SOURCE_DIR}/docs/html"
    )
else()
    message(WARNING "Doxygen not found at ${DOXYGEN_PATH} - documentation disabled")
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
    core/currency/curl/CurlHttpClient.hpp)

if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

target_include_directories(FinanceManager PRIVATE core)


find_package(CURL REQUIRED)
target_link_libraries(FinanceManager PRIVATE
    CURL::libcurl
)
