message(STATUS "Fetching g3log")

FetchContent_Declare(
        g3log
        GIT_REPOSITORY git@github.com:KjellKod/g3log.git
        GIT_TAG 2.4
)

FetchContent_MakeAvailable(g3log)

