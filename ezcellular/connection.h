/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#pragma once

#include <cstdint>     // uint32_t and friends
#include <functional>  // std::function
#include <memory>      // std::unique_ptr, std::weak_ptr
#include <string>      // std::string
#include <optional>    // std::optional

#include <sdbus-c++/sdbus-c++.h>  // sdbus::*

#include "enums.h"    // IPType
#include "structs.h"  // IPConfig, TrafficStats

namespace ezcellular {

/**
 * @brief Represents a connection and provides its most relevant information.
 *
 * Connections can be in an active state, but they don't have to be.
 *
 * For a connection to a mobile network, a bearer needs to be set up.
 * Bearers are tunnels that are used to connect to packet data networks.
 * Paramters to setup a bearer comprise the APN/PDN and the IP type.
 *
 * This class also provides information about the IP network interface related
 * to the represented connection, as well as traffic statistics.
 * For this, the connection needs to be active.
 */
class Connection {
public:
    ~Connection() = default;

    /** @brief Move contructor */
    Connection(Connection&&) = default;
    /** @brief Move assignment operator */
    Connection& operator=(Connection&&) = default; // NOLINT(*-trailing-return-type)

    /** @brief Copy constructor (deleted to forbid copies) */
    Connection(const Connection&) = delete;
    /** @brief Copy assignment operator (deleted to forbid copies) */
    Connection& operator=(const Connection&) = delete; // NOLINT(*-trailing-return-type)

    // --- bearer info ---

    /** @brief Whether the Connection is active (i.e. can be used for data communication) */
    [[nodiscard]] auto active() const -> bool;
    /** @brief The configured APN/PDN. */
    [[nodiscard]] auto apn() const -> std::string;
    /** @brief The configured APN/PDN IP type. */
    [[nodiscard]] auto ip_type() const -> IPType;

    // --- IP info ---

    /** @brief Name of the Linux network interface, e.g. `wwan0` */
    [[nodiscard]] auto linux_interface() const -> std::string;
    /** @brief The current IPv4 configuration. */
    [[nodiscard]] auto ipv4_config() const -> std::optional<IPConfig>;
    /** @brief The current IPv6 configuration. */
    [[nodiscard]] auto ipv6_config() const -> std::optional<IPConfig>;

    // --- IP metrics ---

    /** @brief Traffic statistics */
    [[nodiscard]] auto traffic_stats() const -> TrafficStats;
    /** @brief type for callbacks needed for observe_traffic_stats() */
    using TrafficStatsObserver = std::function<void(TrafficStats)>;
    /**
     * @brief Register a callback for periodic TrafficStats updates.
     * @param observer the TrafficStatsObserver to register
     * @param interval_ms the update interval in milliseconds
     */
    void observe_traffic_stats(TrafficStatsObserver observer, uint32_t interval_ms = 0);

private:
    std::weak_ptr<sdbus::IConnection> conn_;
    std::unique_ptr<sdbus::IProxy> dbus_proxy_;

    // private ctor; supposed to be invoked by class Modem only
    friend class Modem;
    explicit Connection(std::weak_ptr<sdbus::IConnection>, const sdbus::ObjectPath&);

    [[nodiscard]] auto get_nm_device_proxy() const -> std::unique_ptr<sdbus::IProxy>;
    [[nodiscard]] auto get_ip_config(IPType type) const -> std::optional<IPConfig>;
};

} // namespace ezcellular
