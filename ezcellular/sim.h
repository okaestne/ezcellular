/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#pragma once

#include <memory> // std::weak_ptr, std::unique_ptr
#include <string> // std::string

#include <sdbus-c++/sdbus-c++.h>  // sdbus::*

namespace ezcellular {

/**
 * @brief Represents a SIM card.
 *
 */
class SIM {
public:
    ~SIM() = default;

    /** @brief Move contructor */
    SIM(SIM&&) = default;
    /** @brief Move assignment operator */
    SIM& operator=(SIM&&) = default; // NOLINT(*-trailing-return-type)

    /** @brief Copy constructor (deleted to forbid copies) */
    SIM(const SIM&) = delete;
    /** @brief Copy assignment operator (deleted to forbid copies) */
    SIM& operator=(const SIM&) = delete; // NOLINT(*-trailing-return-type)

    // ---- methods ----

    /**
     * @brief Unlocks the SIM card using the given PIN
     * @param pin the PIN
     */
    void send_pin(const std::string& pin);
    /**
     * @brief Unlocks the SIM card using the given PUK and PIN
     * @param puk the PUK code
     * @param pin the new PIN code
     */
    void send_puk(const std::string& puk, const std::string& pin);

    // ---- properties ----

    /** @brief Whether the SIM card is active (primary SIM). */
    [[nodiscard]] auto active() const -> bool;
    /** @brief The international mobile subscriber identity (IMSI). */
    [[nodiscard]] auto imsi() const -> std::string;
    /** @brief The integrated circuit card identifier (ICCID). */
    [[nodiscard]] auto iccid() const -> std::string;
    /** @brief PLMN ID of home network */
    [[nodiscard]] auto home_plmn() const -> std::string;
    /** @brief Name of the network operator that issued the SIM */
    [[nodiscard]] auto operator_name() const -> std::string;


private:
    std::unique_ptr<sdbus::IProxy> dbus_proxy_;

    // private ctor; supposed to be invoked by class Modem only
    friend class Modem;
    explicit SIM(const std::weak_ptr<sdbus::IConnection>&, const sdbus::ObjectPath&);
};

} // namespace ezcellular
