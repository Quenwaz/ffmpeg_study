cmake_minimum_required (VERSION 3.18)

project (ffmpeg_study VERSION 0.1.0 LANGUAGES C CXX)

include (InstallRequiredSystemLibraries)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# include(CTest)
# enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake_modules)

option(OPT1 "description" ON)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPU_ARCHITECTURE 64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(CPU_ARCHITECTURE 32)
endif()
message(STATUS "CPU_ARCHITECTURE=${CPU_ARCHITECTURE}")

macro(find_package_ packagename)
    find_package(${packagename} REQUIRED MODULE)
    if (${packagename}_FOUND)
        message(STATUS "${packagename}_INCLUDE_DIR:${${packagename}_INCLUDE_DIR}")
        message(STATUS "${packagename}_LIBRARY:${${packagename}_LIBRARY}")
    else(${packagename}_FOUND)
        message(FATAL_ERROR "Not Found:${packagename}")
    endif(${packagename}_FOUND)
endmacro(find_package_)

find_package_(ffmpeg)
find_package_(SDL)

include(utils)

GetSubDirectory(SubDirList ${CMAKE_CURRENT_LIST_DIR}/ffmpeg_study)
foreach(child ${SubDirList})
    add_subdirectory(ffmpeg_study/${child})
endforeach()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# ADD_CUSTOM_COMMAND(
#           TARGET targetname 
#           POST_BUILD
#           COMMAND cp ./someobject  ../
# ) -