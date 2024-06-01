
cmake_minimum_required(VERSION 3.28)
project(fgba VERSION 0.1.0.0)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

option(FGBA_DEV_MODE "enable if you are developer" OFF)
option(FGBA_ENABLE_IPO "enables IPO/LTO for release builds" ON)
option(FGBA_BUILD_TESTS "build tests" ON)

include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)
if(result AND FGBA_ENABLE_IPO)
    set_property(
        GLOBAL PROPERTY 
            INTERPROCEDURAL_OPTIMIZATION 
                $<IF:$<CONFIG:RelWithDebInfo,Release>,ON,OFF>
    )
else()
    message(WARNING "IPO/LTO requested but there is no support for one: ${output}")
endif()



add_executable(fgba_exe)
set_target_properties(fgba_exe PROPERTIES OUTPUT_NAME fgba)

add_subdirectory(deps)

add_subdirectory(src)

target_compile_definitions(fgba_exe
    PUBLIC
    $<$<CONFIG:Debug>:
        _LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG
    >
)

target_include_directories(fgba_exe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# target_compile_options(fgba_exe
#     PUBLIC -fexperimental-library
# )
#
target_link_libraries(fgba_exe 
    PRIVATE 
        fmt
        spdlog
        glfw
        glm
        imgui
        glad
)
add_subdirectory(tests)

#------for developers------

if(NOT FGBA_DEV_MODE)
    return()
endif()

include(cmake/compiler-warnings.cmake)
include(cmake/sanitizers.cmake)
include(cmake/static-analyzis.cmake)

option(FGBA_SANITIZE_ADDRESS "dev: enable address sanitizer" ON)
option(FGBA_SANITIZE_LEAK "dev: enable leak sanitizer" OFF)
option(FGBA_SANITIZE_UB "dev: enable undefined behaviour sanitizer" ON)
option(FGBA_SANITIZE_THREAD "dev: enable thread sanitizer" OFF)
option(FGBA_SANITIZE_MEMORY "dev: enable memory sanitizer" OFF)

enable_sanitizers_globaly(
    FGBA_SANITIZE_ADDRESS
    FGBA_SANITIZE_LEAK
    FGBA_SANITIZE_UB
    FGBA_SANITIZE_THREAD
    FGBA_SANITIZE_MEMORY
)


target_enable_static_analysis(fgba_exe)
target_add_warnings(fgba_exe "" "" "" "")
