cmake_minimum_required(VERSION 3.20)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.18.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(lfrb_tests test_lfrb.cpp)

target_link_libraries(lfrb_tests
    PRIVATE
        LockFreeRingBuffer
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(lfrb_tests)
