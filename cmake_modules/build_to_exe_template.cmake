cmake_minimum_required (VERSION 3.18)

GetCurrentFolderName(FolderName)
project (${FolderName} VERSION 0.1.0 LANGUAGES C CXX)

aux_source_directory(. DIR_SRCS)

if (GUI)
    message(STATUS "build to GUI program")
    add_executable(${PROJECT_NAME} WIN32 ${DIR_SRCS})
else()
    add_executable(${PROJECT_NAME} ${DIR_SRCS})
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${ffmpeg_INCLUDE_DIR} ${SDL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${ffmpeg_LIBRARY} ${SDL_LIBRARY})