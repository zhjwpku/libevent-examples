add_executable(echosrv echosrv.cc)
target_include_directories(echosrv PRIVATE ${EVENT_INCLUDE_DIR})
target_link_libraries(echosrv PRIVATE ${EVENT_LIBRARIES})

add_executable(buffered-echosrv buffered-echosrv.cc)
target_include_directories(buffered-echosrv PRIVATE ${EVENT_INCLUDE_DIR})
target_link_libraries(buffered-echosrv PRIVATE ${EVENT_LIBRARIES})