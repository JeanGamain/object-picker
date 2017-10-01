cmake_minimum_required(VERSION 3.1)

project(objectpicker)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -O3 -g3")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(EXECUTABLE_OUTPUT_PATH ../bin/)

set(${PROJECT_NAME}_src
	./src/main.cpp
	./src/gaussian.cpp
	./src/canny.cpp
	./src/convolution.cpp
	)

set(objectpicker_includes_directories
	src/
	)

foreach(DIRECTORY ${jmax_includes_directories}) 
    # Get the directory of the source file
    FILE(GLOB tmp RELATIVE "${CMAKE_SOURCE_DIR}/" ${DIRECTORY}/*.hpp)
    set(objectpicker_includes ${server_includes} ${tmp})
endforeach()


add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_src} ${objectpicker_includes})

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${EXECUTABLE_OUTPUT_PATH}")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${EXECUTABLE_OUTPUT_PATH}")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_range_for)
target_include_directories(${PROJECT_NAME} PRIVATE ${server_includes_directories})

if (APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  endif()
endif(APPLE)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/" ${CMAKE_MODULE_PATH})

FILE(GLOB_RECURSE all_files *.*pp)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL REQUIRED sdl)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL_LIBRARIES})

find_package(LIBVLC REQUIRED)
if (LIBVLC_FOUND)
    include_directories(${LIBVLC_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${LIBVLC_LIBRARY})
endif()

foreach(FILE ${all_files}) 
    # Get the directory of the source file
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

    # Remove common directory prefix to make the group
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

    # Make sure we are using windows slashes
    string(REPLACE "/" "\\" GROUP "${GROUP}")

    # Group into "Source Files" and "Header Files"
    if ("${FILE}" MATCHES ".*\\.cpp")
       set(GROUP "Source Files${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.hpp")
       set(GROUP "Header Files${GROUP}")
    endif()

    source_group("${GROUP}" FILES "${FILE}")
endforeach()

