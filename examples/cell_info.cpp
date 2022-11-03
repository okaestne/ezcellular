/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later

    Example: cell_info.cpp
    Description: Print details cells that the modems receives
*/

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "ezcellular/ezcellular.h"

int main(int argc, char* argv[]) {
    using namespace ezcellular;

    auto mm = ModemManager();
    auto modem = mm.any_modem();

    if(!modem.has_value()) {
        std::cerr << "No modem present." << std::endl;
        return 1;
    }

    std::cout << "Modem:"
        << "\n\tManufacturer/Model: " << modem->manufacturer() << " " << modem->model()
        << "\n\tIMEI: " << modem->imei()
        << "\n\tState: " << modem->state()
        << "\n\tlocked: " << std::boolalpha << modem->locked()
        << " (" << modem->lock_state() << ")"
        << std::endl;

    if (modem->state() < Modem::ModemState::REGISTERED) {
        std::cerr << "Modem is not ready, needs to be registered in a network." << std::endl;
        return 2;
    }

    // location
    std::cout << "Cell Info:\n";
    auto cell_info = modem->cell_info();

    for (const auto& cell : cell_info) {
        std::cout << cell << std::endl;
    }

    return 0;
}
