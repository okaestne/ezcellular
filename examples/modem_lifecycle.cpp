/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#include "ezcellular/ezcellular.h"

#include <chrono>
#include <csignal>
#include <future>
#include <iostream>
#include <optional>
#include <string>
#include <thread> // std::this_thread

using namespace ezcellular;

bool should_run = true;

static void handle_sigint(int) {
    should_run = false;
}

static void usage(char* argv[]) {
    std::cerr << "Usage: " << argv[0] << " <ACTION>"
        << "\n  ACTION must one of: 'disable', 'enable', 'restart', 'poweroff', 'powerdown', 'powerup'"
        << std::endl;
}

static void modem_state_observer(Modem::ModemState from, Modem::ModemState to) {
    std::cout << "Modem state changed: " << from << "->" << to << "\n";
}

int main(int argc, char* argv[]) {
    ::signal(SIGINT, handle_sigint);  // clean shutdown on SIGINT (Ctrl+C)

    ModemManager mm;
    auto modem = mm.any_modem();

    if (!modem) {
        std::cout << "Waiting for any modem to become available...\n";
        auto modem_future = mm.await_modem(/* default: ANY_IMEI */);
        modem_future.wait();
        auto modem = modem_future.get();  // set value into optional (should be a move)
    }

    std::cout << "Got a modem:"
        << "\n\tIMEI:        " << modem->imei()
        << "\n\tState:       " << modem->state()
        << "\n\tPower State: " << modem->power_state()
        << std::endl;

    // observe general state
    modem->observe_modem_state(modem_state_observer);

    if (argc != 2) {
        usage(argv);
        return 1;
    }
    std::string action = argv[1];

    if (action == "disable") {
        std::cout << "Disabling modem\n";
        modem->enable(false);
    } else if (action == "enable") {
        std::cout << "Enabling modem\n";
        modem->enable(true);
    } else if (action == "restart") {
        std::cout << "Restarting modem\n";
        auto restarted_modem =  mm.reset_modem(*modem);
        restarted_modem.observe_modem_state(modem_state_observer);
    } else if (action == "poweroff") {
        std::cout << "Turning off modem\n";
        modem->power_off();
    } else if (action == "powerdown") {
        std::cout << "Powering down modem\n";
        modem->power_low();
    } else if (action == "poweron") {
        std::cout << "Powering on modem\n";
        modem->power_on();
    } else {
        std::cerr << "Unknown action: " << action << "\n\n";
        usage(argv);
        return 1;
    }

    std::cout << "Done. Waiting for state changes, press Ctrl+C to quit." << std::endl;
    while (should_run) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}
