# Убираем INTERFACE, делаем обычную статическую библиотеку
add_library(SCC STATIC) 

# Для обычной библиотеки используем PRIVATE или PUBLIC вместо INTERFACE
target_sources(SCC PRIVATE ${CMAKE_CURRENT_LIST_DIR}/SCC.c)

target_include_directories(SCC PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(SCC PRIVATE pico_stdlib)
