#ifndef FGBA_INSTRUCTION_FLAGS_HPP_WKJNPKJNQS
#define FGBA_INSTRUCTION_FLAGS_HPP_WKJNPKJNQS

namespace fgba::cpu {

enum class immediate_operand : unsigned{
    off = 0,
    on  = 1,
    
    count,
};
enum class s_bit : unsigned {
    off = 0,
    on  = 1,

    count,
};
enum class accumulate : unsigned { off, on, count, };

//I can't spell this word
enum class mll_signedndesd {
    signed_, //NOLINT
    unsigned_, //NOLINT

    count,
};

enum class which_psr : unsigned {
    cpsr = 0,
    spsr,

    count,
};

enum class direction {
    up,
    down,

    count,
};

enum class mask {
    off,
    on, 

    count,
};

enum class indexing {
    pre,
    post,

    count,
};

enum class write_back {
    off,
    on,

    count,
};


}

#endif
