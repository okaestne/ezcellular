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

    ::signal(SIGINT, handle_sigint);  // clean shutdown on SIGINT (Ctrl+C)

    auto mm = ModemManager();
    auto modem = mm.any_modem();

    if (!modem) {
        std::cerr << "Error: no modem present." << std::endl;
        return 1;
    }

    std::cout << "Modem #0:"
        << "\n\tManufacturer/Model: " << modem->manufacturer() << " " << modem->model()
        << "\n\tIMEI: " << modem->imei()
        << "\n\tState: " << modem->state()
        << "\n\tlocked: " << std::boolalpha << modem->locked() << " (" << modem->lock_state() << ")"
        << std::endl;

    modem->observe_modem_state([](Modem::ModemState old_, Modem::ModemState new_) {
        std::cout << "Modem state changed: " << old_ << "->" << new_ << std::endl;
    });

    auto conn = modem->active_connection();

    if (!conn) {
        std::cerr << "Error: not connected.\n";
        return 1;
    }

    std::cout << "Active Connection:"
        << "\n\tOperator: " << modem->operator_name() << " (" << modem->operator_plmn() << ")"
        << "\n\tSettings: APN: " << conn->apn() << ", IP-Type: " << conn->ip_type()
        << "\n\tInterface: " << conn->linux_interface();

    auto ip4 = conn->ipv4_config();
    auto ip6 = conn->ipv6_config();

    if (ip4) {
        std::cout << "\n\t\tIPv4: " << ip4->address << "/" << ip4->prefix
            << "\n\t\tGateway: " << ip4->gateway
            << "\n\t\tDNS: " << ip4->dns1 << ", " << ip4->dns2;
    }
    if (ip6) {
        std::cout << "\n\t\tIPv6: " << ip6->address << "/" << ip6->prefix
            << "\n\t\tGateway: " << ip6->gateway
            << "\n\t\tDNS: " << ip6->dns1 << ", " << ip6->dns2;
    }
    std::cout << std::endl;

    conn->observe_traffic_stats([](TrafficStats stats) {
        std::cout << "Traffic stats: " << stats << "\n";
    }, 2000 /* interval ms */);

    while (should_run) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}
