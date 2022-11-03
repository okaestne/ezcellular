/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#pragma once

#include <cstdint>  // int8_t, ...
#include <ctime>    // time_t, timegm()
#include <functional>  // std::function
#include <memory>   // std::shared_ptr, std::weak_ptr
#include <optional> // std::optional
#include <string>   // std::string
#include <vector>   // std::vector

#include <ModemManager/ModemManager.h>  // enum def's
#include <sdbus-c++/sdbus-c++.h>  // sdbus::*

#include "connection.h"
#include "enums.h"
#include "sim.h"
#include "structs.h"

namespace ezcellular {

/**
 * @brief The central Modem object.
 *
 * Obtain instances using the methods of the ModemManager class.
 */
class Modem {
public:
    ~Modem() = default;

    /** @brief Copy constructor */
    Modem(const Modem&) = default;
    /** @brief Copy assignment operator */
    Modem& operator=(const Modem&) = default; // NOLINT(*-trailing-return-type)
    /** @brief Move contructor */
    Modem(Modem&&) = default;
    /** @brief Move assignment operator */
    Modem& operator=(Modem&&) = default; // NOLINT(*-trailing-return-type)

    // ---- forward declarations ----

    /**
     * @enum ModemState
     * @brief General state of a Modem.
     * @see https://www.freedesktop.org/software/ModemManager/doc/1.20.0/ModemManager/ref-overview-modem-state-machine.html
     */
    enum class ModemState : int8_t;
    /**
     * @enum PowerState
     * @brief Power state of a Modem.
     */
    enum class PowerState : uint32_t;
    /**
     * @enum LockState
     * @brief Details about the reason for a Modem with ModemState::LOCKED.
     */
    enum class LockState : int8_t;

    // ---- properties and observers ----

    /** @brief The manufacturer name of the modem. */
    [[nodiscard]] auto manufacturer() const -> std::string;
    /** @brief The model name of the modem. */
    [[nodiscard]] auto model() const -> std::string;
    /** @brief The IMEI of the modem. */
    [[nodiscard]] auto imei() const -> std::string;
    /** @brief The firmware version of the modem. */
    [[nodiscard]] auto firmware_version() const -> std::string;
    /**
     * @brief The phone number, aka Mobile Subscriber ISDN Number (MSISDN).
     * @note Likely requires the modem to be unlocked.
     * @return optional string, as it is not always available.
     */
    [[nodiscard]] auto phone_number() const -> std::optional<std::string>;

    // ---- PowerState ----

    /** @brief The current PowerState. */
    [[nodiscard]] auto power_state() const -> PowerState;
    /**
     * @brief Turn to modem off.
     * @note Modem needs to be in ModemState::DISABLED
     * @note This feature is not always supported by the hardware
    */
    void power_off() const;
    /**
     * @brief Turn to modem into a low power state (e.g. standy, radio off).
     * @note Modem needs to be in ModemState::DISABLED
     */
    void power_low() const;
    /**
     * @brief Turn to modem into full-on power state
     * @note Modem needs to be in ModemState::DISABLED
     */
    void power_on() const;

    // ---- ModemState ----

    /** @brief The current ModemState. */
    [[nodiscard]] auto state() const -> ModemState;
    /** @brief Whether the modem is enabled. */
    [[nodiscard]] auto enabled() const -> bool;
    /** @brief Whether the modem is locked. */
    [[nodiscard]] auto locked() const -> bool;
    /** @brief Whether the modem is registered in a network. */
    [[nodiscard]] auto registered() const -> bool;
    /** @brief Whether the modem is connected in a network (active call/packet service). */
    [[nodiscard]] auto connected() const -> bool;
    /** @brief type for callbacks needed for observe_modem_state() */
    using ModemStateObserver = std::function<void(ModemState, ModemState)>;
    /**
     * @brief Register a callback for ModemState updates.
     * @param observer a ModemStateObserver to register
     */
    void observe_modem_state(ModemStateObserver observer);
    /**
     * @brief Enable or disable the Modem to register.
     * @param enable whether to enable or disable
     */
    void enable(bool enable) const;
    /**
     * @brief reset the modem (power cycle).
     * @see ModemManager::reset_modem()
     * @warning This action will render this object instance as well as related SIM and Connection objects invalid.
     *          Use ModemManager::reset_modem() instead to get the new modem object after the restart
     */
    void reset();

    // ---- SIM ----

    /**
     * @brief The current LockState.
     *
     * Gives details about the reason, why the modem is in ModemState::LOCKED.
     */
    [[nodiscard]] auto lock_state() const -> LockState;
    /**
     * @brief The currenty active SIM card.
     * @return an std::optional with a SIM object if available, empty otherwise (e.g. no SIM card present)
     */
    [[nodiscard]] auto active_sim() const -> std::optional<SIM>;

    // ---- Connection ----

    /**
     * @brief The currently active connection.
     * @note Requires that the modem is connected to a network (ModemState::CONNECTED).
     * @return an std::optional containing an active Connection object, empty otherwise.
    */
    [[nodiscard]] auto active_connection() const -> std::optional<Connection>;

    /**
     * @brief All related Connection objects to this modem.
     * @note Both active and inactive connections will be returned.
     * @return A std::vector Connection object, empty if there are none.
    */
    [[nodiscard]] auto connections() const -> std::vector<Connection>;
    /**
     * @brief Try to connect to the given APN.
     * @param apn The access point name to connect to.
     * @param ip_type The IP address type for the bearer (e.g. IPv4, IPv6 or both)
     * @todo use networkmanager to apply assigned IPs to linux network interface
     * @note Requires that the modem is ready to connect to a network.
    */
    void connect(std::string apn, IPType ip_type = IPType::IPV4_AND_IPV6);

    /**
     * @brief The network operator PLMN.
     * @note requires that the modem is registered in a network.
     * @return the PLMN (MCC+MNC) as string
    */
    [[nodiscard]] auto operator_plmn() const -> std::string;
    /**
     * @brief The network operator name.
     * @note Requires that the modem is registered in a network.
     * @return operator name as string
    */
    [[nodiscard]] auto operator_name() const -> std::string;

    // ---- Technology, Signal, Cell Info ----

    /** @brief the current radio technology */
    [[nodiscard]] auto technology() const -> Technology;

    /**
     * @brief The current signal quality.
     * @note Requires that the modem is registered in a network (ModemState::REGISTERED).
    */
    [[nodiscard]] auto signal() const -> Signal;

    /** @brief type for callbacks needed for observe_signal() */
    using SignalObserver = std::function<void(Signal)>;
    /**
     * @brief Register a callback for periodic Signal updates.
     * @param observer the SignalObserver to register
     * @param interval_sec the update interval in seconds
     */
    void observe_signal(SignalObserver observer, uint32_t interval_sec);

    /**
     * @brief Cell information
    */
    [[nodiscard]] auto cell_info() const -> std::vector<CellInfo>;

    // Location

    /**
     * @brief The current cell location identifiers.
     * @note requires that the modem is registered in a network
    */
    [[nodiscard]] auto location() const -> Location;
    /** @brief type for callbacks needed for observe_location() */
    using LocationObserver = std::function<void(Location)>;
    /**
     * @brief Register a callback for Location updates.
     * @param observer the LocationObserver to register
     */
    void observe_location(const LocationObserver& observer) const;

    // Time

    /**
     * @brief The time of the modem, which usually is equal to the network's time.
     * @note requires that the modem is registered in a network
     * @return the time reported by the modem as ISO-8601 formatted string
    */
    [[nodiscard]] auto network_time() const -> std::string;
    /**
     * @brief Same as network_time() but returning a std::time_t.
     * @return the time reported by the modem as std::time_t (aka "epoch time" or "unix timestamp")
    */
    [[nodiscard]] auto network_time_epoch() const -> std::time_t;

private:
    // make constructors private to enforce creation using a ModemManager instance (ModemManagerOMProxy to be precise)
    explicit Modem(std::weak_ptr<sdbus::IConnection>, const sdbus::ObjectPath&);
    explicit Modem(std::weak_ptr<sdbus::IConnection>, std::shared_ptr<sdbus::IProxy>);

    friend class ModemManager;
    friend class ModemManagerOMProxy;
    std::weak_ptr<sdbus::IConnection> conn_;
    std::shared_ptr<sdbus::IProxy> dbus_proxy_;

    // user provided observers
    ModemStateObserver user_modemstate_observer_;

    // common helper methods
    void set_power_state(PowerState state) const;

public:
    enum class ModemState : int8_t {
        FAILED = MM_MODEM_STATE_FAILED,               ///< modem failed to initialize
        UNKNOWN = MM_MODEM_STATE_UNKNOWN,             ///< unknown state
        INITIALIZING = MM_MODEM_STATE_INITIALIZING,   ///< modem is initializing, i.e. starting up
        LOCKED = MM_MODEM_STATE_LOCKED,               ///< modem is locked, check lock_state() for more details
        DISABLED = MM_MODEM_STATE_DISABLED,           ///< modem is disabled (low power mode)
        DISABLING = MM_MODEM_STATE_DISABLING,         ///< modem is about to be disabled
        ENABLING = MM_MODEM_STATE_ENABLING,           ///< modem is about to be enabled
        ENABLED = MM_MODEM_STATE_ENABLED,             ///< modem is enabled
        SEARCHING = MM_MODEM_STATE_SEARCHING,         ///< modem is searching for networks to register
        REGISTERED = MM_MODEM_STATE_REGISTERED,       ///< modem is registered in a network
        DISCONNECTING = MM_MODEM_STATE_DISCONNECTING, ///< modem is disconnecting, i.e. from call/packet service
        CONNECTING = MM_MODEM_STATE_CONNECTING,       ///< modem is connecting, i.e. to call/packet service
        CONNECTED = MM_MODEM_STATE_CONNECTED,         ///< modem is connected, i.e. call/packet service is active
    };

    enum class PowerState : uint32_t {
        UNKNOWN = MM_MODEM_POWER_STATE_UNKNOWN,  ///< Unknown power state
        OFF = MM_MODEM_POWER_STATE_OFF,          ///< Modem is powered off
        LOW = MM_MODEM_POWER_STATE_LOW,          ///< Modem is in low-power state (e.g. standby, radio off)
        ON = MM_MODEM_POWER_STATE_ON,            ///< Modem is powered on and can be used
    };

    enum class LockState : int8_t {
        UNKNOWN = MM_MODEM_LOCK_UNKNOWN,   ///< unknown lock state, modem might not be ready yet.
        UNLOCKED = MM_MODEM_LOCK_NONE,     ///< modem is unlocked an can be used to connect to a network
        SIM_PIN = MM_MODEM_LOCK_SIM_PIN,   ///< modem is locked, SIM PIN is required to unlock
        SIM_PIN2 = MM_MODEM_LOCK_SIM_PIN2, ///< modem is unlocked, but SIM PIN2 might be required for certain features
        SIM_PUK = MM_MODEM_LOCK_SIM_PUK,   ///< modem is locked, SIM PUK is required to unlock
        SIM_PUK2 = MM_MODEM_LOCK_SIM_PUK2  ///< modem is locked, SIM PUK2 is required to unlock
    };

};

} // namespace ezcellular
