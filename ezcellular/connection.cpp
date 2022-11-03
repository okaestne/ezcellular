/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#include "connection.h"

#include <map>       // std::map
#include <stdexcept> // std::out_of_range
#include <utility>   // std::move
#include <vector>    // std::vector

#include "dbus_constants.h"
#include "exception.h"

namespace ezcellular {

Connection::Connection(std::weak_ptr<sdbus::IConnection> conn, const sdbus::ObjectPath& dbus_path)
    : conn_{std::move(conn)}, dbus_proxy_{sdbus::createProxy(*conn_.lock(), DBus::MM_BUS_NAME, dbus_path)} {}

// --- bearer info ---

auto Connection::active() const -> bool {
    return dbus_proxy_->getProperty("Connected").onInterface(DBus::MM_IF_BEARER);
}

auto Connection::apn() const -> std::string {
    std::map<std::string, sdbus::Variant> result;
    result = dbus_proxy_->getProperty("Properties").onInterface(DBus::MM_IF_BEARER);
    return result["apn"].get<std::string>();
}

auto Connection::ip_type() const -> IPType {
    std::map<std::string, sdbus::Variant> result;
    result = dbus_proxy_->getProperty("Properties").onInterface(DBus::MM_IF_BEARER);
    return static_cast<IPType>(result["ip-type"].get<std::uint32_t>());
}

// --- IP info ---

auto Connection::linux_interface() const -> std::string {
    return dbus_proxy_->getProperty("Interface").onInterface(DBus::MM_IF_BEARER);
}

auto Connection::get_ip_config(IPType type) const -> std::optional<IPConfig> {
    IPConfig ip_conf{};
    std::string prop_name;

    if (type == IPType::IPV4) {
        prop_name = "Ip4Config";
    } else if (type == IPType::IPV6) {
        prop_name = "Ip6Config";
    } else {
        __builtin_unreachable(); // GCC ONLY
        return {}; // empty
    }

    std::map<std::string, sdbus::Variant> result = dbus_proxy_->getProperty(prop_name)
        .onInterface(DBus::MM_IF_BEARER);

    try {
        ip_conf.ip_type = type;
        ip_conf.address = result.at("address").get<std::string>();
        ip_conf.prefix = result.at("prefix");
        ip_conf.gateway = result.at("gateway").get<std::string>();
        ip_conf.dns1 = result.at("dns1").get<std::string>();
        ip_conf.dns2 = result.at("dns2").get<std::string>();
    } catch (std::out_of_range&) {
        return {};  // empty optional
    }

    return ip_conf;
}

auto Connection::ipv4_config() const -> std::optional<IPConfig> {
    return get_ip_config(IPType::IPV4);
}

auto Connection::ipv6_config() const -> std::optional<IPConfig> {
    return get_ip_config(IPType::IPV6);
}

// --- IP metrics ---

auto Connection::get_nm_device_proxy() const -> std::unique_ptr<sdbus::IProxy> {
    sdbus::ObjectPath obj_path_nm_dev;
    auto iface = linux_interface();

    if (auto conn = conn_.lock()) {
        // 1. get "Device" object path for wwan iface (e.g. "wwan0")
        auto nm_proxy = sdbus::createProxy(*conn, DBus::NM_BUS_NAME, DBus::NM_OBJ_NETWORKMANAGER);
        nm_proxy->callMethod("GetDeviceByIpIface")
            .onInterface(DBus::NM_IF_NETWORKMANAGER)
            .withArguments(iface)
            .storeResultsTo(obj_path_nm_dev);
        // 2. return the proxy
        return sdbus::createProxy(*conn, DBus::NM_BUS_NAME, obj_path_nm_dev);
    }

    throw ConnectionException("DBus connection lost");
}

auto Connection::traffic_stats() const -> TrafficStats {
    TrafficStats stats{};
    auto nm_dev_proxy = get_nm_device_proxy();

    stats.rx_bytes = nm_dev_proxy->getProperty("RxBytes").onInterface(DBus::NM_IF_DEVICE_STATISTICS);
    stats.tx_bytes = nm_dev_proxy->getProperty("TxBytes").onInterface(DBus::NM_IF_DEVICE_STATISTICS);

    return stats;
}

void Connection::observe_traffic_stats(Connection::TrafficStatsObserver observer, uint32_t interval_ms) {
    /* get RX/TX stats from NetworkManager D-Bus endpoint */
    if (auto conn = conn_.lock()) {
        std::shared_ptr<sdbus::IProxy> nm_dev_proxy = get_nm_device_proxy();

        // 1. set refresh interval
        nm_dev_proxy->setProperty("RefreshRateMs").onInterface(DBus::NM_IF_DEVICE_STATISTICS).toValue(interval_ms);

        // 3. subscribe to changes via DBus' default PropertiesChanged signal
        nm_dev_proxy->uponSignal("PropertiesChanged").onInterface(DBus::DBUS_IF_PROPERTIES).call(
            [nm_dev_proxy, observer](const std::string& interfaceName,
                                     [[maybe_unused]] const std::map<std::string, sdbus::Variant>& changedProperties,
                                     [[maybe_unused]] const std::vector<std::string>& invalidatedProperties) {
            // check relevant interface
            if (interfaceName == DBus::NM_IF_DEVICE_STATISTICS) {
                // fetch RxBytes and TxBytes
                TrafficStats stats{};
                stats.rx_bytes = nm_dev_proxy->getProperty("RxBytes").onInterface(DBus::NM_IF_DEVICE_STATISTICS);
                stats.tx_bytes = nm_dev_proxy->getProperty("TxBytes").onInterface(DBus::NM_IF_DEVICE_STATISTICS);
                (void) observer(stats);
            }
        });
        nm_dev_proxy->finishRegistration();

    } else {
        throw ConnectionException("DBus connection lost");
    }
}

} // namespace ezcellular
