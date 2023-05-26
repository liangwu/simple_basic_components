cmake_minimum_required(VERSION 3.22.1)
project(lib)

# file(GLOB LIB_SRC ${PROJECT_SOURCE_DIR}/*.c ${PROJECT_SOURCE_DIR}/*.h)
set(ARRAY_LOCK_FREE_QUEUE_SRC ArrayLockFreeQueue.cpp ArrayLockFreeQueue.h)
set(SKIPLIST_SRC skiplist.c skiplist.h)
set(LIB_SRC ${SKIPLIST_SRC} ${ARRAY_LOCK_FREE_QUEUE_SRC})

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib)
add_library(util SHARED ${LIB_SRC})