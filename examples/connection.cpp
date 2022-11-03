/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#include <iostream>
#include <optional>
#include <string>

#include "ezcellular/ezcellular.h"

int main(int argc, char* argv[]) {
    using namespace ezcellular;

    std::string apn;
    auto ip_type = IPType::IPV4;

    if (argc >= 2) {
        if (argv[1] == std::string{"--help"}) {
            std::cout
                << "Usage: " << argv[0] << " [<APN> [ 4 | 6 | 64 ] ]\n"
                << "       " << argv[0] << " [--help]\n";
            return 1;
        }
        apn = argv[1];
    }
    if (argc == 3) {
        auto ip_ver = std::stoi(argv[2]);
        if (ip_ver == 4)
            ip_type = IPType::IPV4;
        else if (ip_ver == 6)
            ip_type = IPType::IPV6;
        else if (ip_ver == 64)
            ip_type = IPType::IPV4_AND_IPV6;
        else {
            std::cerr << "unknown ip type '" << ip_ver << "' (allowed values: 4, 6, 64)\n";
            return 1;
        }
    }

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

    if (!modem->connected()) {
        if (apn.empty()) {
            std::cerr << "Error: not connected. Pass an APN as argument to connect." << std::endl;
            return 1;
        }
        std::cout << "Connecting to APN '" << apn << "' with IP type '" << ip_type << "'...\n";
        modem->connect(apn, ip_type);
    }

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

    return 0;
}
