/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#pragma once

#include <cstdint>  // uint32_t and friends
#include <memory>   // std::unique_ptr, std::shared_ptr
#include <string>

#include "any_map.h" // AnyMap, sdbus_variant_map
#include "enums.h"   // Technology

namespace ezcellular {

/** @brief signal quality base class, cast to subclass depending on the value of tech */
class SignalBase : public AnyMap {
public:
    /// @private ctor without values
    explicit SignalBase(Technology tech) {
        insert({"tech", tech});
    }

    /// the technology for this signal information
    [[nodiscard]] auto tech() const -> Technology {
        return get<Technology>("tech");
    }
};

using Signal = std::shared_ptr<SignalBase>;

/** @brief LTE signal quality */
class SignalLTE : public SignalBase {
public:
    /** @brief default ctor */
    SignalLTE() : SignalBase{Technology::LTE} {}

    /// @private factory
    static auto from_variant_map(const sdbus_variant_map& dbus_map) -> std::shared_ptr<SignalLTE> {
        auto lte = std::make_shared<SignalLTE>();
        lte->maybe_insert_from_variant_map<double>(dbus_map, "rsrp");
        lte->maybe_insert_from_variant_map<double>(dbus_map, "rsrq");
        lte->maybe_insert_from_variant_map<double>(dbus_map, "rssi");
        lte->maybe_insert_from_variant_map_as<double>(dbus_map, "snr", "sinr");
        return lte;
    }

    /// Reference Signal Received Power (RSRP) in dBm
    [[nodiscard]] auto rsrp() const -> double { return get<double>("rsrp"); }
    /// Reference Signal Received Quality (RSRQ) in dB
    [[nodiscard]] auto rsrq() const -> double { return get<double>("rsrq"); }
    /// Reference Signal Strength Indication (RSSI) in dBm
    [[nodiscard]] auto rssi() const -> double { return get<double>("rssi"); }
    /// Signal to (interference plus) Noise Ratio (SNR) in dB
    [[nodiscard]] auto sinr() const -> double { return get<double>("sinr"); }
};

/** @brief NR5G signal quality */
class SignalNR5G : public SignalBase {
public:
    /** @brief default ctor */
    SignalNR5G() : SignalBase{Technology::NR5G} {}

    /// @private factory
    static auto from_variant_map(const sdbus_variant_map& dbus_map) -> std::shared_ptr<SignalNR5G> {
        auto nr = std::make_shared<SignalNR5G>();
        nr->maybe_insert_from_variant_map<double>(dbus_map, "rsrp");
        nr->maybe_insert_from_variant_map<double>(dbus_map, "rsrq");
        nr->maybe_insert_from_variant_map_as<double>(dbus_map, "snr", "sinr");
        return nr;
    }

    /// Reference Signal Received Power (RSRP) in dBm
    [[nodiscard]] auto rsrp() const -> double { return get<double>("rsrp"); }
    /// Reference Signal Received Quality (RSRQ) in dB
    [[nodiscard]] auto rsrq() const -> double { return get<double>("rsrq"); }
    /// Signal to (interference plus) Noise Ratio (SNR) in dB
    [[nodiscard]] auto sinr() const -> double { return get<double>("sinr"); }
};


/** @brief Location base class, cast to subclass depending on the value of tech */
class LocationBase : public AnyMap {
protected:
    /// @private initialize known keys from a sdbus_variant_map
    void fill_from_variant_map(const sdbus_variant_map& dbus_map) {
        if (auto it = dbus_map.find("operator-id"); it != dbus_map.end()) {
            std::string mcc;
            std::string mnc;
            plmn_to_mcc_mnc(it->second.get<std::string>(), mcc, mnc);
            insert({{"mcc", mcc}, {"mnc", mnc}});
        }
        if (auto it = dbus_map.find("ci"); it != dbus_map.end()) {
            auto ci_hex = it->second.get<std::string>();
            auto ci = static_cast<uint32_t>(std::stoul(ci_hex, nullptr, 16));
            insert({"ci", ci});
        }
    }

public:
    /// @private ctor
    explicit LocationBase(Technology tech) {
        insert({"tech", tech});
    }

    /**
     * @brief split a PLMN id into MCC and MNC
     * @param[in] plmn the PLMN id
     * @param[out] mcc the MCC
     * @param[out] mnc the MNC
     */
    static void plmn_to_mcc_mnc(const std::string& plmn, std::string& mcc, std::string& mnc) {
        mcc = plmn.substr(0, 3);  // always length 3
        mnc = plmn.substr(3, std::string::npos);  // until end
    }

    /// the technology for this location information
    [[nodiscard]] auto tech() const -> Technology { return get<Technology>("tech"); }
    /// Mobile Country Code (3 digits), e.g. "262" for germany
    [[nodiscard]] auto mcc() const -> std::string { return get("mcc"); }
    /// Mobile Network Code (2..3 digits), e.g. "01"
    [[nodiscard]] auto mnc() const -> std::string { return get("mnc"); }
    /// CellIdentity
    [[nodiscard]] auto ci() const -> uint32_t { return get<uint32_t>("ci"); }
};

using Location = std::shared_ptr<LocationBase>;

/**
 * @brief Identifiers that give information about the location of a LTE network cell
 */
class LocationLTE : public LocationBase {
protected:
    /// @private initialize known keys from a sdbus_variant_map
    void fill_from_variant_map(const sdbus_variant_map& dbus_map) {
        // mcc, mnc, ci
        LocationBase::fill_from_variant_map(dbus_map);
        // TAC from HEX to uint32_t
        if (auto it = dbus_map.find("tac"); it != dbus_map.end()) {
            auto tac_hex = it->second.get<std::string>();
            auto tac = static_cast<uint32_t>(std::stoul(tac_hex, nullptr, 16));
            insert({"tac", tac});
        }
    }

public:
    /// @private ctor
    LocationLTE() : LocationBase{Technology::LTE} {}

    /// @private factory
    static auto from_variant_map(const sdbus_variant_map& dbus_map) -> std::shared_ptr<LocationLTE> {
        auto lte = std::make_shared<LocationLTE>();
        lte->fill_from_variant_map(dbus_map);
        return lte;
    }

    /// Tracking Area Code (LTE/NR). 24 bits.
    [[nodiscard]] auto tac() const -> uint32_t { return get<uint32_t>("tac"); }
};

/**
 * @brief Identifiers that give information about the location of a NR/5G network cell
 */
class LocationNR5G : public LocationLTE {
public:
    /// @private ctor, overwrite tech attribute set by LocationLTE ctor
    LocationNR5G() { at("tech") = Technology::NR5G; }

    /// @private factory
    static auto from_variant_map(const sdbus_variant_map& dbus_map) -> std::shared_ptr<LocationNR5G> {
        auto nr = std::make_shared<LocationNR5G>();
        nr->fill_from_variant_map(dbus_map);  // uses LTE impl
        return nr;
    }
};

/**
 * @brief Cell information.
 * This class is based on ModemManagers GetCellInfo() return value
 * and contains signal, location and frequency information.
 *
 * @warning often not all values are set, use has_key() or get_or_default()
 * @note cast to subclass depending on the value of tech
 */
class CellInfoBase : public AnyMap {
public:
    /// @private ctor without values
    explicit CellInfoBase(Technology tech) {
        insert({"tech", tech});
    }

protected:
    /// @private called from derived classes
    void fill_from_variant_map(const sdbus_variant_map& dbus_map) {
        maybe_insert_from_variant_map<bool>(dbus_map, "serving");
        if (auto it = dbus_map.find("ci"); it != dbus_map.end()) {
            auto ci_hex = it->second.get<std::string>();
            auto ci = static_cast<uint32_t>(std::stoul(ci_hex, nullptr, 16));
            insert({"ci", ci});
        }
    }

public:
    /// @brief the technology for this cell information
    [[nodiscard]] auto tech() const -> Technology { return get<Technology>("tech"); }
    /// @brief whether the cell is serving (currently in use) or a neighboring cell
    [[nodiscard]] auto serving() const -> bool { return get_or_default<bool>("serving", false); }
    /// @brief CellIdentity, not available for non-serving cells
    [[nodiscard]] auto ci() const -> uint32_t { return get<uint32_t>("ci"); }
};

using CellInfo = std::shared_ptr<CellInfoBase>;  // always provided as a pointer for casting

/**
 * @brief LTE cell information
 * @see CellInfoBase
*/
class CellInfoLTE : public CellInfoBase {
public:
    /// @private ctor
    CellInfoLTE() : CellInfoBase{Technology::LTE} {}

    /// @private factory
    static auto from_variant_map(const sdbus_variant_map& dbus_map) -> std::shared_ptr<CellInfoBase> {
        auto lte = std::make_shared<CellInfoLTE>();

        // extract "serving" and "ci"
        lte->fill_from_variant_map(dbus_map);
        // earfcn, pci
        lte->maybe_insert_from_variant_map<uint32_t>(dbus_map, "earfcn");
        if (auto it = dbus_map.find("physical-ci"); it != dbus_map.end()) {
            auto pci_hex = it->second.get<std::string>();
            auto pci = static_cast<uint16_t>(std::stoul(pci_hex, nullptr, 16));
            lte->insert({"pci", pci});
        }
        // signal quality
        auto signal = SignalLTE::from_variant_map(dbus_map);
        lte->insert({"signal", signal});
        // location
        auto location = LocationLTE::from_variant_map(dbus_map);
        lte->insert({"location", location});

        return lte;
    }

    /// LTE EARFCN
    [[nodiscard]] auto earfcn() const -> uint32_t { return get<uint32_t>("earfcn"); }
    /// LTE physical cell id (PCI) (0..503)
    [[nodiscard]] auto pci() const -> uint16_t { return get<uint16_t>("pci"); }
    /// LTE signal quality
    [[nodiscard]] auto signal() const -> std::shared_ptr<SignalLTE> {
        return get<std::shared_ptr<SignalLTE>>("signal");
    }
    /// LTE location info
    [[nodiscard]] auto location() const -> std::shared_ptr<LocationLTE> {
        return get_or_default<std::shared_ptr<LocationLTE>>("location", {nullptr});
    }

};

/**
 * @brief 5G/NR cell information
 * @see CellInfoBase
*/
class CellInfoNR5G : public CellInfoBase {
public:
    /// @private ctor
    CellInfoNR5G() : CellInfoBase{Technology::NR5G} {}

    /// @private factory
    static auto from_variant_map(const sdbus_variant_map& dbus_map) -> std::shared_ptr<CellInfoBase> {
        auto nr = std::make_shared<CellInfoNR5G>();

        // extract "serving" and "ci"
        nr->fill_from_variant_map(dbus_map);
        // nrarfcn, pci
        nr->maybe_insert_from_variant_map<uint32_t>(dbus_map, "nrarfcn");
        if (auto it = dbus_map.find("physical-ci"); it != dbus_map.end()) {
            auto pci_hex = it->second.get<std::string>();
            auto pci = static_cast<uint16_t>(std::stoul(pci_hex, nullptr, 16));
            nr->insert({"pci", pci});
        }
        // signal quality
        auto signal = SignalNR5G::from_variant_map(dbus_map);
        nr->insert({"signal", signal});

        // location
        auto location = LocationNR5G::from_variant_map(dbus_map);
        nr->insert({"location", location});

        return nr;
    }

    /// NR5G NRARFCN
    [[nodiscard]] auto nrarfcn() const -> uint32_t { return get<uint32_t>("nrarfcn"); }
    /// NR5G physical cell id (PCI) (0..1007)
    [[nodiscard]] auto pci() const -> uint16_t { return get<uint16_t>("pci"); }
    /// NR5G signal quality
    [[nodiscard]] auto signal() const -> std::shared_ptr<SignalNR5G> {
        return get<std::shared_ptr<SignalNR5G>>("signal");
    }
    /// NR5G location info
    [[nodiscard]] auto location() const -> std::shared_ptr<LocationNR5G> {
        return get_or_default<std::shared_ptr<LocationNR5G>>("location", {nullptr});
    }
};

/**
 * @brief IP configuration of a Connection.
 */
struct IPConfig {
    IPType ip_type;      ///< IP version (only valid: IPType::IPV4, IPType::IPV6)
    std::string address; ///< IP address as string
    uint32_t prefix;     ///< Network mask (CIDR notation)
    std::string gateway; ///< Gateway IP address as string
    std::string dns1;    ///< Primary DNS server IP address as string
    std::string dns2;    ///< Secondary DNS server IP address as string
};

/**
 * @brief Traffic statistics.
 */
struct TrafficStats {
    uint64_t rx_bytes; ///< Received (RX) bytes
    uint64_t tx_bytes; ///< Transmitted (TX) bytes
};


} // namespace ezcellular
