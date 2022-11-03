/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later

    Example: list_modems.cpp
    Description: Print details about all available modems.
*/

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "ezcellular/ezcellular.h"

int main(int argc, char* argv[]) {
    using namespace ezcellular;

    auto mm = ModemManager();
    std::cout << "ModemManager version " << mm.version() << "\n";

    auto modems = mm.available_modems();

    if (modems.empty()) {
        std::cerr << "No modems present." << std::endl;
        return 1;
    }

    std::cout << "Modems: " << modems.size() << std::endl;

    for (const auto& modem : modems) {
        auto state = modem.state();
        auto sim = modem.active_sim();

        std::cout << "Modem:"
            << "\n\tManufacturer/Model: " << modem.manufacturer() << " " << modem.model()
            << "\n\tIMEI:               " << modem.imei()
            << "\n\tFirmware:           " << modem.firmware_version()
            << "\n\tState:              " << state
            << "\n\tlocked:             " << std::boolalpha << modem.locked() << " (" << modem.lock_state() << ")"
            << "\n\tPhone number:       " << modem.phone_number().value_or("<unknown>")
            << "\n\tSIM available:      " << std::boolalpha << (sim ? "yes" : "no")
            << std::endl;
    }

    return 0;
}
