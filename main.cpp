import utility;
#include <fmt/core.h>
#include <iostream>
#include <expected>
auto erroe()
    -> std::expected<void, std::string> {
    return std::unexpected<std::string>{"oooopsie"};
}

int main() {
    fmt::print("hewooo\n");
    try {
        throw traced_exception("oopsie");
    } catch (traced_exception& e){
        std::cout << e.source().file_name() << std::endl;
    }
    return 0;
}