#ifndef LOW_LEVEL_ARITHM_HPP_
#define LOW_LEVEL_ARITHM_HPP_

#ifdef _MSC_VER
#else 
[[nodiscard]]constexpr auto addv_impl(auto operand1, auto operand2, auto& res) noexcept
    -> bool {
#if __has_builtin(__builtin_add_overflow)
    return __builtint_add_overflow(operand1, operand2, res);
#endif
}
#endif


#endif