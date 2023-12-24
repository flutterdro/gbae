target_sources(emulator 
    PUBLIC
        FILE_SET modules TYPE CXX_MODULES FILES
            gbaemu.cpp
            cpu/arm7tdmi.cpp
            cpu/disassembler/disassembler.cpp
            mmu/mmu.cpp
)
