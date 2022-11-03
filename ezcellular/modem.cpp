/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#include "modem.h"

#include <algorithm> // std::any_of
#include <array>     // std::array
#include <cstdio>    // std::sscanf
#include <ctime>     // std::tm
#include <iomanip>   // get_time
#include <iterator>  // std::back_inserter
#include <map>       // std::map
#include <sstream>   // istringstream
#include <utility>   // std::move

#include "any_map.h"  // sdbus_variant_map
#include "dbus_constants.h"
#include "helpers.h" // enums -> ostream
#include "exception.h"

namespace ezcellular {

static void assert_state(const Modem& modem, Modem::ModemState required_state, const std::string& required_for) {
    auto current_state = modem.state();
    if (current_state < required_state) {
        std::ostringstream oss;
        oss << "can't " << required_for << ": "
            << "modem state is '" << current_state << "', but needs to be at least '" << required_state << "'.";
        throw ModemException{oss.str()};
    }
}

Modem::Modem(std::weak_ptr<sdbus::IConnection> conn, const sdbus::ObjectPath& dbus_path)
    : conn_{std::move(conn)}, dbus_proxy_{sdbus::createProxy(*conn_.lock(), DBus::MM_BUS_NAME, dbus_path)} {}

Modem::Modem(std::weak_ptr<sdbus::IConnection> conn, std::shared_ptr<sdbus::IProxy> proxy)
    : conn_{std::move(conn)}, dbus_proxy_{std::move(proxy)} {}

/* properties */

auto Modem::manufacturer() const -> std::string {
    return dbus_proxy_->getProperty("Manufacturer").onInterface(DBus::MM_IF_MODEM);
}

auto Modem::model() const -> std::string {
    return dbus_proxy_->getProperty("Model").onInterface(DBus::MM_IF_MODEM);
}

auto Modem::imei() const -> std::string {
    // not needed anymore since MM commit 6f00fb86 (2023-02-13) (included with release 1.21.4)
    //assert_state(*this, ModemState::ENABLED, "IMEI");
    return dbus_proxy_->getProperty("Imei").onInterface(DBus::MM_IF_MODEM_MODEM3GPP);
}

auto Modem::firmware_version() const -> std::string {
    return dbus_proxy_->getProperty("Revision").onInterface(DBus::MM_IF_MODEM);
}

auto Modem::phone_number() const -> std::optional<std::string> {
    std::vector<std::string> numbers = dbus_proxy_->getProperty("OwnNumbers").onInterface(DBus::MM_IF_MODEM);
    if (!numbers.empty()) {
        return numbers[0];
    }
    return {};  // empty optional
}

// --- PowerState ---

auto Modem::power_state() const -> PowerState {
    uint32_t state = dbus_proxy_->getProperty("PowerState").onInterface(DBus::MM_IF_MODEM);
    return static_cast<PowerState>(state);
}

// (private) common helper
void Modem::set_power_state(PowerState state) const {
    // needs to be DISABLED according to docs
    assert_state(*this, ModemState::DISABLED, "change power state");
    dbus_proxy_->callMethod("SetPowerState").onInterface(DBus::MM_IF_MODEM).withArguments(static_cast<uint32_t>(state));
}

void Modem::power_off() const {
    set_power_state(PowerState::OFF);
}

void Modem::power_low() const {
    set_power_state(PowerState::LOW);
}

void Modem::power_on() const {
    set_power_state(PowerState::ON);
}

// --- ModemState ---

void Modem::enable(bool enable) const {
    dbus_proxy_->callMethod("Enable").onInterface(DBus::MM_IF_MODEM).withArguments(enable);
}

void Modem::reset() {
    dbus_proxy_->callMethod("Reset").onInterface(DBus::MM_IF_MODEM);
}

auto Modem::state() const -> Modem::ModemState {
    int32_t state = dbus_proxy_->getProperty("State").onInterface(DBus::MM_IF_MODEM);
    return static_cast<ModemState>(state);
}

auto Modem::enabled() const -> bool {
    return state() >= ModemState::ENABLED;
}

auto Modem::locked() const -> bool {
    auto state = lock_state();
    return state != LockState::UNLOCKED && state != LockState::SIM_PIN2;
}

auto Modem::registered() const -> bool {
    return state() >= ModemState::REGISTERED;
}

auto Modem::connected() const -> bool {
    return state() == ModemState::CONNECTED;
}

void Modem::observe_modem_state(Modem::ModemStateObserver observer) {
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    auto callback = [&, observer](int32_t old_s, int32_t new_s, [[maybe_unused]] uint32_t reason) {
        // forward both states to observer
        auto old_ = static_cast<Modem::ModemState>(old_s);
        auto new_ = static_cast<Modem::ModemState>(new_s);
        return observer(old_, new_);
    };

    dbus_proxy_->uponSignal("StateChanged").onInterface(DBus::MM_IF_MODEM).call(callback);
    dbus_proxy_->finishRegistration();
}

auto Modem::lock_state() const -> Modem::LockState {
    uint32_t state = dbus_proxy_->getProperty("UnlockRequired").onInterface(DBus::MM_IF_MODEM);
    return static_cast<LockState>(state);
}

auto Modem::active_sim() const -> std::optional<SIM> {
    sdbus::ObjectPath objpath = dbus_proxy_->getProperty("Sim").onInterface(DBus::MM_IF_MODEM);
    if (objpath == "/") {
        return {}; // empty optional
    }
    return SIM{conn_, objpath};
}

/* Connection */

auto Modem::active_connection() const -> std::optional<Connection> {
    auto conns = connections();
    auto result = std::find_if(conns.begin(), conns.end(), [](Connection& conn) { return conn.active(); });

    if (result == conns.end()) {
        return {};  // empty optional
    }

    return std::move(*result);
}

auto Modem::connections() const -> std::vector<Connection> {
    std::vector<Connection> conns;
    std::vector<sdbus::ObjectPath> paths = dbus_proxy_->getProperty("Bearers").onInterface(DBus::MM_IF_MODEM);

    std::transform(paths.begin(), paths.end(), std::back_inserter(conns),
                   [&](const sdbus::ObjectPath& p) { return Connection{conn_, p}; });
    return conns;
}

auto Modem::connect(std::string apn, IPType ip_type) -> void {
    sdbus_variant_map properties = {{"apn", apn}, {"ip-type", static_cast<uint32_t>(ip_type)}};

    // 1. create bearer
    sdbus::ObjectPath bearer_path;
    dbus_proxy_->callMethod("CreateBearer").onInterface(DBus::MM_IF_MODEM).withArguments(properties).storeResultsTo(bearer_path);

    // 2. connect bearer
    auto bearer_proxy = sdbus::createProxy(*conn_.lock(), DBus::MM_BUS_NAME, bearer_path);
    bearer_proxy->callMethod("Connect").onInterface(DBus::MM_IF_BEARER);

    // 3. ???
}

auto Modem::operator_plmn() const -> std::string {
    return dbus_proxy_->getProperty("OperatorCode").onInterface(DBus::MM_IF_MODEM_MODEM3GPP);
}

auto Modem::operator_name() const -> std::string {
    return dbus_proxy_->getProperty("OperatorName").onInterface(DBus::MM_IF_MODEM_MODEM3GPP);
}

/* Signal */

auto Modem::technology() const -> Technology {
    // TODO: check if AccessTechnologies might be a bitmask, e.g. in case of 5G NSA
    uint32_t mm_tech = dbus_proxy_->getProperty("AccessTechnologies").onInterface(DBus::MM_IF_MODEM);

    switch (mm_tech) {
        case MM_MODEM_ACCESS_TECHNOLOGY_GSM:
        case MM_MODEM_ACCESS_TECHNOLOGY_GSM_COMPACT:
        case MM_MODEM_ACCESS_TECHNOLOGY_GPRS:
        case MM_MODEM_ACCESS_TECHNOLOGY_EDGE:
            return Technology::GSM;
        case MM_MODEM_ACCESS_TECHNOLOGY_UMTS:
        case MM_MODEM_ACCESS_TECHNOLOGY_HSDPA:
        case MM_MODEM_ACCESS_TECHNOLOGY_HSUPA:
        case MM_MODEM_ACCESS_TECHNOLOGY_HSPA:
        case MM_MODEM_ACCESS_TECHNOLOGY_HSPA_PLUS:
            return Technology::UMTS;
        case MM_MODEM_ACCESS_TECHNOLOGY_LTE:
            return Technology::LTE;
        case MM_MODEM_ACCESS_TECHNOLOGY_5GNR:
            return Technology::NR5G;
        default:
            return Technology::UNKNOWN;
    }
}

// common helper for signal, observe_signal, cell_info
static auto dbus_signal_to_Signal(Technology tech, const sdbus_variant_map& signal) -> Signal {
    try {
        switch (tech) {
            case Technology::LTE:
                return SignalLTE::from_variant_map(signal);
            case Technology::NR5G:
                return SignalNR5G::from_variant_map(signal);
            default:
                throw ModemException{"signal: current technology unknown or not supported yet"};
        }
    } catch (std::out_of_range&) {
        // some value is not set in response, return "empty" value (shared_ptr with nullptr)
        return {nullptr};
    }
}

auto Modem::signal() const -> Signal {
    // 0. Must be registered
    assert_state(*this, ModemState::REGISTERED, "access signal quality");

    // setup refresh if not done already to get any values
    if (0 == dbus_proxy_->getProperty("Rate").onInterface(DBus::MM_IF_MODEM_SIGNAL).get<uint32_t>()) {
        dbus_proxy_->callMethod("Setup").onInterface(DBus::MM_IF_MODEM_SIGNAL).withArguments(5U);
    }

    // fetch info for current RAT
    auto tech = technology();
    sdbus_variant_map signal;

    switch (tech) {
        case Technology::LTE: {
                signal = dbus_proxy_->getProperty("Lte").onInterface(DBus::MM_IF_MODEM_SIGNAL);
                return dbus_signal_to_Signal(tech, signal);
            }
        case Technology::NR5G: {
                signal = dbus_proxy_->getProperty("Nr5g").onInterface(DBus::MM_IF_MODEM_SIGNAL);
                return dbus_signal_to_Signal(tech, signal);
            }
        default:
            throw ModemException{"signal: current technology unknown or not supported yet"};
    }
}

void Modem::observe_signal(SignalObserver observer, uint32_t interval_sec) {
    // 0. Must be registered
    assert_state(*this, ModemState::REGISTERED, "observe signal quality");

    // 1. setup polling
    dbus_proxy_->callMethod("Setup").onInterface(DBus::MM_IF_MODEM_SIGNAL).withArguments(interval_sec).dontExpectReply();

    // 2. register callback
    auto callback = [observer](const std::string& interfaceName,
                               const sdbus_variant_map& changedProperties,
                               [[maybe_unused]] const std::vector<std::string>& invalidatedProperties) {
        if (interfaceName == DBus::MM_IF_MODEM_SIGNAL) {
            sdbus_variant_map dbus_signal; // signal values from D-Bus attribute. tech specific.

            if (auto it = changedProperties.find("Lte"); it != changedProperties.end()) {
                dbus_signal = it->second;
                auto signal = dbus_signal_to_Signal(Technology::LTE, dbus_signal);
                return observer(signal);
            }
            if (auto it = changedProperties.find("Nr5g"); it != changedProperties.end()) {
                dbus_signal = it->second;
                auto signal = dbus_signal_to_Signal(Technology::NR5G, dbus_signal);
                return observer(signal);
            }
        }
    };

    dbus_proxy_->uponSignal("PropertiesChanged").onInterface(DBus::DBUS_IF_PROPERTIES).call(callback);
    dbus_proxy_->finishRegistration();
}

auto Modem::cell_info() const -> std::vector<CellInfo> {
    std::vector<CellInfo> infos;
    std::vector<sdbus_variant_map> result;

    dbus_proxy_->callMethod("GetCellInfo").onInterface(DBus::MM_IF_MODEM).storeResultsTo(result);

    for (const auto& res : result) {
        uint32_t cell_type = res.at("cell-type");

        if (cell_type == MM_CELL_TYPE_LTE) {
            auto lte = CellInfoLTE::from_variant_map(res);
            infos.push_back(std::move(lte));
        }
        if (cell_type == MM_CELL_TYPE_5GNR) {
            auto nr = CellInfoNR5G::from_variant_map(res);
            infos.push_back(std::move(nr));
        }
    }

    return infos;
}

// common helper
static auto dbus_location_to_Location(const Modem& modem,
                                      const std::map<uint32_t, sdbus::Variant>& location_dict) -> Location {
    auto it = location_dict.find(MM_MODEM_LOCATION_SOURCE_3GPP_LAC_CI); // cell info
    if (it == location_dict.end()) {
        return {nullptr};
    }

    auto loc_data = it->second.get<std::string>();
    std::array<char, 4> mcc{}; // MCC: 3 chars + '\0'
    std::array<char, 4> mnc{}; // MNC: 2 or 3 chars + '\0'
    uint32_t ci{};
    uint32_t tac{};

    auto rat = modem.technology();
    auto lte_nr_loc = std::shared_ptr<LocationLTE>(nullptr); // LTE and NR compatible

    if (rat == Technology::LTE) {
        lte_nr_loc = std::make_shared<LocationLTE>();
    } else if (rat != Technology::NR5G) {
        lte_nr_loc = std::make_shared<LocationNR5G>();
    } else {
        return {nullptr};  // not supported yet
    }

    // parse
    if (std::sscanf(loc_data.c_str(), "%3[0-9],%3[0-9],%*X,%X,%X",
                    mcc.data(), mnc.data(), &ci, &tac) != 4) {
        return {nullptr};  // parsing failed
    }

    lte_nr_loc->insert({"mcc", std::string{mcc.data()}});
    lte_nr_loc->insert({"mnc", std::string{mnc.data()}});
    lte_nr_loc->insert({"ci", ci});
    lte_nr_loc->insert({"tac", tac});

    return lte_nr_loc;
}

/* Location */
auto Modem::location() const -> Location {
    std::map<uint32_t, sdbus::Variant> location_res;

    assert_state(*this, ModemState::REGISTERED, "access cell location");

    //MM_MODEM_LOCATION_SOURCE_3GPP_LAC_CI
    dbus_proxy_->callMethod("GetLocation").onInterface(DBus::MM_IF_MODEM_LOCATION).storeResultsTo(location_res);

    return dbus_location_to_Location(*this, location_res);
}

void Modem::observe_location(const LocationObserver& observer) const {
    assert_state(*this, ModemState::REGISTERED, "observe cell location");

    // 1. enable Location property and the property update signal
    uint32_t location_LAC_CI = MM_MODEM_LOCATION_SOURCE_3GPP_LAC_CI; // location source type to enable, unsigned
    dbus_proxy_->callMethod("Setup").onInterface(DBus::MM_IF_MODEM_LOCATION).withArguments(location_LAC_CI, true);

    // 2. setup signal observer
    auto callback = [this, &observer](const std::string& interfaceName,
                                      const sdbus_variant_map& changedProperties,
                                      [[maybe_unused]] const std::vector<std::string>& invalidatedProperties) {
        if (interfaceName != DBus::MM_IF_MODEM_LOCATION) {
            return;  // we are only interested in the .Modem.Location interface
        }
        auto loc_it = changedProperties.find("Location");
        if (loc_it != changedProperties.end()) {
            // found update!
            auto dbus_loc = loc_it->second.get<std::map<uint32_t, sdbus::Variant>>(); // "cast" map value
            auto loc = dbus_location_to_Location(*this, dbus_loc);
            // call observer
            observer(loc);
        }
    };

    dbus_proxy_->uponSignal("PropertiesChanged").onInterface(DBus::DBUS_IF_PROPERTIES).call(callback);
    dbus_proxy_->finishRegistration();
}

auto Modem::network_time() const -> std::string {
    std::string time_str;
    assert_state(*this, ModemState::ENABLED, "get network time");
    dbus_proxy_->callMethod("GetNetworkTime").onInterface(DBus::MM_IF_MODEM_TIME).storeResultsTo(time_str);
    return time_str;
}

auto Modem::network_time_epoch() const -> std::time_t {
    std::tm tm{};
    auto time_str = network_time();

    // parse time string using get_time into tm and then convert to time_t
    // ignore the timezone as epoch time is UTC
    std::istringstream iss_time{time_str};
    iss_time >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (iss_time.fail()) {
        return 0;  // get_time() failed
    }
    // use POSIX timegm here, as mktime interprets tm using the local timezone :eyes:
    return ::timegm(&tm);
}

} // namespace ezcellular
