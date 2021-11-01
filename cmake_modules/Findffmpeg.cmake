message("Findffmpeg.cmake find ffmpeg lib")
 
set(ffmpegsdk $ENV{DevEnvDir}/ffmpeg)

#find libavcodec
FIND_PATH(ffmpeg_INCLUDE_DIR libavcodec
    ${ffmpegsdk}/include
)
 
 
#find library
# FIND_LIBRARY(ffmpeg_LIBRARY *.lib 
#     ${ffmpegsdk}/lib
# )

FILE(GLOB ffmpeg_LIBRARY ${ffmpegsdk}/lib/*.lib)
 
if(ffmpeg_INCLUDE_DIR AND ffmpeg_LIBRARY)
    set(ffmpeg_FOUND TRUE)
    set(ffmpeg_VERSION 4.4.1)
endif(ffmpeg_INCLUDE_DIR AND ffmpeg_LIBRARY)
