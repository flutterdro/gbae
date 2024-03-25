#include <catch2/catch_test_macros.hpp>
#include <ostream>
#include "emulator/cpudefines.hpp"
#include "emulator/cpu/shifter.hpp"

std::ostream& operator<<(std::ostream& stream, fgba::cpu::shifter::shift_res val) {
    stream << '{' << val.shifted_data << ',' << val.carryout << '}';
    return stream;
}
namespace Catch {
template<>
struct StringMaker<fgba::cpu::shifter::shift_res> {
    static auto convert(fgba::cpu::shifter::shift_res const& val) 
        -> std::string {
        ReusableStringStream rss;
        rss << '{' << val.shifted_data << ',' << val.carryout << '}';
        return rss.str();
    }
};
}

TEST_CASE("Shifter test", "[cpu][shifter]") {
    using namespace fgba::cpu;
    REQUIRE(shifter::asr(0x80000000, 3) == 0xf0000000);
    REQUIRE(shifter::asr(0x40000000, 3) == 0x08000000);
    using shifter::shift_res;
    register_manager registers;
    registers.cpsr().reset_ccf(ccf::c);
    fgba::u32 somenum = 0xf6a09cb0; 
    fgba::u32 somenum_lslby17 = somenum << 17;
    fgba::u32 somenum_lsrby17 = somenum >> 17;
    fgba::u32 somenum_lslby1  = somenum << 1;
    fgba::u32 somenum_lsrby1  = somenum >> 1;
    fgba::u32 somenum_asrby17 = shifter::asr(somenum, 17);
    fgba::u32 somenum_asrby1  = shifter::asr(somenum, 1);
    fgba::u32 somenum_rorby17 = somenum << 15 | somenum >> 17;
    fgba::u32 somenum_rorby1  = somenum << 31 | somenum >> 1;
    fgba::u32 shift_info = 0;
    SECTION("Special shifts") {
        REQUIRE(shifter::shift(0, somenum, registers) == somenum);
        REQUIRE(shifter::shiftlsr32(0, somenum, registers) == 0);
        REQUIRE(shifter::shiftasr32(0, somenum, registers) == 0xffffffff);
        registers.cpsr().reset_ccf(ccf::c);
        REQUIRE(shifter::shiftrrx(0, somenum, registers) == 0x7b504e58);
        registers.cpsr().set_ccf(ccf::c);
        REQUIRE(shifter::shiftrrx(0, somenum, registers) == 0xfb504e58);

        REQUIRE(shifter::shift_s(0, somenum, registers) == shift_res{somenum, registers.cpsr().check_ccf(ccf::c)});
        REQUIRE(shifter::shiftlsr32_s(0, somenum, registers) == shift_res{0, 1});
        REQUIRE(shifter::shiftasr32_s(0, somenum, registers) == shift_res{0xffffffff, 1});
        registers.cpsr().reset_ccf(ccf::c);
        REQUIRE(shifter::shiftrrx_s(0, somenum, registers) == shift_res{0x7b504e58, 0});
        registers.cpsr().set_ccf(ccf::c);
        REQUIRE(shifter::shiftrrx_s(0, somenum, registers) == shift_res{0xfb504e58, 0});
        registers.cpsr().reset_ccf(ccf::c);
        REQUIRE(shifter::shiftrrx_s(0, somenum + 1, registers) == shift_res{0x7b504e58, 1});
        registers.cpsr().set_ccf(ccf::c);
        REQUIRE(shifter::shiftrrx_s(0, somenum + 1, registers) == shift_res{0xfb504e58, 1});
    }

    SECTION("Register specified shift") {
        SECTION("No status update") {
            SECTION("Logical shift left") {
                auto&& shift_amount = registers[0];
                shift_amount = 0xff00;
                REQUIRE(shifter::shiftrslsl(shift_info, somenum, registers) == somenum);
                shift_amount = 0xff;
                REQUIRE(shifter::shiftrslsl(shift_info, somenum, registers) == 0);
                shift_amount = 32;
                REQUIRE(shifter::shiftrslsl(shift_info, somenum, registers) == 0);
                shift_amount = 17;
                REQUIRE(shifter::shiftrslsl(shift_info, somenum, registers) == somenum_lslby17);
                shift_amount = 1;
                REQUIRE(shifter::shiftrslsl(shift_info, somenum, registers) == somenum_lslby1);
            }
            SECTION("Logical shift right") {
                auto&& shift_amount = registers[0];
                shift_amount = 0xff00;
                REQUIRE(shifter::shiftrslsr(shift_info, somenum, registers) == somenum);
                shift_amount = 0xff;
                REQUIRE(shifter::shiftrslsr(shift_info, somenum, registers) == 0);
                shift_amount = 32;
                REQUIRE(shifter::shiftrslsr(shift_info, somenum, registers) == 0);
                shift_amount = 17;
                REQUIRE(shifter::shiftrslsr(shift_info, somenum, registers) == somenum_lsrby17);
                shift_amount = 1;
                REQUIRE(shifter::shiftrslsr(shift_info, somenum, registers) == somenum_lsrby1);
            } 
            SECTION("Arithmetic shift right") {
                auto&& shift_amount = registers[0];
                shift_amount = 0xff00;
                REQUIRE(shifter::shiftrsasr(shift_info, somenum, registers) == somenum);
                shift_amount = 0xff;
                REQUIRE(shifter::shiftrsasr(shift_info, somenum, registers) == 0xffffffff);
                shift_amount = 32;
                REQUIRE(shifter::shiftrsasr(shift_info, somenum, registers) == 0xffffffff);
                shift_amount = 17;
                REQUIRE(shifter::shiftrsasr(shift_info, somenum, registers) == somenum_asrby17);
                shift_amount = 1;
                REQUIRE(shifter::shiftrsasr(shift_info, somenum, registers) == somenum_asrby1);
            }
            SECTION("Rotate right") {
                auto&& shift_amount = registers[0];
                shift_amount = 0xff00;
                REQUIRE(shifter::shiftrsror(shift_info, somenum, registers) == somenum);
                shift_amount = 32;
                REQUIRE(shifter::shiftrsror(shift_info, somenum, registers) == somenum);
                shift_amount = 64;
                REQUIRE(shifter::shiftrsror(shift_info, somenum, registers) == somenum);
                shift_amount = 1;
                REQUIRE(shifter::shiftrsror(shift_info, somenum, registers) == somenum_rorby1);
                shift_amount = 33;
                REQUIRE(shifter::shiftrsror(shift_info, somenum, registers) == somenum_rorby1);
                shift_amount = 17;
                REQUIRE(shifter::shiftrsror(shift_info, somenum, registers) == somenum_rorby17);
                shift_amount = 49;
                REQUIRE(shifter::shiftrsror(shift_info, somenum, registers) == somenum_rorby17);
            }
        }
    }
}