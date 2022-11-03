/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#pragma once

#include <ModemManager/ModemManager.h>

namespace ezcellular::DBus {

/* DBus default ifaces */
constexpr auto DBUS_IF_PROPERTIES = "org.freedesktop.DBus.Properties";
constexpr auto DBUS_IF_OBJECT_MANAGER = "org.freedesktop.DBus.ObjectManager";

/* ModemManager */
constexpr auto MM_BUS_NAME = MM_DBUS_SERVICE;
constexpr auto MM_IF_MODEMMANAGER = MM_DBUS_INTERFACE;
constexpr auto MM_OBJ_MODEMMANAGER = MM_DBUS_PATH;

/* ModemManager: Modem objects */
constexpr auto MM_IF_MODEM = MM_DBUS_INTERFACE_MODEM;
constexpr auto MM_IF_MODEM_LOCATION = MM_DBUS_INTERFACE_MODEM_LOCATION;
constexpr auto MM_IF_MODEM_MODEM3GPP = MM_DBUS_INTERFACE_MODEM_MODEM3GPP;
constexpr auto MM_IF_MODEM_SIMPLE = "org.freedesktop.ModemManager1.Modem.Simple";
constexpr auto MM_IF_MODEM_SIGNAL = MM_DBUS_INTERFACE_MODEM_SIGNAL;
constexpr auto MM_IF_MODEM_TIME = MM_DBUS_INTERFACE_MODEM_TIME;

/* ModemManager: Bearer objects */
constexpr auto MM_IF_BEARER = MM_DBUS_INTERFACE_BEARER;

/* ModemManager: SIM objects */
constexpr auto MM_IF_SIM = MM_DBUS_INTERFACE_SIM;

/* ModemManager: Errors */
constexpr auto MM_ERROR_ME_INCORRECT_PARAMETERS = "org.freedesktop.ModemManager1.Error.MobileEquipment.IncorrectParameters";
constexpr auto MM_ERROR_ME_INCORRECT_PASSWORD = "org.freedesktop.ModemManager1.Error.MobileEquipment.IncorrectPassword";

/* NetworkManager */
constexpr auto NM_BUS_NAME = "org.freedesktop.NetworkManager";
constexpr auto NM_IF_NETWORKMANAGER = NM_BUS_NAME;
constexpr auto NM_OBJ_NETWORKMANAGER = "/org/freedesktop/NetworkManager";

constexpr auto NM_IF_DEVICE_STATISTICS = "org.freedesktop.NetworkManager.Device.Statistics";

} // namespace ezcellular::DBus
