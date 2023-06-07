cmake_minimum_required(VERSION 3.22)
project(Sem4Lab3)

add_executable(
    memmngTest 
    ./memmng/memmngTest.c 
    ./memmng/memmng.c 
    ./memmng/memmng.h
)

add_executable(
    main 
    main.c
    ./memmng/memmng.c 
    ./memmng/memmng.h
)
link_libraries(main pthread)

