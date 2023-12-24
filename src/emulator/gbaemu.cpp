export module fgba;

import module fgba.cpu;
import fgba.mmu;

export namespace fgba {
class gameboy_advance {
public:
private:
    cpu::arm7tdmi m_cpu;
    mmu::memory_managment_unit m_mmu;
};
}