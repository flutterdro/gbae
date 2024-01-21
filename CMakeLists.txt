cmake_minimum_required(VERSION 3.28)
project(gbae VERSION 0.1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

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

set(FGBA_WARNING_FLAGS 
    -Wall
    -Wextra
    -Wshadow
    -Wmove
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
)
set(FGBA_SAFETY_FLAGS 
    -fsanitize=address 
    -fsanitize=undefined 
    -fstack-protector-strong 
    -fstack-clash-protection
)

add_executable(emulator)
add_subdirectory(deps)
add_subdirectory(tests)

add_subdirectory(src/utility)
add_subdirectory(src)

target_compile_definitions(emulator
    PUBLIC
    $<$<CONFIG:Debug>:
        _LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG
    >
)
target_compile_options(emulator
    PUBLIC "$<$<CONFIG:Debug>:${FGBA_SAFETY_FLAGS}>"
    PRIVATE "$<$<CONFIG:Debug>:${FGBA_WARNING_FLAGS}>"
    PUBLIC $<$<CONFIG:Release>: -flto>
    PUBLIC -fexperimental-library
)
target_link_options(emulator 
    PUBLIC 
    $<$<CONFIG:Debug>:
        -fsanitize=address 
        -fsanitize=undefined
    >
    PUBLIC $<$<CONFIG:Release>:
        -flto
    >
)

target_link_libraries(emulator 
    PRIVATE 
        fmt
        spdlog
        glfw
        glm
        imgui
        glad
)


enable_testing()