message(STATUS "Building libraspivid_example")
include_directories("${LIBRASPIVID_INCLUDE_DIRS}")
add_executable(libraspivid_example example.cpp)
target_link_libraries(libraspivid_example raspivid)

