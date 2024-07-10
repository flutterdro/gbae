
# set(FMT_MODULE ON CACHE BOOL "I am going all in on modules" FORCE)
add_subdirectory(fmt)
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "I already have fmt" FORCE)
add_subdirectory(spdlog)
add_subdirectory(Catch2)
add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(mp11)

add_library(glad)
target_include_directories(glad
    PUBLIC
        $<$<CONFIG:DEBUG>:${CMAKE_CURRENT_LIST_DIR}/glad/debug/include>
        $<$<CONFIG:RELEASE>:${CMAKE_CURRENT_LIST_DIR}/glad/release/include>
)
target_sources(glad 
    PRIVATE
        $<$<CONFIG:DEBUG>:${CMAKE_CURRENT_LIST_DIR}/glad/debug/src/gl.c>
        $<$<CONFIG:RELEASE>:${CMAKE_CURRENT_LIST_DIR}/glad/release/src/gl.c>
)

add_library(imgui 
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_internal.h
    imgui/imgui.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui 
    PUBLIC 
        imgui
        imgui/backends
)

target_link_libraries(imgui PRIVATE glfw)


