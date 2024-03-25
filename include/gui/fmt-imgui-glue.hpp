#ifndef FMT_IMGUI_GLUE_HPP_DNIQSNZAL
#define FMT_IMGUI_GLUE_HPP_DNIQSNZAL

#include "imgui.h"
#include "fmt/core.h"

namespace ImGui {
template <typename... Ts>
IMGUI_API void TextFmt(fmt::format_string<Ts...> fmt, Ts&&... args) {
    std::string str = fmt::format(fmt, std::forward<Ts>(args)...);
    ImGui::TextUnformatted(&*str.begin(), &*str.end());
}
template <typename... Ts>
IMGUI_API void vTextFmt(fmt::string_view fmt, Ts&&... args) {
    std::string str = fmt::vformat(fmt, fmt::make_format_args(args...));
    ImGui::TextUnformatted(&*str.begin(), &*str.end());
}
}

#endif