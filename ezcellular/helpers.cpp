/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#include "helpers.h"

#include <cstdint>  // uint32_t, ...
#include <memory>   // std::shared_ptr
#include <optional> // std::optional
#include <string>   // std::string, operator<<

#include "enums.h"

namespace ezcellular {

auto operator<<(std::ostream& os, const Technology& tech)->std::ostream& {
    switch (tech) {
        case Technology::GSM:  os << "GSM"; break;
        case Technology::UMTS: os << "UMTS"; break;
        case Technology::LTE:  os << "LTE"; break;
        case Technology::NR5G: os << "NR5G"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}

auto operator<<(std::ostream& os, const IPType& type) -> std::ostream& {
    switch (type) {
        case IPType::IPV4: os << "IPv4"; break;
        case IPType::IPV6: os << "IPv6"; break;
        case IPType::IPV4_AND_IPV6: os << "IPv4+IPv6"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}

using ModemState = Modem::ModemState;
auto operator<<(std::ostream& os, const ModemState& state) -> std::ostream& {
    switch (state) {
        case ModemState::FAILED: os << "FAILED"; break;
        case ModemState::INITIALIZING: os << "INITIALIZING"; break;
        case ModemState::LOCKED: os << "LOCKED"; break;
        case ModemState::DISABLED: os << "DISABLED"; break;
        case ModemState::DISABLING: os << "DISABLING"; break;
        case ModemState::ENABLING: os << "ENABLING"; break;
        case ModemState::ENABLED: os << "ENABLED"; break;
        case ModemState::SEARCHING: os << "SEARCHING"; break;
        case ModemState::REGISTERED: os << "REGISTERED"; break;
        case ModemState::DISCONNECTING: os << "DISCONNECTING"; break;
        case ModemState::CONNECTING: os << "CONNECTING"; break;
        case ModemState::CONNECTED: os << "CONNECTED"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}

using PowerState = Modem::PowerState;
auto operator<<(std::ostream& os, const PowerState& state) -> std::ostream& {
    switch (state) {
        case PowerState::OFF: os << "OFF"; break;
        case PowerState::LOW: os << "LOW"; break;
        case PowerState::ON: os << "ON"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}

using LockState = Modem::LockState;
auto operator<<(std::ostream& os, const LockState& state) -> std::ostream& {
    switch (state) {
        case LockState::UNLOCKED: os << "UNLOCKED"; break;
        case LockState::SIM_PIN: os << "SIM_PIN"; break;
        case LockState::SIM_PIN2: os << "SIM_PIN2"; break;
        case LockState::SIM_PUK: os << "SIM_PUK"; break;
        case LockState::SIM_PUK2: os << "SIM_PUK2"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}

/* ------- structs ------- */

/**
 * helper method
 */
static auto join_string(const std::vector<std::string>& parts, const std::string& delim = ", ") -> std::string {
    if (parts.empty()) {
        return {};
    }
    if (parts.size() == 1) {
        return parts[0];
    }

    std::string concated{};
    for (const auto& part : parts) {
        concated.append(part).append(delim);
    }
    return concated.erase(concated.length() - delim.length(), delim.length()); // remove last delim
}

auto operator<<(std::ostream& os, const SignalBase* sq)->std::ostream& {
    os << R"("signal": )";

    if (sq == nullptr) {
        os << "null";
        return os;
    }

    // concat values
    // assumes that all values are doubles!!
    std::vector<std::string> parts;
    for (const auto& [key, value] : *sq) {
        if (key != "tech") {
            parts.emplace_back(std::string{"\""} + key + "\": " + std::to_string(std::any_cast<double>(value)));
        }
    }
    os << "{" << join_string(parts) << "}";
    return os;
}

auto operator<<(std::ostream& os, const Signal& sq) -> std::ostream& {
    os << sq.get();
    return os;
}

auto operator<<(std::ostream& os, const LocationBase* loc) -> std::ostream& {
    os << R"("location": )";

    if (loc == nullptr) {
        os << "null";
        return os;
    }

    std::vector<std::string> parts;

    if (loc->has_key("mcc")) {
        parts.push_back(R"("mcc": ")" + loc->mcc() + '"');
    }
    if (loc->has_key("mnc")) {
        parts.push_back(R"("mnc": ")" + loc->mnc() + '"');
    }
    if (loc->has_key("ci")) {
        parts.push_back(R"("ci": )" + std::to_string(loc->ci()));
    }

    switch (loc->tech()) {
        case Technology::LTE:
        case Technology::NR5G: {
                auto lte_nr = static_cast<const LocationLTE&>(*loc); // both compatible
                if (lte_nr.has_key("tac")) {
                    parts.push_back(R"("tac": )" + std::to_string(lte_nr.tac()));
                }
                break;
            }
        default:
            break;
    }

    os << "{" << join_string(parts) << "}";
    return os;
}

auto operator<<(std::ostream& os, const Location& loc) -> std::ostream& {
    os << loc.get();
    return os;
}

auto operator<<(std::ostream& os, const CellInfoBase* ci) -> std::ostream& {
    os << "\"cell_info\": ";

    if (ci == nullptr) {
        os << "null";
        return os;
    }

    // common
    os << "{\n  \"serving\": " << std::boolalpha << ci->serving();

    switch (ci->tech()) {
        case Technology::LTE: {
                auto lte = static_cast<const CellInfoLTE&>(*ci);
                if (lte.signal()) {
                    os << ",\n  " << lte.signal();
                }
                if (lte.location()) {
                    os << ",\n  " << lte.location();
                }
                if (lte.has_key("pci")) {
                    os << ",\n  \"pci\": " << lte.pci();
                }
                if (lte.has_key("earfcn")) {
                    os << ",\n  \"earfcn\": " << lte.earfcn();
                }
                break;
            }
        case Technology::NR5G: {
                auto nr = static_cast<const CellInfoNR5G&>(*ci);
                if (nr.signal()) {
                    os << ",\n  " << nr.signal();
                }
                if (nr.location()) {
                    os << ",\n  " << nr.location();
                }
                if (nr.has_key("pci")) {
                    os << ",\n  \"pci\": " << nr.pci();
                }
                if (nr.has_key("nrarfcn")) {
                    os << ",\n  \"nrarfcn\": " << nr.nrarfcn();
                }
                break;
            }
        default:
            break;
    }

    os << "}";
    return os;
}

auto operator<<(std::ostream& os, const CellInfo& ci) -> std::ostream& {
    os << ci.get();
    return os;
}

auto operator<<(std::ostream& os, const IPConfig& ipconfig) -> std::ostream& {
    os << R"({"address": ")" << ipconfig.address << "/" << ipconfig.prefix << R"(", )"
        << R"("gateway": ")" << ipconfig.gateway << R"(", )"
        << R"("dns1": )" << ipconfig.dns1 << R"(", "dns2": )" << ipconfig.dns2 << "\"}";
    return os;
}

auto operator<<(std::ostream& os, const TrafficStats& stats) -> std::ostream& {
    os << "{\"rx_bytes\": " << stats.rx_bytes << ", \"tx_bytes\": " << stats.tx_bytes << "}";
    return os;
}

} // namespace ezcellular
