target_sources(emulator 
    PRIVATE
        FILE_SET gui TYPE HEADERS FILES
            backend/shader.hpp
    PRIVATE
        backend/shader.cpp
)