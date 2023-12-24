
target_sources(emulator
    PUBLIC
    FILE_SET modules TYPE CXX_MODULES FILES
        fatexception.cpp
        initializer_list.cpp
        span.cpp
        vector.cpp
)