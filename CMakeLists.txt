cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")


# TODO: cmake_template -> your project name
set(PROJECT_NAME utils)
project(${PROJECT_NAME})


# Include dirctories
set(PROJECTS_DIR_NAME projects)
set(TESTS_DIR_NAME test)
include_directories(external)
include_directories(include)
include_directories(src)


# Select compilers
set(CMAKE_C_COMPILER /usr/local/opt/llvm/bin/clang CACHE PATH "" FORCE) 
set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++ CACHE PATH "" FORCE) 

# Compile mode
set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG}           -DDEBUG  -g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO, "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}           -g      -O3")
set(CMAKE_CXX_FLAGS_RELEASE,        "${CMAKE_CXX_FLAGS_RELEASE}         -DNDEBUG         -O4 -flto ")
set(CMAKE_CXX_FLAGS_MINSIZEREL,     "${CMAKE_CXX_FLAGS_MINSIZEREL}      -DNDEBUG         -Oz")

# Resolve dependency
find_package(Threads REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system filesystem thread REQUIRED)
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
else()
    include_directories(${Boost_INCLUDE_DIRS})
endif()


# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")


# Warning settings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wundef")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything")
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat -Wno-c++98-compat-pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-documentation")
endif()


add_definitions(-std=c++17)
file(GLOB SOURCE *.cpp src/*.cpp include/*.hpp)


# Function for building
function(build DIR_NAME TARGET_NAME)
    # Main
    file(GLOB SOURCE *.cpp src/*.cpp include/*.hpp ${DIR_NAME}/${TARGET_NAME}/*.cpp)
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

#link_libraries(${} ${})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")

# TODO: hello -> directory that you want build.
set(TARGETS
    hello
)
buildAll()

set(TEST_TARGETS
)
testAll()
