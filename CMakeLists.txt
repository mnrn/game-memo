cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
cmake_policy(VERSION 3.5)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# TODO: cmake_template -> your project name
set(PROJECT_NAME memo)
project(${PROJECT_NAME})

# Include dirctories
set(PROJECTS_DIR_NAME projects)
set(TESTS_DIR_NAME test)
include_directories(SYSTEM third-party)
include_directories(include)
include_directories(src)

# Select compiler
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    #set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++ CACHE PATH "" FORCE) 
endif()

# Compile mode
set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG}           -DDEBUG  -g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO, "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}           -g      -O3")
set(CMAKE_CXX_FLAGS_RELEASE,        "${CMAKE_CXX_FLAGS_RELEASE}         -DNDEBUG         -O4 -flto ")
set(CMAKE_CXX_FLAGS_MINSIZEREL,     "${CMAKE_CXX_FLAGS_MINSIZEREL}      -DNDEBUG         -Oz")

# Resolve dependency
# Thread
find_package(Threads REQUIRED)

# Boost
set(Boost_USE_STATIC_LIBS ON)
#set(BOOST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third-party")
set(Boost_LIBRARY_DIRS "${CMAKE_SOURCE_DIR}/lib")
find_package(Boost REQUIRED)
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost!")
else()
    #message("@@ Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message("@@ Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
endif()

# jemalloc
set(JEMALLOC_HOME "${CMAKE_SOURCE_DIR}")
find_package(jemalloc)
if (NOT JEMALLOC_FOUND)
    message(FATAL_ERROR "Could not find jemalloc!")
else()
    message("@@JEMALLOC_SHARED_LIB : ${JEMALLOC_SHARED_LIB}")
    message("@@JEMALLOC_STATIC_LIB : ${JEMALLOC_STATIC_LIB}")
endif()

# OpenSSL
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
endif()
find_package(OpenSSL REQUIRED)
if (NOT OPENSSL_FOUND)
    message(FATAL_ERROR "Could not find OpenSSL!")
else()
    message("@@OPENSSL_INCLUDE_DIR : ${OPENSSL_INCLUDE_DIR}")
    message("@@OPENSSL_LIBRARIES : ${OPENSSL_LIBRARIES}")
    include_directories(SYSTEM "${OPENSSL_INCLUDE_DIR}")
endif()

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# Warning settings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wundef")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wundef")
endif()

add_definitions(-std=c++17)
file(GLOB SOURCE *.cpp src/*.cpp include/*.hpp)

# Function for building
function(build DIR_NAME TARGET_NAME)
    # Main
    file(GLOB SOURCE *.cpp src/*.cpp include/*.hpp ${DIR_NAME}/${TARGET_NAME}/*.cpp ${DIR_NAME}/${TARGET_NAME}/*.hpp)
    set(MAIN_CPP ${TARGET_NAME}/main.cpp)
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${DIR_NAME}/${TARGET_NAME}/main.cpp)
        set(MAIN_CPP ${DIR_NAME}/${TARGET_NAME}/main.cpp)
    endif()

    add_executable(${TARGET_NAME}
        ${MAIN_CPP}
        ${SOURCE}
    )
    target_link_libraries(${TARGET_NAME} 
        ${CMAKE_THREAD_LIBS_INIT}
        ${OPENSSL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${JEMALLOC_SHARED_LIB}
    )
endfunction(build)

# Build All
function(buildAll)
    foreach(TARGET ${TARGETS})
        build(${PROJECTS_DIR_NAME} ${TARGET})
    endforeach(TARGET)
endfunction(buildAll)

# Test All
function(testAll)
    foreach(TEST_TARGET ${TEST_TARGETS})
        build(${TESTS_DIR_NAME} ${TEST_TARGET})
    endforeach(TEST_TARGET)
endfunction(testAll)

#link_libraries()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")

# TODO: hello -> directory that you want build.
set(TARGETS
)
buildAll()

set(TEST_TARGETS
    bit
    sha
    tolerance_compare
    easing
    #stack
    #queue
    #skew_heap
    #avl_tree
    uint8x2_uint16
    checksum
    asio_ping
    asio_ssl_server
    asio_ssl_client
    asio_tcp_client_timeout
    asio_tcp_server_timeout
    asio_read_write
    asio_write_fsm
    asio_write_coro
    asio_tcp_echo_server
    jemalloc_stats_print
    jemallctl
    jemalloc_leak_checking
    #uv_ping_pong
    #uv_tcp_echo_server
    #uv_wget
    beast_http_client
)
testAll()
