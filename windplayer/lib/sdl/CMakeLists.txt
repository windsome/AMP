INCLUDE_DIRECTORIES(
    /usr/include/SDL
	.
    )

##sdlplayer
ADD_EXECUTABLE(sdlplayer 
    sdl_abstract.cpp
    sdl_videoouter.cpp
    sdl_audioouter.cpp
    sdl_main.cpp
    )
TARGET_LINK_LIBRARIES (sdlplayer windplayer SDL)


#tutorial
ADD_EXECUTABLE(tutorial tutorial07.c)
TARGET_LINK_LIBRARIES (tutorial 
	swscale
	avformat
	avfilter
	avdevice
	avcodec
	avutil
    SDL
)
