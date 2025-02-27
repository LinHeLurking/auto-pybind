cmake_minimum_required(VERSION 3.20)
project(PythonExt LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)
file(REAL_PATH ${CMAKE_CURRENT_LIST_DIR}/../auto_pybind PYTHON_ROOT)
message(STATUS "python source root: ${PYTHON_ROOT}")

set(THIRD_PARTY_LIBS)
find_package(Boost CONFIG REQUIRED)
list(APPEND THIRD_PARTY_LIBS Boost::boost)

function(add_pyext PYEXT_NAME)
    set(_OPTIONS)
    set(_SINGLE_VALUE_ARGS DESTINATION MODULE_NAME)
    set(_MULTI_VALUE_ARGS SOURCES)
    cmake_parse_arguments(PYEXT "${_OPTIONS}" "${_SINGLE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})
    if (NOT PYEXT_MODULE_NAME)
        set(PYEXT_MODULE_NAME ${PYEXT_NAME})
    endif ()
    if (NOT PYEXT_SOURCES)
        message(FATAL_ERROR "Must provide sources for ${PYEXT_NAME}")
    endif ()

    message(STATUS "building python extension: ${PYEXT_NAME}. sources: ${PYEXT_SOURCES}")
    pybind11_add_module(${PYEXT_NAME} MODULE ${PYEXT_SOURCES})
    target_link_libraries(${PYEXT_NAME} PRIVATE ${THIRD_PARTY_LIBS})
    target_compile_definitions(${PYEXT_NAME} PRIVATE MODULE_NAME=${PYEXT_MODULE_NAME})
    target_include_directories(${PYEXT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    install(TARGETS ${PYEXT_NAME} DESTINATION ${PYEXT_DESTINATION})
endfunction()

set(SRC_SET pybind.cc obj.h util.h pybind.h)
add_pyext(ext DESTINATION ${PYTHON_ROOT} SOURCES ${SRC_SET})


if (BUILD_TESTS)
    enable_testing()

    find_package(GTest CONFIG REQUIRED)

    macro(add_g_test TEST_NAME)
        add_executable(${TEST_NAME} ${ARGN})
        target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_link_libraries(${TEST_NAME} INTERFACE Threads::Threads)
        target_link_libraries(${TEST_NAME} PRIVATE
                GTest::gtest
                GTest::gtest_main
                # If you're using MSVC, don't link gmock.
                # GTest::gmock
                # GTest::gmock_main
        )
        target_link_libraries(${TEST_NAME} PRIVATE ${THIRD_PARTY_LIBS})
        add_test(${TEST_NAME} ${TEST_NAME})
    endmacro()

    add_g_test(test_describe tests/test_describe.cc)
endif ()
