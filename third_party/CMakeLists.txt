set(FETCHCONTENT_BASE_DIR "${PROJECT_SOURCE_DIR}/third_party/_deps")

include(FetchContent)

add_subdirectory(yaml-cpp)

add_subdirectory(rest_rpc)

add_subdirectory(g3log)