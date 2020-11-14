# CMakeLists.txt
#
#     Author: Fabian Meyer
# Created On: 12 Jul 2019

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${CATCH2_INCLUDE_DIR}
)

set(TEST_SRC
    "main.cpp"
    "test_inifile.cpp"
)

add_executable(run_tests ${TEST_SRC})
