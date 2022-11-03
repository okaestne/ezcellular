/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#include <chrono>
#include <csignal>
#include <iostream>
#include <optional>
#include <string>
#include <thread> // std::this_thread

#include "ezcellular/ezcellular.h"

bool should_run = true;

void handle_sigint(int) {
    should_run = false;
}

int main(int argc, char* argv[]) {
    using namespace ezcellular;

    ::signal(SIGINT, handle_sigint);  // clean shutdown

    auto mm = ModemManager();
    auto modem = mm.any_modem();

    if(!modem) {
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

    if (!modem->registered()) {
        std::cerr << "Modem is not ready, needs to be registered in a network." << std::endl;
        return 2;
    }

    // RAT
    std::cout << "Current technology: " <<  modem->technology() << "\n";

    // signal quality
    auto signal = modem->signal();
    std::cout << "Current signal quality: " << signal << "\n";

    // observe signal
    modem->observe_signal([](Signal sq) {
        std::cout << "Signal quality update: " << sq << std::endl;
    }, 2);

    // keep alive
    while (should_run) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}
