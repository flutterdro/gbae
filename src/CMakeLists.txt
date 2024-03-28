add_subdirectory(emulator)
add_subdirectory(gui)

target_sources(fgba_exe
    PRIVATE
        main.cpp
)
