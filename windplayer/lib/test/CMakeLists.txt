##test_packet_list
ADD_EXECUTABLE(packetlist test_packet_list.cpp)
TARGET_LINK_LIBRARIES (packetlist windplayer)

##test_decoder
ADD_EXECUTABLE(decoder test_decoder.cpp)
TARGET_LINK_LIBRARIES (decoder windplayer)
