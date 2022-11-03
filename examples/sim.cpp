/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later

    Example: sim.cpp
    Description: Print details about a SIM card, optionally perform unlock with PIN/PUK
*/

#include "ezcellular/ezcellular.h"

#include <iostream>
#include <optional>
#include <string>

int main(int argc, char* argv[]) {
    using namespace ezcellular;

    /* parse params */
    std::string pin{}, puk{};

    if (argc >= 2) {
        pin = argv[1];
    }
    if (argc == 3) {
        puk = argv[2];
    }

    // init ModemManager, use any available Modem
    ModemManager mm;
    auto modem = mm.any_modem();

    if (!modem) {
        // ... or quit
        std::cerr << "no modem present." << std::endl;
        return 1;
    }

    std::cout << "Using modem: " << modem->manufacturer() << " " << modem->model() << "\n";
    auto sim = modem->active_sim();

    if (!sim) {
        // ... or quit
        std::cerr << "no SIM present." << std::endl;
        return 1;
    }

    // check if locked
    try {
        if (modem->lock_state() == Modem::LockState::SIM_PIN) {
            std::cout << "Modem requires PIN to unlock\n";

            if (pin.empty()) {
                std::cerr << "No PIN provided, bye.\n";
                return 2;
            }
            std::cout << "Sending PIN...\n";
            sim->send_pin(pin);
        }
        if (modem->lock_state() == Modem::LockState::SIM_PUK) {
            std::cout << "Modem requires PUK to unlock\n";

            if (pin.empty() || puk.empty()) {
                std::cerr << "No PIN or PUK provided, bye.\n";
                return 2;
            }
            std::cout << "Sending PUK and new PIN...\n";
            sim->send_puk(puk, pin);
        }
    } catch(SIMException& ex) {
        std::cerr << ex.what() << std::endl;
        return 2;
    }

    // print SIM information
    std::cout << "SIM info:"
        << "\n\tActive:    " << std::boolalpha << sim->active()
        << "\n\tICCID:     " << sim->iccid()
        << "\n\tIMSI:      " << sim->imsi()
        << "\n\tHome PLMN: " << sim->home_plmn()
        << "\n\tOperator:  " << sim->operator_name()
        << std::endl;

    return 0;
}
