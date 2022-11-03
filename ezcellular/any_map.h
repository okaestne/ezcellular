/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#pragma once

#include <any>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include <sdbus-c++/sdbus-c++.h>


namespace ezcellular {


using sdbus_variant_map = std::map<std::string, sdbus::Variant>;

/**
 * @brief Base class for structured data, that is not always completely available.
 *
 */
class AnyMap : public std::map<std::string, std::any> {

public:
    // inherit default ctor
    using std::map<std::string, std::any>::map;

    // --- methods for keys ---

    /**
     * @brief returns the keys of all present values
     * @return vector of strings
     */
    [[nodiscard]] auto keys() const -> std::vector<std::string> {
        std::vector<std::string> keys(size());
        for (const auto& kv : *this) {
            keys.push_back(kv.first);
        }
        return keys;
    };

    /**
     * @brief whether a value for key is set
     * @param key the key
     * @return true if present, false otherwise
     */
    [[nodiscard]] auto has_key(const std::string& key) const -> bool {
        return find(key) != end();
    }

    // --- inherit methods for values ---

    /**
     * @brief get a value with the given type and key
     * @tparam T the type of the value
     * @param key the key of the value
     * @return the value
     * @see get_or_default()
     */
    template<typename T = std::string>
    auto get(const std::string& key) const -> T {
        auto val = at(key);
        return std::any_cast<T>(val);
    }

    /**
     * @brief get a value with the given type and key, or fall back to default
     * @tparam T the type of the value
     * @param key the key of the value
     * @param default_ the fallback value
     * @return the value or the fallback value
     */
    template<typename T = std::string>
    auto get_or_default(const std::string& key, const T& default_) const -> T {
        try {
            auto val = at(key);
            return std::any_cast<T>(val);
        } catch (...) {
            return default_;
        }
    }

    /**
     * @brief Copy a value of the given type and key into this, if available
     *
     * @tparam T the expected type in the variant value
     * @param dbus_map the variant map to extract the value from
     * @param key the key of the value
     */
    template<typename T>
    void maybe_insert_from_variant_map(const sdbus_variant_map& dbus_map, const std::string& key) {
        if (auto it = dbus_map.find(key); it != dbus_map.end()) {
            auto val = it->second.get<T>();
            insert({key, val});
        }
    }
    /**
     * @brief Insert a key-value pair from dbus_map of the given type and source key into this, if available
     *
     * @tparam T the expected type in the variant value
     * @param dbus_map the variant map to extract the value from
     * @param from_key the key of the value in the source map
     * @param as_key the key of the value in this
     */
    template<typename T>
    void maybe_insert_from_variant_map_as(const sdbus_variant_map& dbus_map, const std::string& from_key, const std::string& as_key) {
        if (auto it = dbus_map.find(from_key); it != dbus_map.end()) {
            auto val = it->second.get<T>();
            insert({as_key, val});
        }
    }
};

} // namespace ezcellular
