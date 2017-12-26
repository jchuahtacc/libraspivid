cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
message(STATUS "Building libraspivid")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package( MMAL REQUIRED )
find_package( Broadcom REQUIRED )

set(BUILD_LIBRASPIVID_EXAMPLES FALSE CACHE PATH "Build libraspivid example programs")

include_directories("${BROADCOM_INCLUDE_DIRS}")
include_directories("${MMAL_INCLUDE_DIRS}")

set(LIBRASPIVID_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include ${BROADCOM_INCLUDE_DIRS} ${MMAL_INCLUDE_DIRS} CACHE PATH "libraspivid header files")

include_directories("${LIBRASPIVID_INCLUDE_DIRS}")

add_library(raspivid ./src/components/RaspiRenderer.cpp ./src/RaspiCamControl.cpp ./src/components/RaspiComponent.cpp ./src/components/RaspiNullsink.cpp ./src/components/RaspiEncoder.cpp ./src/RaspiPort.cpp ./src/components/RaspiSplitter.cpp ./src/components/RaspiResize.cpp ./src/components/RaspiCamera.cpp ./src/components/RaspiOverlayRenderer.cpp )

target_include_directories(raspivid PUBLIC ${LIBRASPIVID_INCLUDE_DIRS})
target_link_libraries(raspivid ${MMAL_LIBRARIES} pthread)

if (BUILD_LIBRASPIVID_EXAMPLES)
    add_subdirectory(examples)
endif(BUILD_LIBRASPIVID_EXAMPLES)
