message(STATUS "Fetching rest_rpc")

FetchContent_Declare(
        rest_rpc
        GIT_REPOSITORY https://github.com/qicosmos/rest_rpc.git
        GIT_TAG v0.12
)

FetchContent_MakeAvailable(rest_rpc)
