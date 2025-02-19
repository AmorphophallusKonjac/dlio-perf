set(FETCHCONTENT_BASE_DIR "${PROJECT_SOURCE_DIR}/tests/_deps")

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/6910c9d9165801d8827d628cb72eb7ea9dd538c5.zip
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

FetchContent_MakeAvailable(googletest)


enable_testing()

include(GoogleTest)

add_executable(hello_test hello_test.cc)
target_link_libraries(hello_test GTest::gtest_main)
gtest_discover_tests(hello_test)
