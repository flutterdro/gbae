target_sources(emulator 
    PRIVATE
        FILE_SET headers TYPE HEADERS FILES
            defines.hpp
            gbaemu.hpp
            cpu/arm7tdmi.hpp
            cpu/bus.hpp
            cpu/disassembler/disassembler.hpp
            mmu/mmu.hpp
            cpu/registermanager.hpp
    PRIVATE
        mmu/mmu.cpp
        cpu/arm7tdmi.cpp
        cpu/bus.cpp
        cpu/disassembler/disassembler.cpp
        cpu/registermanager.cpp
        gbaemu.cpp
)
