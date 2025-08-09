cmake_minimum_required(VERSION 3.10)
project(json_to_sexpr C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

# Source and header directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES ${SRC_DIR}/*.c)

add_executable(json_to_sexpr ${SOURCES})

target_include_directories(json_to_sexpr PRIVATE ${INC_DIR})

if(MSVC)
    target_compile_options(json_to_sexpr PRIVATE /W4)
else()
    target_compile_options(json_to_sexpr PRIVATE -Wall -Wextra -pedantic -O2)
endif()

if(ENABLE_ASAN AND NOT MSVC)
    target_compile_options(json_to_sexpr PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(json_to_sexpr PRIVATE -fsanitize=address)
endif()

# Install rule
install(TARGETS json_to_sexpr RUNTIME DESTINATION bin)

# Tests (very simple custom test driver invoking program on sample files)
if(BUILD_TESTS)
    enable_testing()
    file(GLOB TEST_JSON_FILES ${CMAKE_SOURCE_DIR}/tests/data/*.json)
    foreach(jsonfile ${TEST_JSON_FILES})
        get_filename_component(testname ${jsonfile} NAME_WE)
        add_test(NAME run_${testname} COMMAND json_to_sexpr ${jsonfile})
    endforeach()
endif()

# Packaging support
include(CPack)
set(CPACK_PACKAGE_NAME "json_to_sexpr")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_PACKAGE_CONTACT "you@example.com")

# Usage message
message(STATUS "Build options: BUILD_TESTS=${BUILD_TESTS} ENABLE_ASAN=${ENABLE_ASAN}")
