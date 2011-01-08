ADD_DEFINITIONS(-D__STDC_CONSTANT_MACROS=1)
INCLUDE_DIRECTORIES(
    ../../libutils/
    /home/gitbase/ffmpeg/install/include
    /usr/include
    ./
    )

LINK_DIRECTORIES(
    /home/gitbase/ffmpeg/install/lib
    /usr/lib
)

ADD_LIBRARY(windplayer ${DS_LIBRARY_TYPE} 
    packetList.cpp
    decoder.cpp
    decoder_impl.cpp
    masterclock.cpp
    output.cpp
    videoout.cpp
    audioout.cpp
    windplayer.cpp
    )
TARGET_LINK_LIBRARIES(windplayer
	swscale
	avformat
	avfilter
	avdevice
	avcodec
	avutil
	utils
    pthread
    z
    )

ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(sdl)
