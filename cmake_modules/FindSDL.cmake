message("FindSDL.cmake find SDL lib")
 
set(SDLsdk E:/DevEnvs/Cpp/SDL2-2.0.16)

#find SDL
FIND_PATH(SDL_INCLUDE_DIR SDL
    ${SDLsdk}/include
)
 
#find library
# FIND_LIBRARY(SDL_LIBRARY *.lib 
#     ${SDLsdk}/lib/x${CPU_ARCHITECTURE}
# )

file(GLOB SDL_LIBRARY ${SDLsdk}/lib/x${CPU_ARCHITECTURE}/*.lib)
 
if(SDL_INCLUDE_DIR AND SDL_LIBRARY)
    set(SDL_FOUND TRUE)
    set(SDL_VERSION 2.0.16)
endif(SDL_INCLUDE_DIR AND SDL_LIBRARY)
