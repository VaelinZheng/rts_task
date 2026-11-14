# Copyright (c) 2026, vaelin.zheng@gmail.com
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)
project(rts_task_tests LANGUAGES C)

include(CTest)

if(BUILD_TESTING)
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)

    include(FetchContent)

    # Fetch Unity at configure time; it is cached in the build directory.
    FetchContent_Declare(
        unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG        v2.6.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(unity)

    add_executable(test_rts_task test_rts_task.c)
    target_include_directories(test_rts_task PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(test_rts_task PRIVATE unity::framework)
    # The coroutine macros intentionally place a case label right after an
    # assignment, which GCC reports as an implicit fallthrough.
    target_compile_options(test_rts_task PRIVATE -Wall -Wextra -Wpedantic -Wno-implicit-fallthrough)
    add_test(NAME rts_task COMMAND test_rts_task)

    add_executable(test_rts_task_nonatomic test_rts_task_nonatomic.c)
    target_include_directories(test_rts_task_nonatomic PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(test_rts_task_nonatomic PRIVATE unity::framework)
    target_compile_options(test_rts_task_nonatomic PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME rts_task_nonatomic_clock COMMAND test_rts_task_nonatomic)

    add_executable(test_rts_task_disabled test_rts_task_disabled.c)
    add_test(NAME rts_task_disabled_configuration COMMAND test_rts_task_disabled)
endif()
