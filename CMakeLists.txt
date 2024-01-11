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

add_subdirectory(deps)
add_subdirectory(tests)

add_executable(emulator)

add_subdirectory(src/utility)
add_subdirectory(src)

target_compile_options(emulator 
    PUBLIC 
        -fsanitize=address 
        -fsanitize=undefined
        -flto
        -fexperimental-library
)
target_link_options(emulator 
    PUBLIC 
        -fsanitize=address
        -fsanitize=undefined
        -flto
)

target_link_libraries(emulator PUBLIC fmt)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

target_link_libraries(emulator PRIVATE Qt6::Widgets)

enable_testing()