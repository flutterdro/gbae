cmake_minimum_required(VERSION 3.28)
project(gbae VERSION 0.1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CONFIGURATION_TYPES) 
    if(NOT "Profile" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Profile)
    endif()
else()
    set(allowableBuildTypes Debug Release RelWithDebInfo Profile)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
                 STRINGS "${allowableBuildTypes}")   
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
    elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowableBuildTypes)
        message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
    endif()
endif()

add_subdirectory(third_party)
add_subdirectory(tests)

add_executable(emulator)

target_sources(emulator 
    PUBLIC
        FILE_SET modules TYPE CXX_MODULES FILES
            utility.cpp
)
add_subdirectory(src/fuckstd)
add_subdirectory(src)

target_compile_options(emulator 
    PUBLIC 
        -fsanitize=address 
        -fsanitize=undefined
)
target_link_options(emulator 
    PUBLIC 
        -fsanitize=address
)

# target_link_libraries(emulator)


enable_testing()