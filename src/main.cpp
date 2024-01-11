
#include <memory>
#include <source_location>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <ranges>
#include <expected>

#include <fmt/core.h>

#include "utility/fatexception.hpp"
#include "emulator/gbaemu.hpp"
#include "emulator/cpu/disassembler/disassembler.hpp"
#include <QApplication>
#include <QPushButton>

int main(int argc, char* argv[]) {
    fgba::gameboy_advance gba;
    QApplication app(argc, argv);
    auto window_ptr = std::make_unique<QWidget>();
    window_ptr->setFixedSize(100, 50);
    auto button = new QPushButton("Hello World", window_ptr.get());
    button->setGeometry(10, 10, 80, 30);
    window_ptr->show();
    return app.exec();
} 