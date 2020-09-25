cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
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
set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++ CACHE PATH "" FORCE) 


# Compile mode
set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG}           -DDEBUG  -g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO, "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}           -g      -O3")
set(CMAKE_CXX_FLAGS_RELEASE,        "${CMAKE_CXX_FLAGS_RELEASE}         -DNDEBUG         -O4 -flto ")
set(CMAKE_CXX_FLAGS_MINSIZEREL,     "${CMAKE_CXX_FLAGS_MINSIZEREL}      -DNDEBUG         -Oz")


# Resolve dependency
find_package(Threads REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_LIBRARY_DIRS "${CMAKE_SOURCE_DIR}/lib/boost")
find_package(Boost COMPONENTS container REQUIRED)
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost!")
else()
    #message("@@ Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message("@@ Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
endif()
set(JEMALLOC_HOME "${CMAKE_SOURCE_DIR}")
find_package(jemalloc)
if (NOT JEMALLOC_FOUND)
    message(FATAL_ERROR "Could not find jemalloc!")
else()
    message("@@JEMALLOC_SHARED_LIB : ${JEMALLOC_SHARED_LIB}")
    message("@@JEMALLOC_STATIC_LIB : ${JEMALLOC_STATIC_LIB}")
endif()

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")


# Warning settings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wundef")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat -Wno-c++98-compat-pedantic")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-documentation")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wundef")
endif()


add_definitions(-std=c++20)
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
    stack
    queue
    skew_heap
    avl_tree
    two_timers_parallel
    daytime_tcp_client
    daytime_tcp_server
    daytime_udp_client
    daytime_udp_server
    two_async_daytime_servers
)
testAll()
