cmake_minimum_required(VERSION 2.8)
project(texpp)

# Enable testing
include(CTest)

# Find boost libraries
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.56.0 COMPONENTS system filesystem regex python-3 REQUIRED)

# Find python interpreter
find_package(PythonInterp)
if(NOT PYTHONINTERP_FOUND)
    message("python interpreter NOT found - tests/hrefkeywords will NOT be built.")
endif(NOT PYTHONINTERP_FOUND)

# Find python 3.4 libraries
set(Python_ADDITIONAL_VERSIONS 3.4)
find_package( PythonLibs 3.4  EXACT REQUIRED)
if(NOT PYTHONLIBS_FOUND)
    message("python libraries NOT found - texpy and hrefkeywords will NOT be built.")
endif(NOT PYTHONLIBS_FOUND)

# Find Unicode libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})
find_package(ICU 4.4 REQUIRED)

# Find tex executable (required for tests/tex and tests/hrefkeywords)
find_program(TEX_EXECUTABLE "tex")
if(TEX_EXECUTABLE STREQUAL "TEX_EXECUTABLE-NOTFOUND")
    message("tex executable NOT found - tests/tex and tests/hrefkeywords will NOT be built.")
else(TEX_EXECUTABLE STREQUAL "TEX_EXECUTABLE-NOTFOUND")
    set(TEX_FOUND ON)
endif(TEX_EXECUTABLE STREQUAL "TEX_EXECUTABLE-NOTFOUND")
mark_as_advanced(TEX_EXECUTABLE)

# Flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wunused -Woverloaded-virtual -Wwrite-strings")
include_directories(${CMAKE_SOURCE_DIR})

# Subdirectories
add_subdirectory(texpp)

if(Boost_FOUND)
    add_subdirectory(texpy)
    add_subdirectory(hrefkeywords)
endif(Boost_FOUND)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif(BUILD_TESTING)


