/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
    
    Example: time.cpp
    Description: Print details about the current time, received from the network.
*/
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "ezcellular/ezcellular.h"

int main(int argc, char* argv[]) {
    using namespace ezcellular;

    // init ModemManager, use any available Modem
    auto mm = ModemManager();
    auto modem = mm.any_modem();

    if (!modem) {
        // ... or quit
        std::cerr << "No modem present." << std::endl;
        return 1;
    }

    if (!modem->registered()) {
        std::cerr << "Modem is not ready, needs to be registered in a network." << std::endl;
        return 2;
    }

    // fetch an print time from modem
    std::cout << "Network time: " << modem->network_time()
        << " (unix timestamp: " << modem->network_time_epoch() << ")" << std::endl;

    return 0;
}
