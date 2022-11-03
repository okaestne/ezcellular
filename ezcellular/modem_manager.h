/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#pragma once

#include <future>   // std::future
#include <memory>   // std::shared_ptr, std::unique_ptr
#include <optional> // std::optional
#include <string>   // std::string
#include <vector>   // std::vector

#include <sdbus-c++/sdbus-c++.h>  // sdbus::*

#include "modem.h"

namespace ezcellular {

constexpr auto ANY_IMEI = "<ANY_IMEI>";

/**
 * @brief internal helper class
 */
class ModemManagerOMProxy; // IWYU pragma: keep

/**
 * @brief Management of Modem instances and background stuff.
 *
 * Use this class to obtain Modem instances.
 *
 * This class also manages the connection to the ModemManager D-Bus service.
 * There must be only one instance of it and it must be kept alive
 * while the Modem instances are in use.
 */
class ModemManager {
public:
    ModemManager();
    ~ModemManager(); // can't be defined here as ModemManagerOMProxy is incomplete type

    /** @brief Copy constructor (deleted to forbid copies) */
    ModemManager(const ModemManager&) = delete;
    /** @brief Copy assignment operator (deleted to forbid copies) */
    ModemManager& operator=(const ModemManager&) = delete; // NOLINT(*-trailing-return-type)
    /** @brief Move contructor */
    ModemManager(ModemManager&&) = default;
    /** @brief Move assignment operator */
    ModemManager& operator=(ModemManager&&) = default; // NOLINT(*-trailing-return-type)

    /** @brief Whether any Modem is available. */
    [[nodiscard]] auto modems_available() const -> bool;
    /** @brief Vector of all available Modems */
    [[nodiscard]] auto available_modems() const -> std::vector<Modem>;
    /**
     * @brief First available Modem, if there is any.
     * @see await_modem()
     * @return an std::optional value (empty, if no Modem is available)
    */
    [[nodiscard]] auto any_modem() const -> std::optional<Modem>;
    /**
     * @brief Wait for a modem to become available.
     * @param imei The IMEI of the modem to await or ANY_IMEI to take the next Modem to become available.
     * @return A std::future that can be awaited to get the Modem.
     */
    [[nodiscard]] auto await_modem(std::string imei = ANY_IMEI) const -> std::future<Modem>;

    /**
     * @brief Reset a modem (power cycle).
     * @param modem the modem to reset
     * @warning This action will render this object instance as well as related SIM and Connection objects invalid!
     * @return
     */
    [[nodiscard]] auto reset_modem(Modem& modem) const -> Modem;

    /** @brief ModemManager version string */
    [[nodiscard]] auto version() const -> std::string;
private:
    std::shared_ptr<sdbus::IConnection> conn_;
    std::unique_ptr<ModemManagerOMProxy> mm_proxy_;
};

} // namespace ezcellular