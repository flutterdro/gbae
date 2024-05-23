target_sources(fgba_exe 
    PRIVATE
        mmu/mmu.cpp
        # cpu/arm7tdmi.cpp
        # cpu/bus.cpp
        cpu/disassembler/disassembler.cpp
        # cpu/registermanager.cpp
        gbaemu.cpp
        # cpu/instruction-implementation/implementation.cpp
        ppu/ppu.cpp
)
add_subdirectory(cpu)
target_link_libraries(fgba_exe PUBLIC fgba::cpu)
