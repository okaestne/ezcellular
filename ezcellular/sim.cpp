/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#include "sim.h"
#include "dbus_constants.h"
#include "exception.h"

namespace ezcellular {

SIM::SIM(const std::weak_ptr<sdbus::IConnection>& conn, const sdbus::ObjectPath& dbus_path)
    : dbus_proxy_{sdbus::createProxy(*conn.lock(), DBus::MM_BUS_NAME, dbus_path)} {}

/* methods */

void SIM::send_pin(const std::string& pin) {
    try{
        /* dontExpectResult() prevents to throw exceptions
        * therefore, expect a void result (which can be stored in a sdbus::Variant)*/
        sdbus::Variant res;
        dbus_proxy_->callMethod("SendPin")
            .onInterface(DBus::MM_IF_SIM).withArguments(pin)
            .storeResultsTo(res);

    } catch(sdbus::Error& err) {
        if (err.getName() == DBus::MM_ERROR_ME_INCORRECT_PASSWORD) {
            throw SIMException{"Incorrect PIN"};
        }
        if (err.getName() == DBus::MM_ERROR_ME_INCORRECT_PARAMETERS) {
            throw SIMException{"Invalid PIN"};
        }
        throw SIMException{std::string{"failed to unlock SIM with PIN: "} + err.getMessage()};
    }
}

void SIM::send_puk(const std::string& puk, const std::string& pin) {
    try{
        /* dontExpectResult() prevents to throw exceptions
        * therefore, expect a void result (which can be stored in a sdbus::Variant)*/
        sdbus::Variant res;
        dbus_proxy_->callMethod("SendPuk")
            .onInterface(DBus::MM_IF_SIM).withArguments(puk, pin)
            .storeResultsTo(res);

    } catch(sdbus::Error& err) {
        if (err.getName() == DBus::MM_ERROR_ME_INCORRECT_PASSWORD) {
            throw SIMException{"Incorrect PUK"};
        }
        if (err.getName() == DBus::MM_ERROR_ME_INCORRECT_PARAMETERS) {
            throw SIMException{"Invalid PUK or PIN"};
        }
        throw SIMException{std::string{"failed to unlock SIM with PUK: "} + err.getMessage()};
    }
}

/* properties */

auto SIM::active() const -> bool {
    return dbus_proxy_->getProperty("Active").onInterface(DBus::MM_IF_SIM);
}

auto SIM::imsi() const -> std::string {
    return dbus_proxy_->getProperty("Imsi").onInterface(DBus::MM_IF_SIM);
}

auto SIM::iccid() const -> std::string {
    return dbus_proxy_->getProperty("SimIdentifier").onInterface(DBus::MM_IF_SIM);
}

auto SIM::home_plmn() const -> std::string {
    return dbus_proxy_->getProperty("OperatorIdentifier").onInterface(DBus::MM_IF_SIM);
}

auto SIM::operator_name() const -> std::string {
    return dbus_proxy_->getProperty("OperatorName").onInterface(DBus::MM_IF_SIM);
}

} // namespace ezcellular
