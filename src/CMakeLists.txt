add_subdirectory(emulator)
add_subdirectory(gui)

target_sources(emulator
    PRIVATE
        main.cpp
)
