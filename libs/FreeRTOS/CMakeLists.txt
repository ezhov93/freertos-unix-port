file(GLOB FREERTOS_SOURCES
    *.c
    include/*.h
    portable/*.c
    portable/*.h
    portable/utils/*.c
    portable/utils/*.h
)

add_library(FreeRTOS ${FREERTOS_SOURCES})
target_include_directories(FreeRTOS PUBLIC include portable)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(FreeRTOS PRIVATE Threads::Threads)