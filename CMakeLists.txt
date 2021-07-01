add_library(libl3gd20h
        src/l3gd20h.cpp
        )

target_include_directories(libl3gd20h PUBLIC inc)
target_link_libraries(libl3gd20h mbed-os)
