/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#pragma once

#include <cstdint>      // uint32_t and friends

#include <ModemManager/ModemManager.h>  // enum values to map

namespace ezcellular {

/**
 * @brief radio technology
 */
enum class Technology {
    UNKNOWN = 0,    ///< unknown technology
    GSM = 1 << 0,   ///< 2G technology (GSM, GPRS, EDGE)
    UMTS = 1 << 1,  ///< 3G technology (UMTS, HSPA)
    LTE = 1 << 2,   ///< 4G technology (LTE, LTE-A)
    NR5G = 1 << 3,  ///< 5G technology (NR)
};

/**
 * @brief IP type of the connection (and its bearer)
 */
enum class IPType : uint32_t {
    UNKNOWN = MM_BEARER_IP_FAMILY_NONE,         ///< Unknown type (error)
    IPV4 = MM_BEARER_IP_FAMILY_IPV4,            ///< IPv4 only
    IPV6 = MM_BEARER_IP_FAMILY_IPV6,            ///< IPv6 only
    IPV4_AND_IPV6 = MM_BEARER_IP_FAMILY_IPV4V6, ///< both IPv4 and IPv6 (dual stack)
};

} // namespace ezcellular
