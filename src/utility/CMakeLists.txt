
target_sources(emulator
    PUBLIC
    FILE_SET utility TYPE HEADERS FILES
        fatexception.hpp 
        scopeguard.hpp
)