add_library(SCC INTERFACE)
target_link_libraries(SCC INTERFACE pico_stdlib )
target_sources(SCC INTERFACE ${CMAKE_CURRENT_LIST_DIR}/SCC.c)
target_include_directories(SCC INTERFACE ${CMAKE_CURRENT_LIST_DIR})

